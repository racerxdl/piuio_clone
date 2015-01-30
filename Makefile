#/***********************************************************/
#/*   ____ ___ _   _ ___ ___     ____ _                     */
#/*  |  _ \_ _| | | |_ _/ _ \   / ___| | ___  _ __   ___    */
#/*  | |_) | || | | || | | | | | |   | |/ _ \| '_ \ / _ \   */
#/*  |  __/| || |_| || | |_| | | |___| | (_) | | | |  __/   */
#/*  |_|  |___|\___/|___\___/   \____|_|\___/|_| |_|\___|   */
#/*                                                         */
#/*  By: Lucas Teske                                        */
#/***********************************************************/
#/*     Basicly this is an PIUIO Clone with an ATMEGA328    */
#/*     It uses V-USB for the USB Interface and few chips   */
#/***********************************************************/
#/*                    License is GPLv3                     */
#/*  Please consult https://github.com/racerxdl/piuio_clone */
#/***********************************************************/
DEVICE  = atmega328

F_CPU   = 16000000	# in Hz
AVRDUDE = avrdude -c usbasp -p m328p # edit this line for your programmer

CFLAGS  = -Iusbdrv -I. -DDEBUG_LEVEL=0
OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE)

##############################################################################
# Fuse values for particular devices
##############################################################################
# If your device is not listed here, go to
# http://palmavr.sourceforge.net/cgi-bin/fc.cgi
# and choose options for external crystal clock and no clock divider
#

################################ ATTiny2313 #################################
# ATTiny2313 FUSE_L (Fuse low byte):
# 0xef = 1 1 1 0   1 1 1 1
#        ^ ^ \+/   \--+--/
#        | |  |       +------- CKSEL 3..0 (clock selection -> crystal @ 12 MHz)
#        | |  +--------------- SUT 1..0 (BOD enabled, fast rising power)
#        | +------------------ CKOUT (clock output on CKOUT pin -> disabled)
#        +-------------------- CKDIV8 (divide clock by 8 -> don't divide)
# ATTiny2313 FUSE_H (Fuse high byte):
# 0xdb = 1 1 0 1   1 0 1 1
#        ^ ^ ^ ^   \-+-/ ^
#        | | | |     |   +---- RSTDISBL (disable external reset -> enabled)
#        | | | |     +-------- BODLEVEL 2..0 (brownout trigger level -> 2.7V)
#        | | | +-------------- WDTON (watchdog timer always on -> disable)
#        | | +---------------- SPIEN (enable serial programming -> enabled)
#        | +------------------ EESAVE (preserve EEPROM on Chip Erase -> not preserved)
#        +-------------------- DWEN (debug wire enable)


# symbolic targets:
help:
	@echo "This Makefile has no default rule. Use one of the following:"
	@echo "make hex ....... to build main.hex"
	@echo "make program ... to flash fuses and firmware"
	@echo "make fuse ...... to flash the fuses"
	@echo "make flash ..... to flash the firmware (use this on metaboard)"
	@echo "make clean ..... to delete objects and hex file"

hex: main.hex

program: flash fuse

# rule for programming fuse bits:
fuse:
	#@[ "$(FUSE_H)" != "" -a "$(FUSE_L)" != "" ] || \
	#	{ echo "*** Edit Makefile and choose values for FUSE_L and FUSE_H!"; exit 1; }
	$(AVRDUDE) -U hfuse:w:0xd9:m -U lfuse:w:0xff:m 

# rule for uploading firmware:
flash: main.hex
	$(AVRDUDE) -U flash:w:main.hex:i
	chmod 777 -R *

# rule for deleting dependent files (those which can be built by Make):
clean:
	rm -f main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.elf *.o usbdrv/*.o main.s usbdrv/oddebug.s usbdrv/usbdrv.s

# Generic rule for compiling C files:
.c.o:
	$(COMPILE) -c $< -o $@

# Generic rule for assembling Assembler source files:
.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

# Generic rule for compiling C to assembler, used for debugging only.
.c.s:
	$(COMPILE) -S $< -o $@

# file targets:

main.elf: usbdrv $(OBJECTS)	# usbdrv dependency only needed because we copy it
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size main.hex

# debugging targets:

disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c


