cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(fxcarttest)

# Initialize the SDK
pico_sdk_init()

add_executable(fxcarttest main.c)

target_link_libraries(fxcarttest 
	pico_stdlib
	pico_multicore
	)

# enable usb output, disable uart output
pico_enable_stdio_usb(fxcarttest 1)
pico_enable_stdio_uart(fxcarttest 0)

pico_add_extra_outputs(fxcarttest)

# Don't execute from Flash; keep it pinned in SRAM
#
pico_set_binary_type(fxcarttest copy_to_ram)
