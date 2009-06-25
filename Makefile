#########  AVR Project Makefile Template   #########
######                                        ######
######    Copyright (C) 2003-2005,Pat Deegan, ######
######            Psychogenic Inc             ######
######          All Rights Reserved           ######
######                                        ######
###### You are free to use this code as part  ######
###### of your own applications provided      ######
###### you keep this copyright notice intact  ######
###### and acknowledge its authorship with    ######
###### the words:                             ######
######                                        ######
###### "Contains software by Pat Deegan of    ######
###### Psychogenic Inc (www.psychogenic.com)" ######
######                                        ######
###### If you use it as part of a web site    ######
###### please include a link to our site,     ######
###### http://electrons.psychogenic.com  or   ######
###### http://www.psychogenic.com             ######
######                                        ######
####################################################

MCU            = atmega168
PROGRAMMER_MCU = atmega168
F_CPU          = 8000000
AVRDUDE_PORT   = /dev/tty.usbserial-A900a50a

# exectuables

CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
SIZE=avr-size
AVRDUDE=avrdude
REMOVE=rm -f

# linker
LDFLAGS=-Llib -Wl,-Map,$(TRG).map -mmcu=$(MCU) -Llib -lm $(LIBS)  

# assembler
ASMFLAGS =-I. $(INC) -mmcu=$(MCU)        \
	-x assembler-with-cpp            \
	-Wa,-gstabs,-ahlms=$(firstword   \
		$(<:.S=.lst) $(<.s=.lst))

# compiler
CFLAGS=-I. $(INC) -g -mmcu=$(MCU) -Os -DF_CPU=$(F_CPU) \
	-fpack-struct -fshort-enums             \
	-funsigned-bitfields -funsigned-char    \
	-Wall -Wstrict-prototypes -std=gnu99    \
	-Wa,-ahlms=$(firstword                  \
	$(filter %.lst, $(<:.c=.lst)))

.SUFFIXES : .c .cc .cpp .C .o .out .s .S \
	.hex .ee.hex .h .hh .hpp

all: bin/Shwatt.out

MYOBJECTS = Shwatt.o Globals.o ShmittTrigger.o 

MyLibs = lib/libMyMath.a lib/libCalibrate.a lib/libDAQ.a

#####################################################################################################
#####################   Build math libraries                                 ########################
#####################################################################################################
MATHSRC = Math/FastSqrt.c Math/FractSupport.c Math/TrigLookup.c 
MATHOBJ = $(MATHSRC:.c=.o)

lib/libMyMath.a: $(MATHOBJ) Math/AbsFract.o
	avr-ar rcs $@ $(MATHOBJ) Math/AbsFract.o

Math/AbsFract.o: Math/AbsFract.S Math/fixdef.h Math/asmdef.h 
	$(CC) $(ASMFLAGS) -c Math/AbsFract.S -o $@

#dependencies
Math/TrigLookup.o: Math/TrigLookup.h
Math/FastSqrt.o: Math/TrigLookup.h
Math/FractSupport.o: Math/FractSupport.h

#####################################################################################################
#####################   Build DAQ libraries                                  ########################
#####################################################################################################
DAQSRC = DAQ/Clock.c DAQ/ShwattDAQ.c DAQ/ShmittTrigger.c
DAQOBJ = $(DAQSRC:.c=.o)
lib/libDAQ.a: $(DAQOBJ) 
	avr-ar rcs $@ $(DAQOBJ) 

$(DAQOBJ): DAQ/Clock.h DAQ/ShwattDAQ.h Core/ShwattGlobals.h Core/HardwareData.h Math/FractSupport.h

#####################################################################################################
#####################   Build XBee libraries                                  ########################
#####################################################################################################
XBEESRC = XBee/XBee.c XBee/XBeeCommands.c
XBEEOBJ = $(XBEESRC:.c=.o)
lib/libXBee.a: $(XBEEOBJ) 
	avr-ar rcs $@ $(XBEEOBJ) 

$(XBEEOBJ): Math/FractSupport.h Core/ShwattGlobals.h XBee/XBee.h Core/Parameters.h

