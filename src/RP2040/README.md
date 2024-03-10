# PC-FX_Devcart source - RP2040

This section contains the program that runs on the development cartridge's microcontroller,
and interacts with both the I/O bus and the USB port.

# Folders Overview

## boards

Developmnet board capabilities are indicated by a '.h' file describing each board's capabilities,
and are generally found in the {pico-sdk}/src/boards/include/boards directory in the piso-sdk tree.

The original Pico board actually doesn't expose all of the GPIOs, and dedicates some of the GPIOs
to use in different purposes.  As this board needs essentially all GPIOs to be employed for memory
addressing, a board which revealed all 30 GPIOs was needed - and a board description file (the '.h'
file mentioned above) to match.

For the prototype, I ended up using the Olimex RP2040-PICO30 board, which doesn't have its own board
definition file, so I altered an existing file to create one.  This is the 'olimex_rp2040-pico30.h'
file in the 'boards' folder.

## cart

This folder contains the source code for the development cart.
The 'RELEASE' folder includes a '.uf2' file which can be deployed directly on the cart.

## test_device

Through the process of initial build and responsiveness testing, I used 2 Raspberry Pi Pico
boards to test each other (but with a more limited address space).

This 'test_device' was set up to send the signals (with approximate timings) to the device under test,
in order to measure the response timing and accuracy of the results.  GPIOs 0 through 22, as well as
26, 27, and 28 on the test device were connected to the same GPIOs on the device under test, with logic
state analyzer set up to measure the timing of several data lines, several address lines, and all control
lines, in order to validate timing.

# Development Chain & Tools

This was all written using pico-sdk version 1.5.0 and the matching/included version of TinyUSB.
This may be updated for later revisions as they become available, especially if useful new features
become available.

