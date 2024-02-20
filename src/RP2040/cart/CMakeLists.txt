cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(fxdevcart)

# Initialize the SDK
pico_sdk_init()

add_executable(fxdevcart main.c)

target_link_libraries(fxdevcart 
	pico_stdlib
	pico_multicore
# 	hardware_pio
	)

# enable usb output, disable uart output
pico_enable_stdio_usb(fxdevcart 1)
pico_enable_stdio_uart(fxdevcart 0)

pico_add_extra_outputs(fxdevcart)

# Don't execute from Flash; keep it pinned in SRAM
#
# pico_set_binary_type(fxdevcart copy_to_ram)
