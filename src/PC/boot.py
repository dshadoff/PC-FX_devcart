#(c) 2024  David Shadoff
import serial
import os
import sys
import time

# If the bootloader needs to deploy a large file, this is
# the blocksize it will use
BLOCKSIZE = 65536

SEND_BOOT           = b'B'
SEND_BLOCK_FIRST    = b'B'  # same as SEND_BOOT
SEND_BLOCK_CONTINUE = b'1'
SEND_BLOCK_FINAL    = b'2'

def drain_fifo():
    ser.write(b'/')             # one-byte command to fetch FIFO contents
    ser.flush()
    size = int.from_bytes(ser.read(2), "little")
    if (size != 0):
        memory = ser.read(size)     # read the whole (current) blob



def wait_ack():
    timer = time.time()
    warned = False
    while (True):
        if ((time.time() > (timer + 1)) and (warned == False)):
            print("...Waiting for response from PC-FX", end = " ", flush=True)
            warned = True

        ser.write(b'/')             # one-byte command to fetch FIFO contents
        ser.flush()

        size = int.from_bytes(ser.read(2), "little")
        # print('size =', size)

        if (size != 0):
            memory = ser.read(size)     # read the whole (current) blob
            if (memory == b'2'):
                print("Okey-dokey")
            #print(memory)
            break
        else:
            time.sleep(0.2)               # wait for next fetch, only if you didn't find anything


def send_memory(send_type, mem_block):
    ser.write(send_type)
    ser.flush()

    ser.write(len(mem_block).to_bytes(4,'little'))  # write the size of the block of memory
    ser.flush()

    databytes=bytearray(mem_block)
    ser.write(databytes)                           # write the block of memory
    ser.flush()


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

# Check that the file is below 100KB in size:
#
file_stat = os.stat(sys.argv[1])
filesize = file_stat.st_size

simplepath = 0
if (filesize < 102400):
    simplepath = 1

ser.open()

f = open(sys.argv[1], 'rb') 
memory = f.read()
f.close()

if (simplepath == 1):
    send_memory(SEND_BOOT, memory)
    print("You can boot the PC-FX now")

else:
    # Deploy bootloader first
    fboot = open("bootloader", 'rb') 
    bootload = fboot.read()
    fboot.close()

    send_memory(SEND_BOOT, bootload)
    print("You can boot the PC-FX now")

    time.sleep(5)
    wait_ack()
    print("...Received")

    count = 0
    send_type = SEND_BLOCK_FIRST

    # wait for confirmation back from FX FIFO port
    start = time.time()*1000.0
    while (count < filesize):
        if ((filesize - count) > BLOCKSIZE):
            datablock = memory[count:(count+BLOCKSIZE)]
            send_memory(send_type, datablock)
            send_type = SEND_BLOCK_CONTINUE
            count = count + BLOCKSIZE
#            print(memory[count:count+1], end = " ")
            print("sent {0:3}KB".format(int(count/1024)), end = " ", flush=True)
            # wait acknowledge
            wait_ack()
            print("...Received")
        else:
            datablock = memory[count:]
            send_memory(SEND_BLOCK_FINAL, datablock)
            # send_memory(send_type, datablock)
            count = filesize
            print("sent {0:3}KB".format(int(count/1024)))

    # after end block, end timer and report on time
    #
    end = time.time()*1000.0
    elapsed = end-start
    print("DONE")
    if (elapsed != 0):
        speed = filesize/(end-start)
        print("elapsed = ", int(elapsed/1000), "seconds. ", speed, ' KB/s')
        
    

ser.close()

