#(c) 2024  David Shadoff
import serial
import os
import sys
import time

# Notes:
#
# This program will write the PC-FX's external Backup memory card (FX-BMP) from a 128KB file
#
#   Usage: putbmp <input_file> [COM port]
#
#   Example:
#     python putbmp.py pcfxbmp.bin COM3
#

if ((len(sys.argv) != 3) and (len(sys.argv) != 2)):
    print("Usage: putbmp <input_file> [COM port]")
    exit()


ser = serial.Serial()
ser.baudrate = 115200

size = 131072

# Add override for windows-style COM ports.
if (len(sys.argv) == 3):
    ser.port = sys.argv[2]
else:
    ser.port = '/dev/ttyUSB0'

# Check that the file is exactly 128KB in size:
#
file_stat = os.stat(sys.argv[1])
if (file_stat.st_size != 131072):
    print("File must be 131072 bytes in size")
    exit()

ser.open()

f = open(sys.argv[1], 'rb') 
memory = f.read()

ser.write(b'W')
ser.flush()

databytes=bytearray(memory)
ser.write(databytes)     # write a string
ser.flush()

f.close()

ser.close()

