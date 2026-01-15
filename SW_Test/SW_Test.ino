#include <SerialWombat.h>

SerialWombatChip sw;    //Declare a Serial Wombat chip

//we're using a quadrature encoder with built-in pullup resistors
SerialWombatQuadEnc qeWithPullUps(sw);

//PWM at frequency of 31250 Hz.  Can change this probably if needed but we haven't figured out how yet
SerialWombatPWM pwm(sw);



int PWM = 1;
int QE1 = 2;
int QE2 = 3;
int enableBoth = 4;
//int checkStall = 5;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(2000);
  Serial.print("Hello");
  Wire.begin(21, 22); // Initialize I2C with custom pin
  Wire.setTimeOut(5000);
	delay(500);
	uint8_t i2cAddress = 0x60; //sw.find();
    sw.begin(Wire,i2cAddress);  //Initialize the Serial Wombat library to use the primary I2C port
	sw.registerErrorHandler(SerialWombatSerialErrorHandlerBrief); //Register an error handler that will print communication errors to Serial

  Serial.println("here");

  sw.queryVersion();

  Serial.println();
  Serial.print("Version "); Serial.println((char*)sw.fwVersion);
  Serial.println("SW Found.");

  //begin(pin, starting duty cycle, inverted true/false)
  pwm.begin(PWM, 0, false);
  qeWithPullUps.begin(QE1, QE2);  // Initialize a QE on pins 2 and 3


  //digital enable pins
  sw.pinMode(enableBoth, OUTPUT);
  //sw.pinMode(checkStall, INPUT);

  //enable motor
  sw.digitalWrite(enableBoth, HIGH);
  Serial.print("enabled motor");
  //spin motor
  pwm.writeDutyCycle(3000);
  Serial.print("Started motor");

  //analogWrite(33, 100);
}

void loop() {
  //return;
  //Serial.println(qeWithPullUps.read());

//  if(sw.digitalRead(checkStall) == HIGH){
//    Serial.println("Stalling");
 // }
  delay(50);
}