#####################################################################################################
#####################   Calibration libraries                                ########################
#####################################################################################################
CALIBSRC = Calibration/Calibrate.c Calibration/CalibrateAverageVariance.c \
	   Calibration/checkSideUp.c Calibration/FakeCalibrate.c
CALIBOBJ = $(CALIBSRC:.c=.o)
lib/libCalib.a: $(CALIBOBJ)
	avr-ar rcs $@ $(CALIBOBJ)

$(CALIBOBJ): Calibration/Calibrate.h Core/ShwattGlobals.h \
             Math/FractSupport.h DAQ/Clock.h Algorithms/Kalman.h \
	     DAQ/ShwattDAQ.h Core/Parameters.h

#####################################################################################################
#####################   Algorithm   libraries                                ########################
#####################################################################################################
ALGOSRC = Algorithms/Kalman.c
ALGOOBJ = $(ALGOSRC:.c=.o)
lib/libAlgos.a: $(ALGOOBJ)
	avr-ar rcs $@ $(ALGOOBJ)
$(ALGOOBJ): Algorithms/Kalman.h Core/ShwattGlobals.h Core/HardwareData.h Math/TrigLookup.h

#####################################################################################################
#####################   Code entry point                                     ########################
#####################################################################################################
## Main objects ##
Core/ShwattGlobals.o: Core/ShwattGlobals.h Core/HardwareData.h Core/Parameters.h
Shwatt.o: Calibration/Calibrate.h \
   	  DAQ/ShmittTrigger.h \
	  Algorithms/Kalman.h \
	  XBee/XBee.h \
	  Math/FractSupport.h \
	  DAQ/Clock.h \
	  Core/ShwattGlobals.h

all: bin/Shwatt.out

#####################################################################################################
#####################   Link everything into executable
#####################################################################################################
MYLIBS = lib/libCalib.a lib/libXBee.a lib/libDAQ.a lib/libMyMath.a lib/libAlgos.a
LIBS = -lCalib -lXBee -lDAQ -lAlgos -lMyMath 
MYOBJS = Core/ShwattGlobals.o Shwatt.o
ALLDEPS = $(MYLIBS) $(MYOBJS)

# link everything into executable
bin/Shwatt.out: $(ALLDEPS)
	$(CC) $(LDFLAGS) -o bin/Shwatt.out $(MYOBJS) $(LIBS)

# format into hex for upload
hex: bin/Shwatt.hex bin/Shwatt.ee.hex

#####################################################################################################
#####################   Program device                 
#####################################################################################################
AVRDUDE_PROGRAMMERID=stk500v1
UPLOAD_RATE = 19200

write: hex
	$(AVRDUDE) -V -F -C /Applications/Arduino/hardware/tools/avr/etc/avrdude.conf -c $(AVRDUDE_PROGRAMMERID)   \
	 -p $(PROGRAMMER_MCU) -P $(AVRDUDE_PORT) -e        \
	 -U flash:w:bin/Shwatt.hex -b $(UPLOAD_RATE)
size: bin/Shwatt.out
	avr-size bin/Shwatt.out

stats: bin/Shwatt.out
	$(OBJDUMP) -h bin/Shwatt.out

asm: bin/Shwatt.out
	rm -rf DisAsm.s
	$(OBJDUMP) -d bin/Shwatt.out > DisAsm.s


#### Compile object from source
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

#### Generating hex files ####
# hex files from elf
#####  Generating a gdb initialisation file    #####
HEXFORMAT=ihex
.out.hex:
	$(OBJCOPY) -j .text                    \
		-j .data                       \
		-O $(HEXFORMAT) $< $@

.out.ee.hex:
	$(OBJCOPY) -j .eeprom                  \
		--change-section-lma .eeprom=0 \
		-O $(HEXFORMAT) $< $@


#####################################################################################################
#####################   Clean up                      
#####################################################################################################

clean:
	rm -rf **/*.o 
	rm -rf **/*.lst 
	rm -rf lib/*.a
	rm -rf bin/*
	rm -rf Shwatt.o
	rm -rf Shwatt.lst
	rm -rf DisAsm.s

