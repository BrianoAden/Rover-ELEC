#include "esp_timer.h"

#define VERBOSE_COMMANDS
#define COMMAND_SERIAL Serial
#define DEBUG

#include "serial_commands.h"

const int PUL_PINS[] = {14, 26}; 
const int DIR_PINS[] = {12, 5}; 

const int MOTOR_COUNT = 2;

int64_t last_motor_movement[] = {0, 0};
int64_t last_motor_state[] = {0, 0};

float motor_speeds[] = {800, 4000*2}; // steps per second
int motor_steps[] = {400, 8000000};

int input_cursor = 0;
char input_string[MAX_COMMAND_LENGTH];

#define MICROSECONDS (1000000)


void test_command(void **arg_stack)
{
  int argument = *((int*) (arg_stack[0]));
  Serial.print("Got the integer argument: ");
  Serial.println(argument);
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

  CommandArgType test_command_args[MAX_SCOMMAND_ARGUMENTS];
  test_command_args[0] = CommandArgType::INT_ARG;
  cHandler.addCommand('T', test_command, test_command_args);
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
      motor_steps[motor]--;

      digitalWrite(PUL_PINS[motor], last_motor_state[motor]);
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
      read_message = 1;

      // Replacing the end character with a string end to not mess things up
      input_string[input_cursor-1] = '\0'; 

      // print command 
      #ifdef DEBUG
      Serial.print("I received command: \"");
      Serial.print(input_string);
      Serial.println("\"");
      #endif
    }
  }

  // Guard Clause
  if (read_message == 0) {return;}

  cHandler.runCommand(input_string);
  

  // We dont technically have to clear the string as it will get overwritten
  input_cursor = 0;
}