#(c) 2024  David Shadoff
import serial
import sys
import time

# Notes:
#
# This program will pipe the FX devcart's FIFO to a file
#
#   Usage: fifo2file <output_file> [COM port]
#
#   Example:
#     python fifo2file.py fifo.txt COM3
#

if ((len(sys.argv) != 3) and (len(sys.argv) != 2)):
    print("Usage: fifo2file <output_file> [COM port]")
    exit()

ser = serial.Serial()
ser.baudrate = 115200

# Add override for windows-style COM ports.
if (len(sys.argv) == 3):
    ser.port = sys.argv[2]
else:
    ser.port = '/dev/ttyUSB0'

ser.open()

# open file initially (truncate if exists)

f = open(sys.argv[1], 'wb') 
f.close()

while True:
    ser.write(b'/')             # one-byte command to fetch FIFO contents
    ser.flush()

    size = int.from_bytes(ser.read(2), "little")
    print('size =', size)

    if (size != 0):
        f = open(sys.argv[1], 'ab') # append to the file for each set of data
        memory = ser.read(size)     # read the whole (current) blob
        f.write(memory)
        f.close()
        print(memory)
    else:
        time.sleep(1)               # wait for next fetch, only if you didn't find anything

ser.close()

