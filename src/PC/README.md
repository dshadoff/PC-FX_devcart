# PC-FX_Devcart source

This contains small programs written in Python which will interact with the developnemt cart,
ending/receiving backup memory files, preparing programs to be bootable software, or reading
from the FIFO port.

## boot.py

This will probably be the most-commonly-used program for PC-FX developers, for testing
their programs on real hardware by booting into them.

```
Usage: python boot.py <input_file> [COM port]
```

With the PC-FX off and the development cartridge inserted into the FX-BMP port, connect
a PC to the development cart and send a boot program.  Once sent, the Python script
will print:
```
You can boot the PC-FX now
```
...At which point, booting the PC-FX will start the program.

NOTE: Programs larger than 128KB can also be downloaded to the PC-FX; in those cases,
the initial boot program is a program which will continue to request blocks of data 
until program download is complete, and automatically invoke the downloaded program.


## getbmp.py

With the cartridge in the FX-BMP port and connected to the PC, you can format the external
cartridge memory and move game save files to it (from the console internal memory), using
the file manager portion of the boot ROM.

Running this command will transmit that data to the host PC.

It is important to remember that the devcart does not retain data when the power (from USB)
is removed, so don't try to use the dev cart for any permanent storage.

```
Usage: python getbmp.py <output_file> [COM port]
```

## putbmp.py

With the cartridge in the FX-BMP port and connected to the PC, you can move game save data
images to the external cartridge memory, and subsequently move game save files from there to the
console internal memory), using the file manager portion of the boot ROM.

Running this command will transmit that data from the host PC.

It is important to remember that the devcart does not retain data when the power (from USB)
is removed, so don't try to use the dev cart for any permanent storage.

```
Usage: python putbmp.py <input_file> [COM port]
```

