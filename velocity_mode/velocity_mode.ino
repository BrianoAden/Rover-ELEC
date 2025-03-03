#include <DynamixelShield.h>
#include <SoftwareSerial.h>

SoftwareSerial soft_serial(7, 8);

const uint8_t DXL_ID_1 = 1;
const uint8_t DXL_ID_2 = 2;
const uint8_t DXL_ID_3 = 3;
const uint8_t DXL_ID_4 = 4;
const float DXL_PROTOCOL_VERSION = 2.0;

#define STOP 0
#define MOVE_FORWARD 1
#define MOVE_BACKWARDS 2
#define ROTATE_LEFT 3
#define ROTATE_RIGHT 4

#define MOTOR_MODE_POSITION 0
#define MOTOR_MODE_VELOCITY 1

struct Motor {
  int current_mode; //0 for MOTOR_MODE_POSITION and 1 for MOTOR_MODE_VELOCITY
  int current_unit;
  float value;
};

// #define DEBUG

String inputString = "";

DynamixelShield dxl;

//This namespace is required to use Control table item names
using namespace ControlTableItem;

Motor motors[] = {
  {1, UNIT_RAW, 0},
  {1, UNIT_RAW, 0},
  {1, UNIT_RAW, 0},
  {1, UNIT_RAW, 0},
};

void check_for_dxl_error(String tag)
{
  #ifdef DEBUG
  uint8_t packet_error = dxl.getLastStatusPacketError();
  if (packet_error) {
    soft_serial.print(tag);
    soft_serial.print("Packet error with error code int:");
    soft_serial.print((int) packet_error);
    soft_serial.print(", char:");
    soft_serial.println((char) packet_error);
  }
  
  DXLLibErrorCode_t error_code = dxl.getLastLibErrCode();
  if (error_code != 0) {
    soft_serial.print("Lib error with error code int:");
    soft_serial.print((int) error_code);
    soft_serial.print(", char:");
    soft_serial.println((char) error_code);
  }
    
  #endif
}

void setup() {
  // put your setup code here, to run once:
  soft_serial.begin(9600);
  
  // For Uno, Nano, Mini, and Mega, use UART port of DYNAMIXEL Shield to debug.
  // DEBUG_SERIAL.begin(115200);
  
  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(1000000);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  check_for_dxl_error("Dynamixel Startup");

  // Get DYNAMIXEL information
  dxl.ping(DXL_ID_1);
  dxl.ping(DXL_ID_2);
  dxl.ping(DXL_ID_3);
  dxl.ping(DXL_ID_4); 

  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(DXL_ID_1);
  dxl.torqueOff(DXL_ID_2);
  dxl.torqueOff(DXL_ID_3);
  dxl.torqueOff(DXL_ID_4);
  dxl.setOperatingMode(DXL_ID_1, OP_VELOCITY);
  dxl.setOperatingMode(DXL_ID_2, OP_VELOCITY);
  dxl.setOperatingMode(DXL_ID_3, OP_VELOCITY);
  dxl.setOperatingMode(DXL_ID_4, OP_VELOCITY);
  dxl.torqueOn(DXL_ID_1);
  dxl.torqueOn(DXL_ID_2);
  dxl.torqueOn(DXL_ID_3);
  dxl.torqueOn(DXL_ID_4);

  check_for_dxl_error("Setting motor modes");
}

bool refresh_motor(int id)
{
  switch(motors[id].current_mode) 
  {
    case MOTOR_MODE_POSITION:
      return dxl.setGoalVelocity(id, motors[id].value, motors[id].current_unit);
    case MOTOR_MODE_VELOCITY:
      return dxl.setGoalVelocity(id, motors[id].value, motors[id].current_unit);
  }
}

bool set_motor(int id, int mode, float value, int unit)
{
  motors[id].current_mode = mode;
  motors[id].value = value;
  motors[id].current_unit = unit;
  return refresh_motor(id);
}

void move_forward(float speed_rpm) //function to turn on all motors and move forward
{
  set_motor(1, 1, speed_rpm, UNIT_RPM);
  set_motor(2, 1, speed_rpm, UNIT_RPM);
  set_motor(3, 1, speed_rpm, UNIT_RPM);
  set_motor(4, 1, speed_rpm, UNIT_RPM);
}
void motor_command() // Example Motor Command "M1:position:100&"
{
  check_for_dxl_error("Pre Motor command");

  // Getting motor id
  uint8_t id = inputString.charAt(1);

  if (id != 'A'){
    id = id - '0';
  }

  // checking for valid id 1,2,3 or 4
  #ifdef DEBUG
  if (id != 'A'){
    if (id < 1 || id > 4) {
      soft_serial.println("Invalid Motor Id \""+String((int) inputString.charAt(1))+"\"");
      return;
    }
  }
  
  if (inputString.charAt(2) != ':') {
    soft_serial.println("Invalid formatting Error: Invalid character \""+String(inputString.charAt(2))+"\" at position 2");
    return;
  }
  #endif

  // Getting the command
  int arg_position = inputString.indexOf(':', 3);

  #ifdef DEBUG
  if (arg_position == -1) {
    soft_serial.println("Formatting Error: Unable to find the : needed to mark the end of an argument");
  }
  #endif

  String motor_command = inputString.substring(3, arg_position);
  int value_arg_pos = inputString.indexOf('&', arg_position+1);
  String value_str = inputString.substring(arg_position+1, value_arg_pos);
  float value = value_str.toFloat();

  bool action_result;

  if (motor_command == "set_position_raw") {
      action_result = set_motor(id, 0, value, UNIT_RAW);
  } else if (motor_command == "set_position_deg") {
      action_result = set_motor(id, 0, value, UNIT_DEGREE);
  } else if (motor_command == "set_velocity_raw") {
      action_result = set_motor(id, 1, value, UNIT_RAW);
  } else if (motor_command == "set_velocity_rpm") {
      action_result = set_motor(id, 1, value, UNIT_RPM);
  } else if (motor_command == "set_velocity_per") {
      action_result = set_motor(id, 1, value, UNIT_RPM);
  } else if (motor_command == "move_forward"){
      move_forward(value);
  } 

  if (id == 1)
    refresh_motor(3);
  else if (id == 2)
    refresh_motor(4);

  // #ifdef DEBUG
  // else {
  //   soft_serial.println("Invalid Motor Command:\"" + motor_command+"\"");
  //   return;
  // }

  // check_for_dxl_error("Motor command");

  
  // if (action_result == false)
  // {
  //   // int error_code = dxl.getLastLibErrCode();
  //   soft_serial.print("Motor command \"" + motor_command+"\" with error ");
  //   soft_serial.println(dxl.getLastLibErrCode());
  // }
  // #endif
}

void loop() {
  // check_for_dxl_error("Initial Motor Movements");
  
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
      soft_serial.println(F("I received command: \"" + inputString + "\""));
      #endif
    }
  }

  // Guard Clause
  if (read_message == 0) {return;}

  // Parsing the command
  switch (inputString.charAt(0)) {
    case 'M':
      soft_serial.println(F("Got a motor command"));
      motor_command();
      break;
  }

  inputString = ""; // clear after use
}