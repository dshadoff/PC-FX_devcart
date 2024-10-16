#!/bin/sh
cmake -DFAMILY=rp2040 -DBOARD=raspberry_pi_pico -B cmake_pico1
cmake -DFAMILY=rp2350-arm-s -DPICO_BOARD=pico2 -B cmake_pico2
