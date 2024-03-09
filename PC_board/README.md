# PC-FX_devcart PC Board

This is the PC board design of a development cart for the PC-FX, based on the RP2040 microcontroller.

##  Prototype Board

This is the initial board, using an Olimex RP2040-Pico30 board, and some nylon standoffs
(5mm on bottom, 7mm on top).  In future, there will a new design for the board which won't
need a daughterboard, and will have plans for a proper 3D printed case.

![Prototype Board](images/devcart_proto.png)


## Overview

The overall board design was based on my earlier PC-FX_nvBMP board using EAGLE.

The prototype board uses the Olimex RP2040-Pico30 board, because it is an easy-to-use,
self-contained module which also exposes all 30 of the GPIOs (which most boards don't do).

One major difference from the PC-FX_nvBMP board, is that the 3.3V power is not derived from
PC-FX's power: The RP2040 module is powered from the USB host, and an on-board 3.3V regulator
provides the 3.3V rail for the module and the 3.3V side of the level-shifters.  Only the 5V side
of the level-shifters is supplied by the PC-FX (however, they share a common ground).

## Files

EAGLE was used for the prototype board, and all design files are included.

Gerber files are supplied in the relevant 'xxx_gerbers.zip' file.

If you would like to use JLCPCB's SMT Assembly service, a 'BOM.csv' (BOM/bill-of-materials)
file and 'assembly.csv' (CPL/component positioning and layout) files are also included.

