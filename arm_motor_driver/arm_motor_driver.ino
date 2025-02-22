#include "esp_timer.h"

// #define VERBOSE_COMMANDS
#define COMMAND_SERIAL Serial
// #define DEBUG

#include "serial_commands.h"

const int PUL_PINS[] = {14, 26}; 
const int DIR_PINS[] = {12, 5}; 

constexpr int MOTOR_COUNT = 2;

const float max_motor_speed = 1200;

int64_t last_motor_movement[] = {0, 0};
int64_t last_motor_state[] = {0, 0};

// Motor Pulses per Revolution
int motor_ppr[MOTOR_COUNT] = {400, 400};

float motor_speeds[MOTOR_COUNT] = {800, 800}; // steps per second
int motor_steps[MOTOR_COUNT] = {100, 0};

int motor_directions[MOTOR_COUNT] = {1, 1};

// Stored in pulses
int motor_positions[MOTOR_COUNT] = {0, 0};

int input_cursor = 0;
char input_string[MAX_COMMAND_LENGTH];

#define MICROSECONDS (1000000)


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

  // For debugging
  // Serial.print("Got the integer argument: ");
  // Serial.println(motor_id);
  // Serial.print("Got the float argument: ");
  // Serial.println(motor_speed);

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

void loop() {
  update_motors();
  int incomingByte = 0;
  char read_message = 0;
  while (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    input_string[input_cursor] = (char) incomingByte;
    input_cursor++;

    #ifdef DEBUG
    Serial.println("I received: "+String((char) incomingByte));
    #endif

    if (incomingByte == '&') { // end character

      // Replacing the end character with a string end to not mess things up
      input_string[input_cursor-1] = '\0'; 

      // print command 
      #ifdef DEBUG
      Serial.print("I received command: \"");
      Serial.print(input_string);
      Serial.println("\"");
      #endif

      cHandler.runCommand(input_string);
  

      // We dont technically have to clear the string as it will get overwritten
      input_cursor = 0;
    }
  }
}