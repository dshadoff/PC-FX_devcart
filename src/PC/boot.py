#(c) 2024  David Shadoff
import serial
import os
import sys
import time

# Notes:
#
# This program will write a bottable program to the PC-FX's external Backup memory card
#
#   Usage: boot <input_file> [COM port]
#
#   Example:
#     python boot.py client COM3
#

if ((len(sys.argv) != 3) and (len(sys.argv) != 2)):
    print("Usage: boot <input_file> [COM port]")
    exit()


ser = serial.Serial()
ser.baudrate = 115200


# Add override for windows-style COM ports.
if (len(sys.argv) == 3):
    ser.port = sys.argv[2]
else:
    ser.port = '/dev/ttyUSB0'

# Check that the file is exactly 128KB in size:
#
file_stat = os.stat(sys.argv[1])
filesize = file_stat.st_size

ser.open()

f = open(sys.argv[1], 'rb') 
memory = f.read()

ser.write(b'B')
ser.flush()

ser.write(filesize.to_bytes(4,'little'))
ser.flush()

databytes=bytearray(memory)
ser.write(databytes)     # write a string
ser.flush()

f.close()

ser.close()

