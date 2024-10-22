###############################################################################
# Makefile for the project TowerOfRubble
###############################################################################

## General Flags
PROJECT = TowerOfRubble
GAME = TowerOfRubble
MCU = atmega644
TARGET = $(GAME).elf
CC = avr-gcc
INFO=../gameinfo.properties
UZEBIN_DIR = ../../../bin

## Kernel settings
KERNEL_DIR = ../../../kernel

## Escape spaces in mixer path (due to custom sounds.inc)
EMPTY:=
SPACE:= $(EMPTY) $(EMPTY)
SPACE_ESC:= \\$(SPACE)
MIX_PATH:= $(realpath ../data/sounds.inc)
MIX_PATH_ESC:= $(subst $(SPACE),$(SPACE_ESC),$(MIX_PATH))

KERNEL_OPTIONS  = -DMAP_COMPRESSION=1 -DVIDEO_MODE=3 -DSCREEN_TILES_V=24 -DVRAM_TILES_V=32 -DFIRST_RENDER_LINE=36 #-DRESOLUTION_EXT=1 -DVRAM_TILES_H=32
KERNEL_OPTIONS += -DRAM_TILES_COUNT=36 -DINTRO_LOGO=0 -DTRANSLUCENT_COLOR=0xC7 -DTRUE_RANDOM_GEN=1
KERNEL_OPTIONS += -DMUSIC_ENGINE=STREAM -DSOUND_MIXER=1 -DSOUND_CHANNEL_5_ENABLE=0 -DMIXER_WAVES=\"$(MIX_PATH_ESC)\"
KERNEL_OPTIONS += -DNO_EEPROM_FORMAT=1 -DNO_PC_SLIDE=1 -DNO_PC_LOOP=1 -DNO_CHAN_EXPRESSION=1

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=28636360UL -Os -fsigned-char -ffunction-sections -fno-toplevel-reorder
CFLAGS += -fdata-sections -ffreestanding -fomit-frame-pointer -fmerge-all-constants
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 
CFLAGS += $(KERNEL_OPTIONS)

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map=$(GAME).map 
LDFLAGS += -Wl,-gc-sections


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = uzeboxVideoEngineCore.o uzeboxCore.o uzeboxSoundEngine.o uzeboxSoundEngineCore.o uzeboxVideoEngine.o bootlib.o spiram.o $(GAME).o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Include Directories
INCLUDES = -I"$(KERNEL_DIR)" 

## Resources
RESOURCES = ../data/inc/tiles.inc ../data/inc/sprites.inc
RESOURCES += ../data/inc/da-logo-f0.inc ../data/inc/da-logo-f1.inc ../data/inc/da-logo-f2.inc ../data/inc/da-logo-f3.inc
RESOURCES += ../data/inc/da-logo-f4.inc ../data/inc/da-logo-f5.inc ../data/inc/da-logo-f6.inc
RESOURCES += TOR_DATA.BIN

## Build
all: $(RESOURCES) $(TARGET) $(GAME).hex $(GAME).eep $(GAME).lss $(GAME).uze size

## Rebuild graphics resources
../data/inc/tiles.inc: ../data/png/tiles.png ../data/xml/tiles.xml
	$(UZEBIN_DIR)/gconvert ../data/xml/tiles.xml

../data/inc/sprites.inc: ../data/png/sprites.png ../data/xml/sprites.xml
	$(UZEBIN_DIR)/gconvert ../data/xml/sprites.xml

../data/inc/da-logo-f0.inc: ../data/png/da-logo-f0.png ../data/xml/da-logo-f0.xml
	$(UZEBIN_DIR)/gconvert ../data/xml/da-logo-f0.xml

../data/inc/da-logo-f1.inc: ../data/png/da-logo-f1.png ../data/xml/da-logo-f1.xml
	$(UZEBIN_DIR)/gconvert ../data/xml/da-logo-f1.xml

../data/inc/da-logo-f2.inc: ../data/png/da-logo-f2.png ../data/xml/da-logo-f2.xml
	$(UZEBIN_DIR)/gconvert ../data/xml/da-logo-f2.xml

../data/inc/da-logo-f3.inc: ../data/png/da-logo-f3.png ../data/xml/da-logo-f3.xml
	$(UZEBIN_DIR)/gconvert ../data/xml/da-logo-f3.xml

../data/inc/da-logo-f4.inc: ../data/png/da-logo-f4.png ../data/xml/da-logo-f4.xml
	$(UZEBIN_DIR)/gconvert ../data/xml/da-logo-f4.xml

../data/inc/da-logo-f5.inc: ../data/png/da-logo-f5.png ../data/xml/da-logo-f5.xml
	$(UZEBIN_DIR)/gconvert ../data/xml/da-logo-f5.xml

../data/inc/da-logo-f6.inc: ../data/png/da-logo-f6.png ../data/xml/da-logo-f5.xml
	$(UZEBIN_DIR)/gconvert ../data/xml/da-logo-f6.xml

## Rebuild SD resources(Graphics and ADPCM sounds)
TOR_DATA.BIN: ../data/pcm/intro.raw ../data/pcm/title.raw ../data/pcm/map.raw ../data/inc/da-logo.inc ../data/sd-resource-gen.cfg
	$(UZEBIN_DIR)/dconvert ../data/sd-resource-gen.cfg


## Compile Kernel files
uzeboxVideoEngineCore.o: $(KERNEL_DIR)/uzeboxVideoEngineCore.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

uzeboxSoundEngineCore.o: $(KERNEL_DIR)/uzeboxSoundEngineCore.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

uzeboxCore.o: $(KERNEL_DIR)/uzeboxCore.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uzeboxSoundEngine.o: $(KERNEL_DIR)/uzeboxSoundEngine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uzeboxVideoEngine.o: $(KERNEL_DIR)/uzeboxVideoEngine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

spiram.o: $(KERNEL_DIR)/spiram.s $(DIRS)
	$(CC) $(INCLUDES) $(ASMFLAGS) -c $< -o $@

bootlib.o: $(KERNEL_DIR)/bootlib.s $(DIRS)
	$(CC) $(INCLUDES) $(ASMFLAGS) -c $< -o $@

## Compile game sources
asmFunctions.o: ../asmFunctions.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

$(GAME).o: ../TowerOfRubble.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

%.uze: $(TARGET)
	-$(UZEBIN_DIR)/packrom $(GAME).hex $@ $(INFO)

UNAME := $(shell sh -c 'uname -s 2>/dev/null || echo not')
AVRSIZEFLAGS := -A ${TARGET}
ifneq (,$(findstring MINGW,$(UNAME)))
AVRSIZEFLAGS := -C --mcu=${MCU} ${TARGET}
endif

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(RESOURCES) $(OBJECTS) $(GAME).* dep/* *.uze

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

