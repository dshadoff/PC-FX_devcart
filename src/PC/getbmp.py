#(c) 2024  David Shadoff
import serial
import sys
import time

# Notes:
#
# This program will read the PC-FX's external Backup memory card (FX-BMP) into a 128KB file
#
#   Usage: getbmp <output_file> [COM port]
#
#   Example:
#     python getbmp.py pcfxbmp.bin COM3
#

if ((len(sys.argv) != 3) and (len(sys.argv) != 2)):
    print("Usage: getbmp <output_file> [COM port]")
    exit()

ser = serial.Serial()
ser.baudrate = 115200

size = 131072

# Add override for windows-style COM ports.
if (len(sys.argv) == 3):
    ser.port = sys.argv[2]
else:
    ser.port = '/dev/ttyUSB0'

ser.open()

f = open(sys.argv[1], 'wb') 

# start = time.time()*1000.0

ser.write(b'R')             # one-byte command to receive a whole 128KB save file
ser.flush()

memory = ser.read(size)     # read a while file
f.write(memory)

# end = time.time()*1000.0
# speed = 131072/(end-start)
# print(end-start, ' milliseconds for 128KB')
# print(speed, ' KB/s')

f.close()

ser.close()

