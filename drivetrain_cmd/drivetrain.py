import serial
import sys

ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)

print("Type commands with newline character. Format is integer,speed. 30000 is a reasonable speed for testing. \n")
print("0 = stop rover\n1 = rover go forward\n2 = rover go backward\n3 = rover turn right\n4 = rover turn left\n5 = motor0 forward\n6 = motor0 backward\n7 = motor0 stop\n8 = motor1 forward\n9 = motor1 backward\n10 = motor1 stop\n11 = motor2 forward\n12 = motor2 backward\n13 = motor2 stop\n14 = motor3 forward\n15 = motor3 backward\n16 = motor3 stop\n17 = motor4 forward\n18 = motor4 backward\n19 = motor4 stop")


try:
    while True:
        input_data = sys.stdin.readline()
        if input_data:
            command = input_data + "\n"
            ser.write(command.encode())
except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()