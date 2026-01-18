/*NEED to add serial communication with jetson:

1) initialize serial in setup(); ~line 75
2) update message and speed (both integers) from Jetson messages; ~line 85
3) broadcast "stalled" back to Jetson if stalled; ~line 165
4) provide doccumentation about how to format messages from the Jetson to work with
this program; ~line 15
*/


//use this sketch to run main communication with arduino and call appropriate move functions


  /*Instructions that can be sent by Jetson:
    format: NEED instruction format that allows both speed and message to be sent and extracted

    speed = any integer between 0 and 65536.  Higher number = higher speed.
    Note: 30000 is a reasonable speed to start testing.

    message = one of the following integers with the desired corresponding message:
      0 = stop rover
      1 = rover go forward
      2 = rover go backward
      3 = rover turn right
      4 = rover turn left
      5 = motor0 forward
      6 = motor0 backward
      7 = motor0 stop
      8 = motor1 forward
      9 = motor1 backward
      10 = motor1 stop
      11 = motor2 forward
      12 = motor2 backward
      13 = motor2 stop
      14 = motor3 forward
      15 = motor3 backward
      16 = motor3 stop
      17 = motor4 forward
      18 = motor4 backward
      19 = motor4 stop

      Note: see printed PCB for motor definitions.
      motor0 is the drill; motor1--4 are drivetrain
  */

//variable definitions
  bool newcommand = false;
  bool stalled = false;
  int currentcommand = 10;
  int message = 10;
  int speed = 0;


//list of functions from movementLibrary.ino to make compilation smoother
  void initializeSW();
  void r(int motornum, int dutyCycle);
  void l(int motornum, int dutyCycle);
  void s(int motornum);

  void stop();
  void df();
  void db();
  void tl();
  void tr();

  bool checkStall(bool newcommand, int currentcommand);
  bool getStall(int motornum);

void setup() {
  // put your setup code here, to run once:

  //initialize sw chips (from movement library)
  initializeSW();

  
  //NEED to set up serial communication between ESP32 and Jetson

}

void loop() {


  //NEED code to get instructions from Jetson
  message = 0;//new command from Jetson
  speed = 30000;//can also get this from jetson


  if(message != currentcommand){
    currentcommand = message;
    newcommand = true;
    stalled = false; 
  }

  //stalled cannot be set to true if newcommand is true
  //if newcommand is true, stalled just updates RE status
  stalled = checkStall(newcommand, currentcommand);

  if(newcommand){
    newcommand = false;
    //do correct motor thing using command
      if(currentcommand==0){
        stopAll();
      }
      if(currentcommand==1){
        df(speed);
      }
      if(currentcommand==2){
        db(speed);
      }
      if(currentcommand==3){
        tr(speed);
      }
      if(currentcommand==4){
        tl(speed);
      }
      if(currentcommand==5){
        r(0, speed);
      }
      if(currentcommand==6){
        l(0, speed);
      }
      if(currentcommand==7){
        s(0);
      }
      if(currentcommand==8){
        r(1, speed);
      }
      if(currentcommand==9){
        l(1, speed);
      }
      if(currentcommand==10){
        s(1);
      }
      if(currentcommand==11){
        r(2, speed);
      }
      if(currentcommand==12){
        l(2, speed);
      }
      if(currentcommand==13){
        s(2);
      }
            if(currentcommand==14){
        r(3, speed);
      }
      if(currentcommand==15){
        l(3, speed);
      }
      if(currentcommand==16){
        s(3);
      }
            if(currentcommand==17){
        r(4, speed);
      }
      if(currentcommand==18){
        l(4, speed);
      }
      if(currentcommand==19){
        s(4);
      }

    
    }
    
  if(stalled){
    //send to the Jetson that it's stalled

  }
    //
  }
