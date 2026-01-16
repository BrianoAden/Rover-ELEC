#include "movementLibrary.ino"


//use this sketch to run main communication with arduino and call appropriate move functions

  /*Instructions that can be sent by Jetson:
    0 = stop rover
    1 = rover go forward
    2 = rover go backward
    3 = rover turn right
    4 = rover turn left
    5 = drill forward
    6 = drill backward
    7 = drill stop
    8 = m1 forward
    9 = m1 backward
    10 = m1 stop
    11 = m2 forward
    12 = m2 backward
    13 = m2 stop
    14 = m3 forward
    15 = m3 backward
    16 = m3 stop
    17 = m4 forward
    18 = m4 backward
    19 = m4 stop
  */

bool newcommand = false;
bool stalled = false;
int currentcommand = 10;
int message = 10;

void setup() {
  // put your setup code here, to run once:

  //initialize sw chips (from movement library)
  void initializeSW();

  Serial.begin(115200);
  
  //NEED to set up serial communication between ESP32 and Jetson

}

void loop() {


  //NEED code to get instructions from Jetson
  message = //new command from Jetson
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
        stop();
      }
      if(currentcommand==1){
        df();
      }
      if(currentcommand==2){
        db();
      }
      if(currentcommand==3){
        tr();
      }
      if(currentcommand==4){
        tl();
      }
      if(currentcommand==5){
        r(0, drillSpeed);
      }
      if(currentcommand==6){
        l(0, drillSpeed);
      }
      if(currentcommand==7){
        s(0);
      }
      if(currentcommand==8){
        r(1, motorSpeed);
      }
      if(currentcommand==9){
        l(1, motorSpeed);
      }
      if(currentcommand==10){
        s(1);
      }
      if(currentcommand==11){
        r(2, motorSpeed);
      }
      if(currentcommand==12){
        l(2, motorSpeed);
      }
      if(currentcommand==13){
        s(2);
      }
            if(currentcommand==14){
        r(3, motorSpeed);
      }
      if(currentcommand==15){
        l(3, motorSpeed);
      }
      if(currentcommand==16){
        s(3);
      }
            if(currentcommand==17){
        r(4, motorSpeed);
      }
      if(currentcommand==18){
        l(4, motorSpeed);
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
}
