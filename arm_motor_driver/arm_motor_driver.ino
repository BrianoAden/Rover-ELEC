#include "esp_timer.h"

#define MICROSECONDS (1000000)

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define VERBOSE_COMMANDS
#define COMMAND_SERIAL Serial
#define INPUT_SERIAL Serial
// #define DEBUG

#include "serial_commands.h"

const int PUL_PINS[] = {14, 26}; 
const int DIR_PINS[] = {12, 25}; 

constexpr int MOTOR_COUNT = 2;

const float max_motor_speed = 1200;

int64_t last_motor_movement[] = {0, 0};
int64_t last_motor_state[] = {0, 0};

#define REFRESH_RATE (5)
#define REFRESH_TICKS (MICROSECONDS / REFRESH_RATE)

// Motor Pulses per Revolution
int motor_ppr[MOTOR_COUNT] = {400, 400};

float motor_speeds[MOTOR_COUNT] = {200, 1600}; // steps per second
int motor_steps[MOTOR_COUNT] = {0, 0};

int motor_directions[MOTOR_COUNT] = {1, 1};

// Stored in pulses
int motor_positions[MOTOR_COUNT] = {0, 0};

int input_cursor = 0;
char input_string[MAX_COMMAND_LENGTH];


void test_command(void **arg_stack)
{
  int argument = *((int*) (arg_stack[0]));
  Serial.print("Got the integer argument: ");
  Serial.println(argument);
}

void motor_speed_set(void **arg_stack)
{
  int motor_id = *((int*) (arg_stack[0]));
  float motor_speed = *((float*) (arg_stack[1]));

  motor_speeds[motor_id] = motor_speed;
}

void motor_per_speed_set(void **arg_stack)
{
  int motor_id = *((int*) (arg_stack[0]));
  float motor_speed = *((float*) (arg_stack[1]));
  motor_speeds[motor_id] = motor_speed*max_motor_speed;
}

void motor_step_set(void **arg_stack)
{
  int motor_id = *((int*) (arg_stack[0]));
  int steps = *((int*) (arg_stack[1]));
  motor_steps[motor_id] = steps;
}

void motor_pos_set(void **arg_stack)
{
  int motor_id = *((int*) (arg_stack[0]));
  int pos = *((int*) (arg_stack[1]));
  int step_dif = pos-motor_positions[motor_id];

  motor_directions[motor_id] = step_dif > 0 ? 1 : -1;
  motor_steps[motor_id] = abs(step_dif);
}

CommandHandler cHandler;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  for (int motor = 0; motor < MOTOR_COUNT; motor++)
  {
    pinMode(PUL_PINS[motor], OUTPUT);
    pinMode(DIR_PINS[motor], OUTPUT);
  }

  // Test Command
  CommandArgType test_command_args[MAX_SCOMMAND_ARGUMENTS];
  test_command_args[0] = CommandArgType::INT_ARG;
  cHandler.addCommand('F', test_command, test_command_args);

  // Motor Speed
  CommandArgType motor_speed_cargs[MAX_SCOMMAND_ARGUMENTS] = {INT_ARG, FLOAT_ARG};
  cHandler.addCommand('S', motor_speed_set, motor_speed_cargs);

  // Motor Speed
  CommandArgType motor_per_speed_cargs[MAX_SCOMMAND_ARGUMENTS] = {INT_ARG, FLOAT_ARG};
  cHandler.addCommand('R', motor_per_speed_set, motor_speed_cargs);

  // Motor Steps
  CommandArgType motor_steps_cargs[MAX_SCOMMAND_ARGUMENTS] = {INT_ARG, INT_ARG};
  cHandler.addCommand('C', motor_step_set, motor_steps_cargs);

  // Motor Steps
  CommandArgType motor_pos_cargs[MAX_SCOMMAND_ARGUMENTS] = {INT_ARG, INT_ARG};
  cHandler.addCommand('P', motor_pos_set, motor_pos_cargs);

  #ifdef DEBUG
  Serial.println("Finished Setup");
  #endif

  // Screen Setup
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    // for(;;); // Don't proceed, loop forever
  }

  display.display();

  motor_steps[1] = 10000;


}

void update_motors()
{
  int64_t time = esp_timer_get_time();
  for (int motor = 0; motor < MOTOR_COUNT; motor++)
  {
    if (motor_steps[motor] > 0 && last_motor_movement[motor]+2*MICROSECONDS/motor_speeds[motor] < time)
    {
      last_motor_movement[motor] = time;
      last_motor_state[motor] = !last_motor_state[motor];
      motor_positions[motor] += motor_directions[motor];
      motor_steps[motor]--;

      digitalWrite(PUL_PINS[motor], last_motor_state[motor]);
      digitalWrite(DIR_PINS[motor], motor_directions[motor]==1?LOW:HIGH);
    }
  }
}

int64_t last_refresh = 0;
void loop() {
  // cHandler.readSerial();
  update_motors();
  

  int64_t time = esp_timer_get_time();

  if (time-last_refresh > REFRESH_TICKS)
  {
    last_refresh = time;
    display.clearDisplay();

    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner

    display.print(MOTOR_COUNT);
    display.print(F(" Motors\n\n"));
    

    for (int i = 0; i < MOTOR_COUNT; i++)
    {
      display.printf("MTR:%6d STP:%6d\n", i, motor_steps[i]);
      display.printf("RPM:%6.1f POS:%6d\n", (float) motor_speeds[i] / motor_ppr[i] * 60.0, motor_positions[i]);
    }


    display.display();
  }
}