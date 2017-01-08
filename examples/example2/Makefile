#######################################################################################

# environment variable of the current user to locate the AVR8 toolchain
AVRPATH = $(AVR8TOOLCHAINBINDIR)

# the type of avr microcontroller
DEVICE = atxmega128a3u
DEVICE = atxmega384c3
DEVICE = atmega2560
DEVICE = atmega8
EFUSE  = ""
HFUSE  = 0xd9
LFUSE  = 0xe1

# the frequency the microcontroller is clocked with
F_CPU = 16000000

# extra data section
# DEFINES += -DEXTFUNC_NOEXT=0
# DEFINES += -DCPUCONTEXT_EXTRASYMBOLS=1
# DEFINES += -DEXTFUNCFAR=__attribute__\ \(\(section\ \(\".farfunc\"\)\)\) -Wl,--section-start=.farfunc=0x1300
# DEFINES += -DEXTFUNCNEAR=PROGMEM
# DEFINES += -DEXTFUNCNEAR=__attribute__\ \(\(section\ \(\".nearfunc\"\)\)\) -Wl,--section-start=.nearfunc=0x1000

# DEFINES += -D__AVR_LIBC_DEPRECATED_ENABLE__
# DEFINES += -DDATASECTION=__attribute__\ \(\(section\ \(\".extradata\"\)\)\)
# LDFLAGS += -Wl,--section-start=.extradata=0x6000

# where the firmware should be located within the flashmemory (in case you trampoline)
FLASHADDRESS = 0x0000

# (not important for compiling) - the device transporting firmware into the controller
PROGRAMMER = -c usbasp

#######################################################################################



# Tools:
ECHO=@echo
GCC=gcc
RM=@rm -f

DOX=@doxygen

CC=$(AVRPATH)avr-gcc
OBC=@$(AVRPATH)avr-objcopy
OBD=@$(AVRPATH)avr-objdump
SIZ=@$(AVRPATH)avr-size

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
AVRDUDE_FUSE = -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m
ifneq ($(EFUSE), "")
AVRDUDE_FUSE += -U efuse:w:$(EFUSE):m
endif


MYCFLAGS = -Wall -g3 -ggdb -Os -fno-move-loop-invariants -fno-tree-scev-cprop -fno-inline-small-functions -ffunction-sections -fdata-sections -I. -I../../include/ -mmcu=$(DEVICE) -DF_CPU=$(F_CPU) $(CFLAGS)   $(DEFINES)
MYLDFLAGS = -Wl,--relax,--gc-sections $(LDFLAGS)


FLASHPREAMBLEDEFINE = 
ifneq ($(FLASHADDRESS), 0)
ifneq ($(FLASHADDRESS), 00)
ifneq ($(FLASHADDRESS), 000)
ifneq ($(FLASHADDRESS), 0000)
ifneq ($(FLASHADDRESS), 00000)
ifneq ($(FLASHADDRESS), 0x0)
ifneq ($(FLASHADDRESS), 0x00)
ifneq ($(FLASHADDRESS), 0x000)
ifneq ($(FLASHADDRESS), 0x0000)
ifneq ($(FLASHADDRESS), 0x00000)
FLASHPREAMBLE = 0x0000
FLASHPREAMBLEDEFINE = -DFLASHPREAMBLE=$(FLASHPREAMBLE)
MYLDFLAGS += -Wl,--section-start=.text=$(FLASHADDRESS)
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif




STDDEP	 = ../../include/*.h
EXTRADEP = Makefile


all: main.hex eeprom.hex main.bin eeprom.bin main.asm main.S



main.o: main.c $(STDDEP) $(EXTRADEP)
	$(CC) main.c -c -o main.o $(MYCFLAGS)

extfunc.o: ../../source/extfunc.c $(STDDEP) $(EXTRADEP)
	$(CC) ../../source/extfunc.c -c -o extfunc.o $(MYCFLAGS)

cpucontext.o: ../../source/cpucontext.c $(STDDEP) $(EXTRADEP)
	$(CC) ../../source/cpucontext.c -c -o cpucontext.o $(MYCFLAGS)





MYOBJECTS = main.o extfunc.o cpucontext.o
main.elf: $(MYOBJECTS) $(STDDEP) $(EXTRADEP)
	$(CC) $(MYOBJECTS) -o main.elf $(MYCFLAGS) -Wl,-Map,main.map $(MYLDFLAGS)
	$(ECHO) "."
	$(SIZ) main.elf
	$(ECHO) "."

main.asm: main.elf $(STDDEP) $(EXTRADEP)
	$(OBD) -d main.elf > main.asm

main.S: main.elf $(STDDEP) $(EXTRADEP)
	$(OBD) -dS main.elf > main.S

main.hex: main.elf $(STDDEP) $(EXTRADEP)
	$(OBC) -R .eeprom -R .fuse -R .lock -R .signature -O ihex main.elf main.hex

eeprom.hex: main.elf $(STDDEP) $(EXTRADEP)
	$(OBC) -j .eeprom -O ihex main.elf eeprom.hex

main.bin: main.elf $(STDDEP) $(EXTRADEP)
	$(OBC) -R .eeprom -R .fuse -R .lock -R .signature -O binary main.elf main.bin

eeprom.bin: main.elf $(STDDEP) $(EXTRADEP)
	$(OBC) -j .eeprom -O binary main.elf eeprom.bin

disasm: main.elf $(STDDEP) $(EXTRADEP)
	$(OBD) -d main.elf

fuse:
	$(ECHO) "."
	$(AVRDUDE) $(AVRDUDE_FUSE)
	$(ECHO) "."

flash: all
	$(ECHO) "."
	$(AVRDUDE) -U flash:w:main.hex:i
	$(ECHO) "."

eeprom: all
	$(ECHO) "."
	$(AVRDUDE) -D -U eeprom:w:eeprom.hex:i
	$(ECHO) "."

deepclean: clean
	$(RM) *~

clean:
	$(RM) *.bin
	$(RM) *.hex
	$(RM) *.map
	$(RM) *.asm
	$(RM) *.S
	$(RM) *.o
	$(RM) *.elf
