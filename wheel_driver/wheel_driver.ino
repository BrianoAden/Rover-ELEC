#include <DynamixelShield.h>
#include <SoftwareSerial.h>
#include "BTS7960.h"

const uint8_t L_PWM = 10;
const uint8_t R_PWM = 11;
const uint8_t L_EN = 12;
const uint8_t R_EN = 13;

BTS7960 motorController(L_EN, R_EN, L_PWM, R_PWM);

SoftwareSerial soft_serial(7, 8);

#define VERBOSE_COMMANDS
#define COMMAND_SERIAL soft_serial
#define INPUT_SERIAL soft_serial

#include "serial_commands.h"

const float DXL_PROTOCOL_VERSION = 2.0;

#define MOTOR_MODE_POSITION 0
#define MOTOR_MODE_VELOCITY 1

struct Motor {
  char id;
  int current_mode; //0 for MOTOR_MODE_POSITION and 1 for MOTOR_MODE_VELOCITY
  int current_unit;
  float value;
};

DynamixelShield dxl;

//This namespace is required to use Control table item names
using namespace ControlTableItem;

#define MOTOR_COUNT 5

Motor motors[] = {
  {-1, MOTOR_MODE_VELOCITY, UNIT_RAW, 0}, // unused

  {1, MOTOR_MODE_VELOCITY, UNIT_RAW, 0},
  {2, MOTOR_MODE_VELOCITY, UNIT_RAW, 0},
  {3, MOTOR_MODE_VELOCITY, UNIT_RAW, 0},
  {4, MOTOR_MODE_VELOCITY, UNIT_RAW, 0}
};


CommandHandler cHandler;

bool refresh_motor(int id)
{
  switch(motors[id].current_mode) 
  {
    case MOTOR_MODE_POSITION:
      return dxl.setGoalVelocity(motors[id].id, motors[id].value, motors[id].current_unit);
    case MOTOR_MODE_VELOCITY:
      return dxl.setGoalVelocity(motors[id].id, motors[id].value, motors[id].current_unit);
  }
}

void update_motor(int id)
{
  refresh_motor(id);
  if (id == 1)
    refresh_motor(3);
  else if (id == 2)
    refresh_motor(4);
}

void set_motor_velocity(int id, int unit, float value)
{
  motors[id].current_unit = unit;
  motors[id].value = value;
  update_motor(id);
}

void set_motor_velocity_command(void **arg_stack)
{
  int motor_id = *((int*) (arg_stack[0]));
  int unit = *((int*) (arg_stack[1]));
  float motor_speed = *((float*) (arg_stack[2]));
  set_motor_velocity(motor_id, unit, motor_speed);
}

void set_sides_command(void **arg_stack)
{
  int unit = *((int*) (arg_stack[0]));
  float left_motor_speed = *((float*) (arg_stack[1]));
  float right_motor_speed = *((float*) (arg_stack[2]));
  set_motor_velocity(1, unit, left_motor_speed);
  set_motor_velocity(4, unit, left_motor_speed);

  set_motor_velocity(2, unit, -right_motor_speed);
  set_motor_velocity(3, unit, -right_motor_speed);
}

// Drill commands
void set_drill_velocity_command(void **arg_stack)
{
  float motor_speed = *((float*) (arg_stack[0]));
  set_drill_velocity(motor_speed);
}

void set_drill_velocity(float speed)
{
  if (speed > 0) {
        motorController.TurnRight(speed); // Move forward
      } else if (speed < 0) {
        motorController.TurnLeft(-speed); // Move backward
      } else {
        motorController.Stop(); // Stop motor
  }
}
void setup() {
  // Serial
  soft_serial.begin(9600);
  motorController.Enable();
  // Command Setup

  // set_motor_velocity
  CommandArgType motor_velocity_cargs[MAX_SCOMMAND_ARGUMENTS] = {INT_ARG, INT_ARG, FLOAT_ARG};
  cHandler.addCommand('V', set_motor_velocity_command, motor_velocity_cargs);

  // set_sides
  CommandArgType sides_cargs[MAX_SCOMMAND_ARGUMENTS] = {INT_ARG, FLOAT_ARG, FLOAT_ARG};
  cHandler.addCommand('S', set_sides_command, sides_cargs);

  // set_drill_velocity
  CommandArgType drill_velocity_cargs[MAX_SCOMMAND_ARGUMENTS] = {FLOAT_ARG};
  cHandler.addCommand('D', set_drill_velocity_command, drill_velocity_cargs);

  // Dynamixel Setup
  dxl.begin(1000000);
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  for (int i = 0; i < MOTOR_COUNT; i++)
  {
    if (motors[i].id == -1) {continue;}
    dxl.torqueOff(motors[i].id);
    dxl.setOperatingMode(motors[i].id, OP_VELOCITY);
    dxl.torqueOn(motors[i].id);
  }
}

void loop()
{
  cHandler.readSerial();
}