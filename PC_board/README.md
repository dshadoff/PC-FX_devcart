# PC-FX_devcart PC Board

This is the PC board design of a development cart for the PC-FX, based on the RP2040 microcontroller.

##  Current version

The current version is Ver2_RevB.
[(For the Protoype board using an Olimex RP2040-Pico30 module, please click here.)](README_protoype.md)

The only changes from RevA to Rev B are related to schematic annotations, and a minor labelling update ont eh PC Board itself,
so the picture of RevA is included here.

![Current version](../images/devcart_ver2_reva.jpg)


## Overview

The board design was a ground-up design for 4-layer board using KiCAD 8.0.4 .

As this was my first KiCAD design of a reasonable complexity, I was happy to receive some guidance
and schematic updates from [Regis Galland](https://github.com/rgalland).

The board is based around a RP2040 microcontroller, and its "minimal design", as shown in the Raspberry Pi
[Hardware Design With RP2040 Design Guide](https://datasheets.raspberrypi.com/rp2040/hardware-design-with-rp2040.pdf).

The bus decoding was based on my earlier work with the [PC-FX_nvBMP board](https://github.com/pcfx-devel/PC-FX-NVBMP).

The 3.3V power is not derived from PC-FX's power: The RP2040 is powered from the USB host, and an on-board
3.3V regulator provides the 3.3V rail for the module and the 3.3V side of the level-shifters.

Only the 5V side of the level-shifters is supplied by the PC-FX (however, they share a common ground).

This means that:
 1) The FX-devcart board will not function correctly when not connected to USB, as this is the main power source.
 2) The FX-devcart can be programmed while inserted into the PC-FX, while the PC-FX is not turned on - this is
important for the ability to "boot" code from teh FX-devcart


## Files

KiCAD was used for designing the board, and all design files are included.
(FX-Dev_Ver2_revB.zip is a KiCAD Project archive, including all relevant footprints and datasheets).

In order to manufacture the board, the key files are in the "production" subfolder:
 - Gerber files are supplied in the relevant 'Gerbersi_xxx.zip' file.
 - bom.csv is the bill-of-materials file, prepared in the format for JLCPCB's assembly service
 - positions.csv is the pick-and-place part positioning file, prepared in the format for JLCPCB's assembly service

