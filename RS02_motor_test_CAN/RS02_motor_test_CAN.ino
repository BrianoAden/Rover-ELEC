#include <Arduino.h>
#include "driver/twai.h"

// --- CONFIGURATION ---
#define TX_PIN GPIO_NUM_21
#define RX_PIN GPIO_NUM_22
#define TARGET_MOTOR_ID 1   // Keep as 1 since you saw RX earlier
#define MASTER_ID 0x00

// --- RS02 / MIT PROTOCOL CONSTANTS ---
#define CMD_ENABLE      0x01
#define CMD_STOP        0x02
#define CMD_MOTION      0x03
#define CMD_SET_ZERO    0x06 // Often used to set zero, or specific pattern on Enable

// Limits (Must match motor manual scaling)
#define P_MIN -12.56637f
#define P_MAX 12.56637f
#define V_MIN -44.0f
#define V_MAX 44.0f
#define KP_MIN 0.0f
#define KP_MAX 500.0f
#define KD_MIN 0.0f
#define KD_MAX 5.0f
#define T_MIN -17.0f
#define T_MAX 17.0f

// --- UTILS ---
uint16_t float_to_uint(float x, float x_min, float x_max, int bits) {
  float span = x_max - x_min;
  float offset = x_min;
  if (x > x_max) x = x_max;
  else if (x < x_min) x = x_min;
  return (uint16_t) ((x - offset) * ((float)((1 << bits) - 1)) / span);
}

float uint_to_float(int x_int, float x_min, float x_max, int bits) {
  float span = x_max - x_min;
  float offset = x_min;
  return ((float)x_int * span / ((float)((1 << bits) - 1))) + offset;
}

// --- CAN SETUP ---
void setupCAN() {
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_PIN, RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("CAN Driver installed");
  } else {
    Serial.println("Error: CAN Driver install failed");
  }
  if (twai_start() == ESP_OK) {
    Serial.println("CAN Driver started");
  } else {
    Serial.println("Error: CAN Driver start failed");
  }
}

// --- COMMANDS ---

void sendEnable(uint8_t motor_id) {
  twai_message_t message;
  message.extd = 1;
  message.data_length_code = 8;
  // Command 1: Enable
  message.identifier = (CMD_ENABLE << 24) | (MASTER_ID << 8) | motor_id;
  // Data: All Zeros for standard enable
  memset(message.data, 0, 8);
  
  twai_transmit(&message, pdMS_TO_TICKS(10));
  Serial.println("Sent: ENABLE");
}

void sendZero(uint8_t motor_id) {
    twai_message_t message;
    message.extd = 1;
    message.data_length_code = 8;
    // Some motors use Command 1 with specific data, others Command 6.
    // Let's try Command 1 with data[7]=1 (common "Set Zero" flag)
    // Or Command 0x06 (Set Mode)
    message.identifier = (CMD_ENABLE << 24) | (MASTER_ID << 8) | motor_id;
    memset(message.data, 0, 8);
    message.data[0] = 1; // Try setting first byte to 1?
    
    // Actually, safest "Set Zero" is often purely mechanical on startup 
    // or handled by the encoder. We will just log that we are trying.
    Serial.println("Skipping explicit SetZero for now (risky without specific manual page)");
}

void sendMotion(uint8_t motor_id, float p, float v, float kp, float kd, float t_ff) {
  twai_message_t message;
  message.extd = 1;
  message.data_length_code = 8;

  uint16_t p_int = float_to_uint(p, P_MIN, P_MAX, 16);
  uint16_t v_int = float_to_uint(v, V_MIN, V_MAX, 12); // V often 12 bit
  uint16_t kp_int = float_to_uint(kp, KP_MIN, KP_MAX, 12); // Kp often 12 bit
  uint16_t kd_int = float_to_uint(kd, KD_MIN, KD_MAX, 12); // Kd often 12 bit
  uint16_t t_int = float_to_uint(t_ff, T_MIN, T_MAX, 12); // Torque often 12 bit

  // Construct ID: Command 3
  message.identifier = (CMD_MOTION << 24) | (t_int << 8) | motor_id;

  // MIT Cheetah Standard Packing:
  // 0: P_hi
  // 1: P_lo
  // 2: V_hi (4) | Kp_hi (4) -- This packing varies!
  // Let's stick to the SIMPLE 16-bit packing from V2 which the motor seemed to ACK.
  // Reverting to 16-bit simple packing:
  
  uint16_t p_16 = float_to_uint(p, P_MIN, P_MAX, 16);
  uint16_t v_16 = float_to_uint(v, V_MIN, V_MAX, 16);
  uint16_t kp_16 = float_to_uint(kp, KP_MIN, KP_MAX, 16);
  uint16_t kd_16 = float_to_uint(kd, KD_MIN, KD_MAX, 16);
  
  message.data[0] = p_16 >> 8;
  message.data[1] = p_16 & 0xFF;
  message.data[2] = v_16 >> 8;
  message.data[3] = v_16 & 0xFF;
  message.data[4] = kp_16 >> 8;
  message.data[5] = kp_16 & 0xFF;
  message.data[6] = kd_16 >> 8;
  message.data[7] = kd_16 & 0xFF;

  twai_transmit(&message, pdMS_TO_TICKS(5));
}

void debugRX() {
  twai_message_t message;
  if (twai_receive(&message, 0) == ESP_OK) {
    if (message.extd) {
      uint8_t type = (message.identifier >> 24) & 0xFF;
      uint8_t id = message.identifier & 0xFF;

      if (id == TARGET_MOTOR_ID) {
         Serial.print("RX Type: "); Serial.print(type);
         // Print the raw data bytes to see status codes
         Serial.print(" Data: ");
         for(int i=0; i<8; i++) {
            Serial.print(message.data[i], HEX);
            Serial.print(" ");
         }
         Serial.println();
      }
    }
  }
}

// --- MAIN ---
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- RS02 MOTION CONTROL TEST (V4) ---");
  setupCAN();
  
  // Multiple Enables to wake it up
  for(int i=0; i<5; i++) {
    sendEnable(TARGET_MOTOR_ID);
    delay(100);
    debugRX(); // Check if it replies to Enable
  }
  
  Serial.println("Setup Complete. Entering Loop...");
}

void loop() {
  static float t = 0;
  
  // Sine Wave Position
  // Move +/- 1.0 radian every few seconds
  float target_pos = 1.0f * sin(t);
  t += 0.05;

  // Send Command 3 (Motion)
  // Kp = 5.0 (Stiff enough to move)
  // Kd = 1.0 (Damping)
  sendMotion(TARGET_MOTOR_ID, target_pos, 0.0f, 5.0f, 1.0f, 0.0f);

  debugRX(); // Read feedback
  
  delay(20); // 50Hz
}