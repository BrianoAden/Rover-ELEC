//make all of the move functions here to be called by main program
#include <SerialWombat.h>

//speed of all driving motors:
int driveSpeed = 30000;
//speed of drill:
int drillSpeed = 30000;

//-------------starting definitions:
  //for rotary encoder stall tracking:
  int CLK0 = 0;
  int CLK1 = 0;
  int CLK2 = 0;
  int CLK3 = 0;
  int CLK4 = 0;

  //sw pins setup:
  int RIS = 1;
  int REN = 2;
  int RPWM = 3;
  int LPWM = 4;
  int LEN = 5;
  int LIS = 6;
  int CHA = 7;
  int CHB = 8;

  //initialize sw chips
  SerialWombatChip motor0;
  SerialWombatChip motor1;
  SerialWombatChip motor2;
  SerialWombatChip motor3;
  SerialWombatChip motor4;

  //initialize REs
  SerialWombatQuadEnc RE0(motor0);
  SerialWombatQuadEnc RE1(motor1);
  SerialWombatQuadEnc RE2(motor2);
  SerialWombatQuadEnc RE3(motor3);
  SerialWombatQuadEnc RE4(motor4);

  //initialize PWMs
  SerialWombatPWM RPWM0(motor0);
  SerialWombatPWM LPWM0(motor0);
  SerialWombatPWM RPWM1(motor1);
  SerialWombatPWM LPWM1(motor1);
  SerialWombatPWM RPWM2(motor2);
  SerialWombatPWM LPWM2(motor2);
  SerialWombatPWM RPWM3(motor3);
  SerialWombatPWM LPWM3(motor3);
  SerialWombatPWM RPWM4(motor4);
  SerialWombatPWM LPWM4(motor4);



//-------------setup: must be called at the beginning of main program in order for the rest of the commands to work 
  void initializeSW(){
    //initialize sw chip and pins for each motor (motor numbers printed on PCB)

    //start i2C
    Wire.begin(21, 22); // Initialize I2C with custom pin
    Wire.setTimeOut(5000);
    delay(500);

    //set sw addresses
    uint8_t i2cAddress0 = 0x60;
    uint8_t i2cAddress1 = 0x61;
    uint8_t i2cAddress2 = 0x62;
    uint8_t i2cAddress3 = 0x63;
    uint8_t i2cAddress4 = 0x64;

    //Initialize sws on I2C
    motor0.begin(Wire,i2cAddress0);
    motor1.begin(Wire,i2cAddress1);
    motor2.begin(Wire,i2cAddress2);
    motor3.begin(Wire,i2cAddress3);
    motor4.begin(Wire,i2cAddress4);

    //begin PWM pins
    RPWM0.begin(RPWM, 0, false);
    LPWM0.begin(LPWM, 0, false);
    RPWM1.begin(RPWM, 0, false);
    LPWM1.begin(LPWM, 0, false);
    RPWM2.begin(RPWM, 0, false);
    LPWM2.begin(LPWM, 0, false);
    RPWM3.begin(RPWM, 0, false);
    LPWM3.begin(LPWM, 0, false);
    RPWM4.begin(RPWM, 0, false);
    LPWM4.begin(LPWM, 0, false);
    
    //set up enable pins
    motor0.pinMode(LEN, OUTPUT);
    motor0.pinMode(REN, OUTPUT);
    motor1.pinMode(LEN, OUTPUT);
    motor1.pinMode(REN, OUTPUT);
    motor2.pinMode(LEN, OUTPUT);
    motor2.pinMode(REN, OUTPUT);
    motor3.pinMode(LEN, OUTPUT);
    motor3.pinMode(REN, OUTPUT);
    motor4.pinMode(LEN, OUTPUT);
    motor4.pinMode(REN, OUTPUT);

    //set up current alarms
    motor0.pinMode(LIS, INPUT);
    motor0.pinMode(RIS, INPUT);
    motor1.pinMode(LIS, INPUT);
    motor1.pinMode(RIS, INPUT);
    motor2.pinMode(LIS, INPUT);
    motor2.pinMode(RIS, INPUT);
    motor3.pinMode(LIS, INPUT);
    motor3.pinMode(RIS, INPUT);
    motor4.pinMode(LIS, INPUT);
    motor4.pinMode(RIS, INPUT);

    //initialize rotary encoders
    RE0.begin(CHA, CHB, 0.01, true, QE_ONHIGH_INT);
    RE1.begin(CHA, CHB, 0.01, true, QE_ONHIGH_INT);
    RE2.begin(CHA, CHB, 0.01, true, QE_ONHIGH_INT);
    RE3.begin(CHA, CHB, 0.01, true, QE_ONHIGH_INT);
    RE4.begin(CHA, CHB, 0.01, true, QE_ONHIGH_INT);

    //enable all motors
    motor0.digitalWrite(LEN, HIGH);
    motor0.digitalWrite(REN, HIGH);
    motor1.digitalWrite(LEN, HIGH);
    motor1.digitalWrite(REN, HIGH);
    motor2.digitalWrite(LEN, HIGH);
    motor2.digitalWrite(REN, HIGH);
    motor3.digitalWrite(LEN, HIGH);
    motor3.digitalWrite(REN, HIGH);
    motor4.digitalWrite(LEN, HIGH);
    motor4.digitalWrite(REN, HIGH);
  }

