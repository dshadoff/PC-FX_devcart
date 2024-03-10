# PC-FX_Devcart source

This section contains all the code related to the PC-FX devcart, divided into three sections:

### PC

This contains small program written in Python which will interact with the developmnet cart,
ending/receiving backup memory files, preparing programs to be bootable software, or reading
from the FIFO port.

### RP2040

This section contains the program that runs on the development cartridge's microcontroller,
and interacts with both the I/O bus and the USB port.

### PC-FX

This is sample code to include in your PC-FX project, in order to use the FIFO port.
The sample file is intermingled comments and code snippets which explain what components
are needed to intiializae and to interact with the FIFO port (but not a sample program).

