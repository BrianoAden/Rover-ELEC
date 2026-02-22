#include <SerialWombat.h>

// --- IMPORTANT DEFINITIONS ---
int SPEED = 30000;
int PWM_PIN = 2;
int QE1_PIN = 6; 
int QE2_PIN = 7;
int REN_PIN = 1;
int LEN_PIN = 4;

// --- SERIAL WOMBAT CHIP OBJECTS ---
SerialWombatChip sw61;
SerialWombatChip sw62;
SerialWombatChip sw63;
SerialWombatChip sw64;

// Attach PWM and QE specifically to whichever you want sw__
SerialWombatPWM pwm(sw64); 
SerialWombatQuadEnc qeWithPullUps(sw64);

// ESP32 Encoder variables
#define DT GPIO_NUM_33
#define CLK GPIO_NUM_32
int lastStateCLK;
int oldval = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Initializing I2C Bus...");

  Wire.begin(21, 22); 
  Wire.setTimeOut(5000);
  delay(500);

  // --- INITIALIZE ALL CHIPS ---
  // We call begin on all of them so they are recognized on the bus
  if(sw61.begin(Wire, 0x61)) Serial.println("SW 0x61 Found");
  if(sw62.begin(Wire, 0x62)) Serial.println("SW 0x62 Found");
  if(sw63.begin(Wire, 0x63)) Serial.println("SW 0x63 Found");
  if(sw64.begin(Wire, 0x64)) Serial.println("SW 0x64 Found - ACTIVE");

  // --- CONFIGURE CHIP 0x62 ONLY ---
  // Even though others are initialized, we only send configuration to sw62
  pwm.begin(PWM_PIN, 0, false);
  qeWithPullUps.begin(QE1_PIN, QE2_PIN, 0.01, true, QE_ONHIGH_INT);

  sw64.pinMode(REN_PIN, OUTPUT);
  sw64.pinMode(LEN_PIN, OUTPUT);
  sw64.digitalWrite(REN_PIN, HIGH);
  sw64.digitalWrite(LEN_PIN, HIGH);

  Serial.println("Motor 0x64 enabled.");
  
  // Start motor on 0x62
  pwm.writeDutyCycle(SPEED);
  Serial.println("Motor 0x64 started.");

  // ESP32 Pins
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  lastStateCLK = digitalRead(CLK);
}

void loop() {
  // Only reads encoder from sw62
  int newval = qeWithPullUps.read();
  if(newval != oldval){
    Serial.print("Encoder 0x63: ");
    Serial.println(newval);
    oldval = newval;
  }

  delay(50);
}