# PC-FX_Devcart source - PC-FX

This is sample code to include in your PC-FX project, in order to use the FIFO port.
The sample file is intermingled comments and code snippets which explain what components
are needed to intiialize and to interact with the FIFO port (but not a sample program).

# Program Files Overview

fifoport.s - This is a set of functions in V810 assembly which access the actual hardware memory
locations directly
fifoport.h - This should be included in your program in order to make use of the FIFO functions above
sample.c - This is a couple of sample functions to use in your program, as well as a reminder to
unlock/allow access to the BMP memory region

## Development Chain & Tools

This was written using a version of gcc for V810 processor, with 'pcfxtools' which assist in
building executables for PC-FX, and 'liberis' which is a library of functions targetting the PC-FX.

These can be found here:\
![https://github.com/jbrandwood/v810-gcc](https://github.com/jbrandwood/v810-gcc)\
![https://github.com/jbrandwood/pcfxtools](https://github.com/jbrandwood/pcfxtools)\
![https://github.com/jbrandwood/liberis](https://github.com/jbrandwood/liberis)

