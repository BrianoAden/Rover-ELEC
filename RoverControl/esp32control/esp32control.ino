#include <Arduino.h>

#define STEPPER_ID 0x01
#define STEPPER_CMD_MODE 0x06

// Pins for DRV8825
const int DIR_PIN = 19;
const int STEP_PIN = 18;
const int STEP_DELAY_US = 500; 

void decodeAndMove(uint8_t* data);

void setup() {
  // Over USB, we use the standard Serial port
  Serial.begin(115200);                
  
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  Serial.println("ESP32 USB Stepper Receiver Ready");
}

void loop() {
  // Sync: Look for 0xAA
  while (Serial.available() > 0 && Serial.peek() != 0xAA) {
    Serial.read(); 
  }

  // Need 13 bytes for a full frame
  if (Serial.available() >= 13) {
    uint8_t h1 = Serial.read(); // 0xAA
    uint8_t h2 = Serial.read(); // 0x55
    
    if (h1 == 0xAA && h2 == 0x55) {
      uint8_t mode = Serial.read();
      uint8_t motor_id = Serial.read();
      uint8_t data[8];
      Serial.readBytes(data, 8);
      uint8_t received_chk = Serial.read();

      // Checksum
      uint8_t calculated_chk = mode ^ motor_id;
      for (int i = 0; i < 8; i++) calculated_chk ^= data[i];

      if (calculated_chk == received_chk) {
        if (motor_id == STEPPER_ID && mode == STEPPER_CMD_MODE) {
          // Debug print (will show up in your Serial Monitor)
          Serial.println("\n>>> Stepper Frame Received!");
          decodeAndMove(data);
        }
      }
    }
  }
}

void decodeAndMove(uint8_t* data) {
  uint8_t direction = data[0]; 
  uint16_t steps = (data[1] << 8) | data[2];
  uint16_t speed_delay = (data[3] << 8) | data[4];

  if (speed_delay == 0) speed_delay = 500; 

  digitalWrite(DIR_PIN, direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(speed_delay);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(speed_delay);
  }
}