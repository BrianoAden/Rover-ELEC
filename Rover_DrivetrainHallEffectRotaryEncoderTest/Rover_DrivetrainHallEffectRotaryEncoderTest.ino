// Rotary Encoder Inputs
#define DT 33
#define CLK 32

int clicks_per_rotation = 5000;

float lastClick = 0;
float currentClick = 0;
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
      currentClick = millis();
    } else {
      // Encoder is rotating CW so increment
      counter ++;
      currentDir ="CW";
      currentClick = millis();
    }
    RPM = 60000/(clicks_per_rotation*(currentClick - lastClick)); //1 click/[change in time] ms * 1 rotation/[clicks per rotation] * 1000ms/1s * 60s/1min
    Serial.print("Direction: ");
    Serial.print(currentDir);
    Serial.print(" | Counter: ");
    Serial.println(counter);
    lastClick = currentClick;
  }

  // Remember last CLK state
  lastStateCLK = currentStateCLK;

  // Put in a slight delay to help debounce the reading
  delay(1);
}
