//make all of the move functions here to be called by main program
//UNFINISHED
//need to do sw syntax in initializeSW(), r(), l(), and s().
//need to add ability to control motor speed via r(), l(), and s() if desired.
//need to add rotary encoders/speed monitoring and control in drillf(), drillb(), df(), db(), tr(), and tl() if desired.
//need to add motor stall prevention using current alarms if desired.


//sw pins setup:
int RIS = 1;
int REN = 2;
int RPWM = 3;
int LPWM = 4;
int LEN = 5;
int LIS = 6;

/*//uncomment if using REs through sw chip:
int CLK = 7;
int DT = 8;

//if using ESP-32 to manage REs, need to copy pin connection names for REs 0--4
*/



//-------------setup: must be called at the beginning of main program in order for the rest of the commands to work
void initializeSW(){
  //create sw chip for each motor, named motor0, motor1, etc.
  //numbers corresponding to each motor are printed on PCB!!
  for(int i=0; i<5; i++){
    str name = "motor" + str(i);
    //create new sw chip named "name"
    //set i2c address of chip equal to 60 + i
    //set pinmodes for each pin (RIS & LIS: input, REN & LEN: output, RPWM & LPWM: PWM, DT & CLK: encoders)
    //start i2c communication with sw chip
  }
}

//------------subcommands: will not be called in main program, but used to build commands

//turns any motor right
void r(str motorname){
  s(motorname);
  //set pins on sw chip to turn motor R
}

//turns any motor left
void l(str motorname){
  s(motorname);
  //set pins on sw chip to turn motor  L
}

//stops motor
void s(str motorname){
  //set pins on sw chip to stop motor
}


//------------commands: will be called in main program

//stops all drivetrain motors
void stop(){
  s(motor1);
  s(motor2);
  s(motor3);
  s(motor4);
}

//turns drill forward. Can swap "r" with "l" if drill turns wrong direction
void drillf(){
  r(motor0);
}

//turns drill backward. Can swap "r" with "l" if drill turns wrong direction
void drillb(){
  l(motor0);
}

//makes rover drive forward. Can swap "r" with "l" if rover goes wrong direction
df(){
  stop()
  r(motor1);
  r(motor2);
  l(motor3);
  l(motor4);
}

//makes rover drive backward. Can swap "r" with "l" if rover goes wrong direction
db(){
  stop()
  l(motor1);
  l(motor2);
  r(motor3);
  r(motor4);
}

//makes rover turn right. Can swap "r" with "l" if rover goes wrong direction
tl(){
  stop()
  l(motor1);
  l(motor2);
  l(motor3);
  l(motor4);
}

//makes rover turn left. Can swap "r" with "l" if rover goes wrong direction
tl{
  stop()
  r(motor1);
  r(motor2);
  r(motor3);
  r(motor4);
}







