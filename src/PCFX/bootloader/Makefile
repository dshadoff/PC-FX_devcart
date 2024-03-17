# Makefile for dev cart bootloader
#
#(c) 2024 David Shadoff

bootloader: bootloader.s
	v810-as bootloader.s -a=bootloader.lst -o bootloader.o
	v810-ld bootloader.o -o bootloader.linked
	v810-objcopy  bootloader.linked -O binary bootloader
	

cd: bootloader.cue

bootloader.cue: cdlink_bootloader.txt bootloader 
	pcfx-cdlink cdlink_bootloader.txt bootloader
	
run: bootloader.cue
	$(EMU) bootloader.cue
		
	
dump: bootloader.o
	v810-objdump -d bootloader.o

clean:
	rm -f bootloader bootloader.cue bootloader.bin *.o bootloader.lst bootloader.linked
