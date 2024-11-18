# PC-FX_devcart Enclosure

This is the 3D Print design for an enclosure for the development cart for the PC-FX.


## Introduction

Once the FX-Devcart PC board was designed and tested, an enclosure was needed in order to protect it, as well
as to align the card edge with the slot, and allow easy insertion and removal.

This design was made in two halves, both of which are described within the same design file.


##  Design Tool - Why OpenSCAD ?

While most people study 3D design in Blender or Fusion 360, I chose the OpenSCAD for my design.
The reasons are as follows:
 1. OpenSCAD is a free tool and is cross-platform.
 2. As it is code-based, it's easier for a programmer type (like myself) to become comfortable with.
 3. As it's basically code, it should be straightforward to make make edits to any portion of the design at a later date, without needing much effort.
 4. It's simple to have both halves of the case in a single design, driven by a variable - ensuring that dimensions match.


## Overview

The case design was a ground-up design using OpenSCAD 2021.01 .

As the PC Board has 3 LEDs on it next to the USB connector, the case should ideally be printed in clear resin. However,
the LED functions aren't critical to the operation of the board, so a simple 3D Print should suffice.

The case should be held together with 4 M2-13mm screws (and matching nuts).

In OpenSCAD, you can visualize different views of the model by manipulating the variables at the top of the file:
```
show_half = 1;      // 0 = bottom;     1 = top
show_board = 0;     // 0 = don't show; 1 = show 
```
You can enable either the bottom half or the top half (and render in preparation for STL output); you can also
show the PC board (an STL visualization of the board is also included, to check alignment of various features).


## Files

```
FXDev_Case.scad    - This is the master design (or 'program' if you prefer)

FX_Dev_Bottom.stl  - These files can be used in a slicer for 3D at home, or submitted to a factory like JLCPCB
FX_Dev_Top.stl

Devboard.stl       - This is an STL visualization of the PC Board (Ver2, revB), to check alignment
```

