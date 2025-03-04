#include "esp_timer.h"

#define MICROSECONDS (1000000)

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define BUTTON_PIN 23

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define VERBOSE_COMMANDS
#define COMMAND_SERIAL Serial
#define INPUT_SERIAL Serial
// #define DEBUG

#include "serial_commands.h"

const int PUL_PINS[] = {14, 25}; 
const int DIR_PINS[] = {12, 26}; 

constexpr int MOTOR_COUNT = 2;

// In revolutions per second
const float max_motor_speed = 3;

int64_t last_motor_movement[] = {0, 0, 0, 0};
int64_t last_motor_state[] = {0, 0, 0, 0};

#define REFRESH_RATE (10)
#define REFRESH_TICKS (MICROSECONDS / REFRESH_RATE)

// Motor Pulses per Revolution
int motor_ppr[MOTOR_COUNT] = {1600, 1600};

// steps per second
float motor_speeds[MOTOR_COUNT] = {0, 0}; 
float motor_target_speeds[MOTOR_COUNT] = {200, 200};
float motor_min_speeds[MOTOR_COUNT] = {10, 10};

// steps per second per second
float motor_acceleration[MOTOR_COUNT] = {10, 10};

// motor steps - two steps is one on/off cycle
int motor_steps[MOTOR_COUNT] = {0, 0};

// the direction the stepper will move in
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

  motor_target_speeds[motor_id] = motor_speed;
}

void motor_per_speed_set(void **arg_stack)
{
  int motor_id = *((int*) (arg_stack[0]));
  float motor_speed = *((float*) (arg_stack[1]));

  // speed percentage * max revolutions per second * pulses per revolution * 2
  motor_target_speeds[motor_id] = motor_speed*max_motor_speed*motor_ppr[motor_id]*2;
}

void motor_step_set(void **arg_stack)
{
  int motor_id = *((int*) (arg_stack[0]));
  int steps = *((int*) (arg_stack[1]));

  // setting direction based off of sign
  motor_directions[motor_id] = steps > 0 ? 1 : -1;
  motor_steps[motor_id] = abs(steps);
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

  // Motor Pins
  for (int motor = 0; motor < MOTOR_COUNT; motor++)
  {
    pinMode(PUL_PINS[motor], OUTPUT);
    pinMode(DIR_PINS[motor], OUTPUT);
  }

  // Button Pin
  pinMode(BUTTON_PIN, INPUT);

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
}

float deceleration_window = 5;
int64_t last_time = 0;
void update_motors()
{
  int64_t time = esp_timer_get_time();
  int64_t dt = time - last_time;
  float dt_s = (float) dt / MICROSECONDS;
  last_time = time;
  for (int motor = 0; motor < MOTOR_COUNT; motor++)
  {
    if (motor_steps[motor] > 0)
    {
      // Handling Speed
      if (motor_speeds[motor] > motor_target_speeds[motor])
      {
        // Decelerate
        deceleration_window -= dt_s;
        if (deceleration_window < 0)
          deceleration_window = 0;
        else
          // motor_speeds[motor] = ((5000-motor_steps[motor])+0.5*motor_acceleration[motor]*deceleration_window*deceleration_window)/deceleration_window;
          motor_speeds[motor] = 2 * motor_steps[motor] / deceleration_window;
      } else if (motor_speeds[motor] < motor_target_speeds[motor]) {
        // Accelerate
        motor_speeds[motor] = min(motor_speeds[motor] + motor_acceleration[motor] * dt_s, motor_target_speeds[motor]);
      }
    }
    

    if (motor_steps[motor] > 0 && last_motor_movement[motor]+MICROSECONDS/motor_speeds[motor] < time)
    {
      last_motor_movement[motor] = time;
      last_motor_state[motor] = !last_motor_state[motor];
      motor_positions[motor] += motor_directions[motor];
      motor_steps[motor]--;

      int steps_to_stop = motor_speeds[motor]*motor_speeds[motor] / (motor_acceleration[motor] * 2);

      if (motor_steps[motor] <= 0.5*deceleration_window*motor_speeds[motor])
      {
        motor_target_speeds[motor] = 0;
      }

      digitalWrite(PUL_PINS[motor], last_motor_state[motor]);
      digitalWrite(DIR_PINS[motor], motor_directions[motor]==1?LOW:HIGH);
    }
  }
}

void displayStats()
{
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner

  display.print(MOTOR_COUNT);
  display.print(F(" Motors\n\n"));
  

  for (int i = 0; i < MOTOR_COUNT; i++)
  {
    display.printf("MTR%7d STP%7d\n", i, motor_steps[i]);
    display.printf("RPM%7.1f POS%7d\n", (float) motor_speeds[i] / motor_ppr[i] * 60.0, motor_positions[i]);
  }

  display.printf("WIN%7.1f\n", (float) deceleration_window);


  display.display();
}

int64_t last_refresh = 0;
void loop() {
  cHandler.readSerial();
  update_motors();
  

  int64_t time = esp_timer_get_time();

  if (time-last_refresh > REFRESH_TICKS)
  {
    last_refresh = time;
    displayStats();
  }
}