#include "esp_timer.h"
#include "serial_commands.h"

const int PUL_PINS[] = {14, 26}; 
const int DIR_PINS[] = {12, 5}; 

const int MOTOR_COUNT = 2;

int64_t last_motor_movement[] = {0, 0};
int64_t last_motor_state[] = {0, 0};

float motor_speeds[] = {800, 4000*2}; // steps per second
int motor_steps[] = {400, 8000000};

#define MICROSECONDS (1000000)

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  for (int motor = 0; motor < MOTOR_COUNT; motor++)
  {
    pinMode(PUL_PINS[motor], OUTPUT);
    pinMode(DIR_PINS[motor], OUTPUT);
  }
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
  while (soft_serial.available() > 0) {
    // read the incoming byte:
    incomingByte = soft_serial.read();
    inputString += (char) incomingByte;

    #ifdef DEBUG
    soft_serial.println("I received: "+String((char) incomingByte));
    #endif

    if (incomingByte == '&') { // end character
      read_message = 1;

      // print command 
      #ifdef DEBUG
      soft_serial.println("I received command: \"" + inputString + "\"");
      #endif
    }
  }

  // Guard Clause
  if (read_message == 0) {return;}

  // Parsing the command
  

  inputString = ""; // clear after use
}