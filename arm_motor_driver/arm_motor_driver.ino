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

struct StepperMotor {
  int PUL_PIN;
  int DIR_PIN;
  int64_t last_movement = 0;
  int last_state = 0;
  int ppr = 400;
  float max_speed = 400;
  float velocity = 0;
  float target_velocity = 0;
  float acceleration = 20000;
  int direction = 1;
  int position = 0;
  int steps = 0;
  float deceleration_window;
};

int display_state = 0;
int64_t last_button_down;
float button_average = 0;
int button_state = 0;
const int64_t BUTTON_DEBOUNCE = ((int64_t) (MICROSECONDS * .25));

constexpr int MOTOR_COUNT = 4;
StepperMotor motors[MOTOR_COUNT];

// In revolutions per second
const float max_motor_speed = 3;

#define REFRESH_RATE (10)
#define REFRESH_TICKS (MICROSECONDS / REFRESH_RATE)

int input_cursor = 0;
char input_string[MAX_COMMAND_LENGTH];

int sgn(int val) {
    return (0 < val) - (val < 0);
}

float sgnf(float val) {
    return (0 < val) - (val < 0);
}

void motor_speed_set(void **arg_stack)
{
  int motor_id = *((int*) (arg_stack[0]));
  float motor_speed = *((float*) (arg_stack[1]));

  motors[motor_id].max_speed = motor_speed;
}

void motor_per_speed_set(void **arg_stack)
{
  int motor_id = *((int*) (arg_stack[0]));
  float motor_speed = *((float*) (arg_stack[1]));

  // speed percentage * max revolutions per second * pulses per revolution * 2
  motors[motor_id].max_speed = motor_speed*max_motor_speed*motors[motor_id].ppr*2;
}

void motor_set_step_offset(int motor_id, int step_offset)
{
  StepperMotor *motor = &motors[motor_id];
  motor->target_velocity = sgn(step_offset)*motor->max_speed;
  motor->direction = sgn(step_offset);
  motor->steps = abs(step_offset);
}

void motor_step_set(void **arg_stack)
{
  int motor_id = *((int*) (arg_stack[0]));
  int steps = *((int*) (arg_stack[1]));

  motor_set_step_offset(motor_id, steps);
}

void motor_pos_set(void **arg_stack)
{
  int motor_id = *((int*) (arg_stack[0]));
  int pos = *((int*) (arg_stack[1]));

  int step_dif = pos-motors[motor_id].position;
  motor_set_step_offset(motor_id, step_dif);
}

CommandHandler cHandler;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // Initializing Motors

  // Motor 0
  motors[0].PUL_PIN = 26;
  motors[0].DIR_PIN = 25;

  // Motor 1
  motors[1].PUL_PIN = 14;
  motors[1].DIR_PIN = 27;

  // Motor 2
  motors[2].PUL_PIN = 32;
  motors[2].DIR_PIN = 33;

  // Motor 3
  motors[3].PUL_PIN = 18;
  motors[3].DIR_PIN = 19;

  // Motor Pins
  for (int motor_id = 0; motor_id < MOTOR_COUNT; motor_id++)
  {
    StepperMotor *motor = &motors[motor_id];
    pinMode(motor->PUL_PIN, OUTPUT);
    pinMode(motor->DIR_PIN, OUTPUT);
  }

  // Button Pin
  pinMode(BUTTON_PIN, INPUT);

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

int64_t last_time = 0;
void update_motors()
{
  int64_t time = esp_timer_get_time();
  int64_t dt = time - last_time;
  float dt_s = (float) dt / MICROSECONDS; // deltatime in seconds
  last_time = time;

  for (int motor_id = 0; motor_id < MOTOR_COUNT; motor_id++)
  {
    StepperMotor *motor = &motors[motor_id];
    if (motor->steps > 0)
    {
      // Handling Speed
      if (motor->target_velocity == 0)
      {
        // Decelerate
        motor->deceleration_window -= dt_s;
        if (motor->deceleration_window < 0)
          motor->deceleration_window = 0;
        else
          motor->velocity = 2 * motor->steps / motor->deceleration_window * sgn(motor->velocity);

      } else  {
        // Accelerate
        if (motor->target_velocity > 0)
          motor->velocity = min(motor->velocity + motor->acceleration * dt_s, motor->target_velocity);
        else
           motor->velocity = max(motor->velocity + motor->acceleration * -dt_s, motor->target_velocity);
      }
    } else {
      motor->velocity = 0;
    }

    if (motor->steps > 0 && abs(motor->velocity) > 0 && motor->last_movement+MICROSECONDS/abs(motor->velocity) < time)
    {
      motor->last_movement = time;
      motor->last_state = !motor->last_state;
      motor->position += sgn(motor->velocity);
      if (abs(motor->target_velocity) > 0)
        motor->steps -= sgn(motor->velocity*motor->target_velocity);
      else 
        motor->steps--;
      
      int steps_to_stop = motor->velocity*motor->velocity / (motor->acceleration * 2);
      if (motor->steps <= steps_to_stop)
      {
        motor->target_velocity = 0;
        motor->deceleration_window = abs(motor->velocity)/motor->acceleration;
      } else {
        motor->target_velocity = motor->max_speed*motor->direction;
      }

      digitalWrite(motor->PUL_PIN, motor->last_state);
      digitalWrite(motor->DIR_PIN, motor->velocity > 0 ?LOW:HIGH);
    }
  }
}

#define BUTTON_AVG (500.0)
int64_t last_display_refresh = 0;
void displayUpdate()
{
  int64_t time = esp_timer_get_time();

  // checking for button down

  if (digitalRead(BUTTON_PIN) == LOW)
  {
    button_average = button_average * (float) (BUTTON_AVG-1.0)/BUTTON_AVG + 1.0/BUTTON_AVG;
    if (button_average > .65 && button_state == 0)
    {
      display_state = (display_state + 1) % MOTOR_COUNT;
      last_button_down = time;
      button_state = 1;
    }
    
  } else {
    button_average *= (float) (BUTTON_AVG-1.0)/BUTTON_AVG;
    if (button_average < .4)
      button_state = 0;
  }

  if (time-last_display_refresh < REFRESH_TICKS) {return;}
  last_display_refresh = time;
  // Serial.println(digitalRead(BUTTON_PIN));

  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner

  int motor_id = display_state; // probably temp
  StepperMotor* motor = &(motors[motor_id]);
  

  display.print(F("Motor: "));
  display.print(motor_id);
  display.println();
  display.println();
  

  
  display.printf("STP%7d\n", motor->steps);
  display.printf("VEL%7.1f\n", motor->velocity);
  display.printf("RPM%7.1f\n", motor->velocity / motor->ppr * 30.0);
  display.printf("POS%7d\n", motor->position);
  display.printf("WIN%7.1f\n", motor->deceleration_window);

  // display.printf("WIN%7.1f\n", (float) motor[deceleration_windows);


  display.display();
}

void loop() {
  cHandler.readSerial();
  update_motors();
  

  displayUpdate();
}