//------------subcommands: will not be called in main program, but used to build commands

  //turns any motor right
  void r(str motornum, dutyCycle){
    s(motorname);
    str RightPWM = "RPWM" + motornum;
    RightPWM.writeDutyCycle(dutyCycle);
  }

  //turns any motor left
  void l(str motornum, dutyCycle){
    s(motornum);
    str LeftPWM = "LPWM" + motornum;
    LeftPWM.writeDutyCycle(dutyCycle);
  }

  //stops motor
  void s(str motornum){
    str RightPWM = "RPWM" + motornum;
    str LeftPWM = "LPWM" + motornum;
    RightPWM.writeDutyCycle(0);
    RightPWM.writeDutyCycle(0);
  }


//------------commands: will be called in main program 

  //stops all drivetrain motors
  void stop(){
    s(1, driveSpeed);
    s(2, driveSpeed);
    s(3, driveSpeed);
    s(4, driveSpeed);
  }

  //makes rover drive forward. Can swap "r" with "l" if rover goes wrong direction
  void df(){
    stop()
    r(1, driveSpeed);
    r(2, driveSpeed);
    l(3, driveSpeed);
    l(4, driveSpeed);
  }

  //makes rover drive backward. Can swap "r" with "l" if rover goes wrong direction
  void db(){
    stop()
    l(1, driveSpeed);
    l(2, driveSpeed);
    r(3, driveSpeed);
    r(4, driveSpeed);
  }

  //makes rover turn right. Can swap "r" with "l" if rover goes wrong direction
  void tl(){
    stop()
    l(1, driveSpeed);
    l(2, driveSpeed);
    l(3, driveSpeed);
    l(4, driveSpeed);
  }

  //makes rover turn left. Can swap "r" with "l" if rover goes wrong direction
  void tr(){
    stop()
    r(1, driveSpeed);
    r(2, driveSpeed);
    r(3, driveSpeed);
    r(4, driveSpeed);
  }



//------------checkStall and subcommands: a class all of its own 
  bool checkStall(bool newcommand, int currentcommand){
    bool stall;
    if(newcommand){
      //NEED TO UPDATE ALL GLOBAL ENCODER VALUES
      return false;
    }
    else if(currentcommand == 0){
        return false;
      }
    }
    else if((currentcommand >0)&&(currentcommand<4)){
      stall1 = getStall(1);
      stall2 = getStall(2);
      stall3 = getStall(3);
      stall4 = getStall(4);
      if(((stall1 == true)||(stall2==true))||((stall3==true)||(stall4==true))){
        return true;
      }
      else{
        return false;
      }
    }
    else if((currentcommand >3)&&(currentcommand<8)){
      stall = getStall(0);
      return stall;
    }
    else if((currentcommand >7)&&(currentcommand<11)){
      stall = getStall(1);
      return stall;
    }
    else if((currentcommand >10)&&(currentcommand<14)){
      stall = getStall(2);
      return stall;
    }
    else if((currentcommand >13)&&(currentcommand<17)){
      stall = getStall(3);
      return stall;
    }
    else if((currentcommand >16)&&(currentcommand<20)){
      stall = getStall(4);
      return stall;
    }
  }

  bool getStall(string motornum){
    REname = "RE" + motornum;
    int newCLK = REname.read();
    int oldCLK = CLK(motornum);
    if((oldCLK-newCLK != 0)){
      return false;
    }
    else{
      return true;
    }
  }

  int CLK(string motornum){
    if(motornum==0){
      return CLK0;
    }
    if(motornum==1){
      return CLK1;
    }
    if(motornum==2){
      return CLK2;
    }
    if(motornum==3){
      return CLK3;
    }
    if(motornum==4){
      return CLK4;
    }
  }






