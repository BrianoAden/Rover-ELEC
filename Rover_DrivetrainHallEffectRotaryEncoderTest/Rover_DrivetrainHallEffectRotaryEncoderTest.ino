// Rotary Encoder Inputs
#define DT 33
#define CLK 32

float clicks_per_rotation = 5264;

//for calculating RPM & only printing once every second (to keep the printing process from slowing it down):
unsigned long lastPrint = 0;
unsigned long lastCounter = 0;

float RPM = 0;
int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;

void setup() {
  
  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);

  // Setup Serial Monitor
  Serial.begin(115200);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
}

void loop() {
  
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      counter --;
      currentDir ="CCW";
    } else {
      // Encoder is rotating CW so increment
      counter ++;
      currentDir ="CW";
    }
    if((millis() - lastPrint) > 1000){
      //RPM = [# of clicks]/[change in time, ms] * 1 rotation/[clicks per rotation] * 1000ms/1s * 60s/1min
      RPM = 60000*(counter - lastCounter)/(clicks_per_rotation*(millis() - lastPrint));
      Serial.print("Direction: ");
      Serial.print(currentDir);
      Serial.print(" | Counter: ");
      Serial.print(counter);
      Serial.print(" | RPM: ");
      Serial.println(RPM);
      lastPrint = millis();
      lastCounter = counter;
    }
  }

  // Remember last CLK state
  lastStateCLK = currentStateCLK;

  // Put in a slight delay to help debounce the reading
  delayMicroseconds(1);
}
