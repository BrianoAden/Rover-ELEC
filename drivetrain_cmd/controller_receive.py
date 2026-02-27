# import serial

# PORT = "/dev/ttyTHS1"   # Jetson Nano GPIO UART
# BAUD = 57600

# ser = serial.Serial(
#     port=PORT,
#     baudrate=BAUD,
#     timeout=1
# )

# HEADER = bytes([0xAA, 0x55])
# PAYLOAD_LEN = 12

# def find_packet():
#     while True:
#         b = ser.read(1)
#         if not b:
#             return None

#         if b == HEADER[:1]:
#             b2 = ser.read(1)
#             if b2 == HEADER[1:]:
#                 rest = ser.read(PAYLOAD_LEN + 1)

#                 if len(rest) != PAYLOAD_LEN + 1:
#                     return None

#                 payload = rest[:PAYLOAD_LEN]
#                 chk = rest[-1]

#                 # Verify checksum
#                 x = 0
#                 for bb in payload:
#                     x ^= bb

#                 if x == chk:
#                     return payload
#                 # checksum fail → keep searching


# while True:
#     payload = find_packet()
#     if payload is None:
#         continue

#     data = list(payload)

#     print("Got:", data)

import serial
import time

# --- CONFIGURATION ---
RADIO_PORT = '/dev/ttyTHS1'  # From Radio
ROVER_PORT = '/dev/ttyUSB0'  # To Motor Controller
BAUD_RADIO = 57600
BAUD_ROVER = 115200

HEADER = bytes([0xAA, 0x55])

def start_bridge():
    try:
        # Initialize Serial Connections
        ser_radio = serial.Serial(RADIO_PORT, BAUD_RADIO, timeout=0.1)
        ser_rover = serial.Serial(ROVER_PORT, BAUD_ROVER, timeout=0.1)
        
        print(f"✅ Bridge Active: {RADIO_PORT} (Radio) <-> {ROVER_PORT} (Rover)")
        print("Waiting for packets...")

        while True:
            # 1. Sync: Look for Header [0xAA, 0x55]
            if ser_radio.read(1) == b'\xAA':
                if ser_radio.read(1) == b'\x55':
                    
                    # 2. Read Protocol Bytes
                    # [Mode(1)][ID(1)][Data(8)][Checksum(1)] = 11 bytes remaining
                    packet_body = ser_radio.read(11)
                    if len(packet_body) < 11:
                        continue
                    
                    mode = packet_body[0]
                    motor_id = packet_body[1]
                    data = packet_body[2:10]
                    received_chk = packet_body[10]

                    # 3. Verify Checksum (XOR of Mode + ID + 8 Data bytes)
                    calculated_chk = mode ^ motor_id
                    for b in data:
                        calculated_chk ^= b

                    if calculated_chk == received_chk:
                        # 4. Handle Specific Modes
                        if mode == 0x05:
                            # Decode string, strip null padding, add newline
                            cmd_str = data.decode('utf-8').strip('\x00')
                            full_cmd = (cmd_str + "\n").encode()
                            
                            ser_rover.write(full_cmd)
                            print(f"🚀 Relayed to Rover: {cmd_str}")
                        
                        elif mode == 0x01:
                            # Handle Arm Motor data here if needed
                            print(f"🦾 Arm Packet for ID {motor_id} received.")
                    else:
                        print("⚠️ Checksum mismatch. Dropping packet.")

    except serial.SerialException as e:
        print(f"❌ Serial Error: {e}")
    except KeyboardInterrupt:
        print("\nShutting down bridge...")
    finally:
        if 'ser_radio' in locals(): ser_radio.close()
        if 'ser_rover' in locals(): ser_rover.close()

if __name__ == "__main__":
    start_bridge()