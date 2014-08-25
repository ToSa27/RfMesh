PROJECT = RfMeshBootloader

LIBSOURCES = 
LIBHEADERS = rfm70reg RfMeshProtocol RfMeshBootloaderRfm70 RfMeshNodeType RfMeshNodeConfig

NODE_ADDRESS = 0
DB_HWTYPE = 2
DB_HWVERSION = 1

FINALOUT = ../master/bootloader/$(DB_BLVERSION)_$(PROJECT)

MCU = atmega328p
CLK = 16000000L

ISP_PORT = com5
ISP_SPEED = 115200
ISP_PROTOCOL = stk500v2
ISP_MCU = m328p
# HFUSE = D8 - 4096 byte boot
ISP_HFUSE = D8
# LFUSE = 4E - Low power resonator (16MHz), fast start, clkdiv/8
ISP_LFUSE = 4E
# EFUSE = 06 - 1.8V brownout (hence the clkdiv/8 requirement)
ISP_EFUSE = 06
ISP_ARGS = -c$(ISP_PROTOCOL) -P$(ISP_PORT) -b$(ISP_SPEED) -p$(ISP_MCU)

ifeq ($(OS),Windows_NT)
	BINPATH = C:/Program Files (x86)/Arduino/hardware/tools/avr/bin/
	JAVAPATH = C:/Program Files/Java/jdk1.7.0_40/bin/
	JAVAC = $(JAVAPATH)javac.exe
	JAVA = $(JAVAPATH)java.exe
else
	UNAME_S := $(shell uname -s)
	UNAME_P := $(shell uname -p)
	UNAME_M := $(shell uname -m)
	ifeq ($(UNAME_S),Linux)
		ifeq ($(UNAME_P),x86_64)
			BINPATH = ../arduino/bin-linux64/bin/
		else ifneq ($(filter %86,$(UNAME_P)),)
			BINPATH = ../arduino/bin-linux32/bin/
		else ifneq ($(filter arm%,$(UNAME_M)),)
			BINPATH = /usr/bin/
		endif
	endif
endif

PWD = $(shell pwd)
SOURCEPATH = source
BUILDPATH = build
LIBPATH = ../lib
OUTPATH = out

CFLAGS = -c -g -Os -fno-inline-small-functions -fno-split-wide-types -mshort-calls -Wall -mmcu=$(MCU) -DF_CPU=$(CLK) -std=gnu99
LDFLAGS = -Os -Wl,--section-start=.text=0x7000 -nostartfiles -mmcu=$(MCU) 

all: clean source out

clean:
	rm -rf $(BUILDPATH)
	rm -rf $(SOURCEPATH)
	rm -rf $(OUTPATH)

source: $(SOURCEPATH)/$(PROJECT).c $(addprefix $(SOURCEPATH)/, $(addsuffix .h, $(LIBSOURCES))) $(addprefix $(SOURCEPATH)/, $(addsuffix .cpp, $(LIBSOURCES))) $(addprefix $(SOURCEPATH)/, $(addsuffix .h, $(LIBHEADERS)))

$(SOURCEPATH)/%.java: %.java
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(SOURCEPATH)/%.h: %.h
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(SOURCEPATH)/%.c: %.c
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(SOURCEPATH)/%.cpp: %.cpp
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(SOURCEPATH)/%.h: $(LIBPATH)/%.h
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(SOURCEPATH)/%.cpp: $(LIBPATH)/%.cpp
	mkdir -p $(SOURCEPATH)
	cp $< $@

$(BUILDPATH)/$(PROJECT).o: $(SOURCEPATH)/$(PROJECT).c
	mkdir -p $(BUILDPATH)
	"$(BINPATH)avr-gcc" $(CFLAGS) -I$(SOURCEPATH) $< -o $@

$(BUILDPATH)/%.class: $(SOURCEPATH)/%.java
	mkdir -p $(BUILDPATH)
	"$(JAVAC)" $< -d $(BUILDPATH)
	
$(OUTPATH)/$(PROJECT).elf: $(BUILDPATH)/$(PROJECT).o
	mkdir -p $(OUTPATH)
	"$(BINPATH)avr-gcc" $(LDFLAGS) -o $@ $< -lm
	
$(OUTPATH)/$(PROJECT).eep: $(BUILDPATH)/eep.class
	mkdir -p $(OUTPATH)
	"$(JAVA)" -cp $(BUILDPATH) eep $(DB_HWTYPE) $(DB_HWVERSION) $(NODE_ADDRESS) > $@

$(OUTPATH)/$(PROJECT).hex: $(OUTPATH)/$(PROJECT).elf
	"$(BINPATH)avr-objcopy" -O ihex -R .eeprom $< $@ 

out: $(OUTPATH)/$(PROJECT).elf $(OUTPATH)/$(PROJECT).eep $(OUTPATH)/$(PROJECT).hex
	"$(BINPATH)avr-size" $(OUTPATH)/$(PROJECT).elf
	cp -f $(OUTPATH)/$(PROJECT).eep $(FINALOUT).eep
	cp -f $(OUTPATH)/$(PROJECT).hex $(FINALOUT).hex

load: clean source out isp

isp: $(FINALOUT).hex $(FINALOUT).eep
	"$(BINPATH)avrdude" $(ISP_ARGS) -e -u -Ulock:w:0x3f:m -qq -Uefuse:w:0x$(ISP_EFUSE):m -Uhfuse:w:0x$(ISP_HFUSE):m -Ulfuse:w:0x$(ISP_LFUSE):m -Ulock:w:0x0f:m
	"$(BINPATH)avrdude" $(ISP_ARGS) -V -q -s -Uflash:w:$(FINALOUT).hex -Ueeprom:w:$(FINALOUT).eep
	"$(BINPATH)avrdude" $(ISP_ARGS)
