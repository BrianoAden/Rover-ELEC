# import serial
# import time
# import sys
# import struct
# from pynput import keyboard

# # --- CONFIGURATION ---
# RADIO_PORT = '/dev/ttyUSB0' 
# BAUD = 57600
# MASTER_ID = 0xFD
# MOTOR_04_ID = 7    # A/D Keys
# MOTOR_02_ID = 127  # W/S Keys

# # --- SMOOTHING TUNES ---
# MOVE_INCREMENT = 0.12  # Larger steps to bridge radio gaps
# KP_ARM = 25.0          # Stiff tracking
# KD_ARM = 2.2           # Damping for smooth stops

# # --- STATE ---
# targets = {MOTOR_04_ID: 0.0, MOTOR_02_ID: 0.0}
# active_keys = {'w': False, 's': False, 'a': False, 'd': False}
# is_enabled = False
# HEADER = bytes([0xAA, 0x55])

# try:
#     ser = serial.Serial(RADIO_PORT, BAUD, timeout=0.1)
#     print(f"✅ Radio Link Active on {RADIO_PORT}")
# except Exception as e:
#     print(f"❌ Radio Error: {e}"); sys.exit(1)

# def float_to_uint16(v, v_min, v_max):
#     v = max(v_min, min(v_max, v))
#     return int((v - v_min) * 65535 / (v_max - v_min))

# def send_radio_frame(mode_byte, motor_id, data_bytes):
#     """[Header 2b][Mode 1b][ID 1b][Data 8b][Checksum 1b]"""
#     frame = HEADER + bytes([mode_byte, motor_id]) + data_bytes
#     chk = mode_byte ^ motor_id
#     for b in data_bytes: chk ^= b
#     packet = frame + bytes([chk])
#     ser.write(packet)
#     return packet

# def on_press(key):
#     global is_enabled
#     try:
#         if key.char in active_keys: active_keys[key.char] = True
#         if key.char == 'e':
#             print("\n[!] Enabling Motors...")
#             for m_id in [MOTOR_04_ID, MOTOR_02_ID]:
#                 send_radio_frame(0x04, m_id, bytes([0]*8)) # Clear
#                 time.sleep(0.05)
#                 send_radio_frame(0x04, m_id, bytes([1] + [0]*7)) # Enable
#                 time.sleep(0.05)
#                 send_radio_frame(0x03, m_id, bytes([0]*8)) # Op Mode
#             is_enabled = True
#             print("✅ Ready.")
#     except: pass

# def on_release(key):
#     try:
#         if key.char in active_keys: active_keys[key.char] = False
#         if key.char == 'x': return False
#     except: pass

# listener = keyboard.Listener(on_press=on_press, on_release=on_release)
# listener.start()

# print(f"CONTROLLER: A/D -> ID {MOTOR_04_ID} | W/S -> ID {MOTOR_02_ID}")
# print("[E] Enable | [X] Exit")

# try:
#     while listener.running:
#         if is_enabled:
#             # Update targets for ID 127 (02)
#             if active_keys['w']: targets[MOTOR_02_ID] += MOVE_INCREMENT
#             if active_keys['s']: targets[MOTOR_02_ID] -= MOVE_INCREMENT
            
#             # Update targets for ID 7 (04)
#             if active_keys['a']: targets[MOTOR_04_ID] -= MOVE_INCREMENT
#             if active_keys['d']: targets[MOTOR_04_ID] += MOVE_INCREMENT

#             for m_id in [MOTOR_04_ID, MOTOR_02_ID]:
#                 targets[m_id] = max(-12.5, min(12.5, targets[m_id]))
                
#                 # Build Robstride Motion Packet
#                 p_int = float_to_uint16(targets[m_id], -12.5, 12.5)
#                 can_data = struct.pack('>HHHH', p_int, 0, 
#                                    float_to_uint16(KP_ARM, 0, 500), 
#                                    float_to_uint16(KD_ARM, 0, 5))
                
#                 send_radio_frame(0x01, m_id, can_data)

#             sys.stdout.write(f"\rTargets -> 04(ID7): {targets[7]:.2f} | 02(ID127): {targets[127]:.2f}   ")
#             sys.stdout.flush()

#         time.sleep(0.02) 

# except KeyboardInterrupt: pass
# finally:
#     ser.close()


import serial
import time
import sys
import struct
from pynput import keyboard

# --- CONFIGURATION ---
RADIO_PORT = '/dev/ttyUSB0' 
BAUD = 57600
HEADER = bytes([0xAA, 0x55])
ROVER_ID = 255 

# --- MODES ---
MODE_ARM = 0
MODE_ROVER = 1
current_control_mode = MODE_ARM

try:
    ser = serial.Serial(RADIO_PORT, BAUD, timeout=0.1)
    print(f"✅ Radio Link Active on {RADIO_PORT}")
except Exception as e:
    print(f"❌ Radio Error: {e}"); sys.exit(1)

def send_radio_frame(mode_byte, motor_id, data_bytes):
    """Encapsulates data into the radio protocol"""
    # Pad data to 8 bytes to maintain fixed frame length for the radio
    data_bytes = data_bytes.ljust(8, b'\x00')
    frame = HEADER + bytes([mode_byte, motor_id]) + data_bytes[:8]
    chk = mode_byte ^ motor_id
    for b in data_bytes[:8]: chk ^= b
    ser.write(frame + bytes([chk]))

def on_press(key):
    global current_control_mode
    try:
        if key.char == 'm':
            current_control_mode = MODE_ROVER if current_control_mode == MODE_ARM else MODE_ARM
            mode_name = "ROVER (Terminal Pass-through)" if current_control_mode == MODE_ROVER else "ARM (Joints)"
            print(f"\n🔄 Mode: {mode_name}")
    except: pass

listener = keyboard.Listener(on_press=on_press)
listener.start()

print("--- CONTROLLER ACTIVE ---")
print("[M] Toggle Mode | [X] Exit")

try:
    while True:
        if current_control_mode == MODE_ROVER:
            # Direct terminal input passed to Radio
            cmd = input("ROVER CMD (e.g. 1,30000): ")
            if cmd:
                # Mode 0x05 signals this is a string command for the Jetson's USB0
                send_radio_frame(0x05, ROVER_ID, cmd.encode())
                print(f"📡 Sent to Radio: {cmd}")
        else:
            if active_keys['w']: targets[MOTOR_02_ID] += 0.12
            if active_keys['s']: targets[MOTOR_02_ID] -= 0.12
            if active_keys['a']: targets[MOTOR_04_ID] -= 0.12
            if active_keys['d']: targets[MOTOR_04_ID] += 0.12
                
            for m_id in [MOTOR_04_ID, MOTOR_02_ID]:
                targets[m_id] = max(-12.5, min(12.5, targets[m_id]))
                p_int = int((targets[m_id] + 12.5) * 65535 / 25.0)
                can_data = struct.pack('>HHHH', p_int, 0, int(25.0 * 131), int(2.2 * 13107))
                send_radio_frame(0x01, m_id, can_data)
                
            status = f"ARM -> 04: {targets[7]:.2f} | 02: {targets[127]:.2f}"

        sys.stdout.write(f"\r{status}      ")
        sys.stdout.flush()
    time.sleep(0.1)

except KeyboardInterrupt: pass
finally: ser.close()