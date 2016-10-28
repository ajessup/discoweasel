# Tiva Makefile
# #####################################
#
# Part of the uCtools project
# uctools.github.com
#
#######################################
# user configuration:
#######################################
# TARGET: name of the output file
TARGET = main
# MCU: part number to build for
MCU = TM4C123GH6PM
# SOURCES: list of input source sources
SOURCES = heap.c lpd8806_ledstrip.c fft.c nokia.c gpio.c main.c startup_gcc.c
# INCLUDES: list of includes, by default, use Includes directory
INCLUDES = -IInclude
# OUTDIR: directory to use for output
OUTDIR = build
# TIVAWARE_PATH: path to tivaware folder
TIVAWARE_PATH = /Users/jessup/sites/tivaware
# ARM_CS_TOOLS_PATH: path to the arm-cs-tools installation
ARM_CS_TOOLS_PATH = /Users/jessup/sites/gcc-arm-none-eabi-5_4-2016q3
# LM4_TOOLS_PATH: path to the lm4tools installation
LM4_TOOLS_PATH = /Users/jessup/sites/lm4tools
# OPENOCD_PATH:
OPENOCD_PATH=/usr/local/Cellar/open-ocd/0.9.0

# LD_SCRIPT: linker script
LD_SCRIPT = $(MCU).ld

# define flags
CFLAGS = -g -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
CFLAGS += -Os -ffunction-sections -fdata-sections -MD -std=c99 -Wall -O
CFLAGS += -pedantic -DPART_$(MCU) -c -I$(TIVAWARE_PATH)
CFLAGS += -DTARGET_IS_BLIZZARD_RA1
# LDFLAGS = -T $(LD_SCRIPT) --entry ResetISR --gc-sections
LDFLAGS = -L$(TIVAWARE_PATH)/driverlib/gcc
LDFLAGS += -L$(ARM_CS_TOOLS_PATH)/arm-none-eabi/lib/armv7-m
LDFLAGS += -L$(ARM_CS_TOOLS_PATH)/lib/gcc/arm-none-eabi/5.4.1/armv7-m
LDFLAGS += --script=$(LD_SCRIPT)
LDFLAGS += --entry=ResetISR
LDFLAGS += -rpath=$(ARM_CS_TOOLS_PATH)/arm-none-eabi/lib/armv7-m
LDFLAGS += -lm -lgcc -lgcov -lc -ldriver --gc-sections

#######################################
# end of user configuration
#######################################
#
#######################################
# binaries
#######################################
CC = $(ARM_CS_TOOLS_PATH)/bin/arm-none-eabi-gcc
LD = $(ARM_CS_TOOLS_PATH)/bin/arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
RM      = rm -f
MKDIR	= mkdir -p
#######################################

# list of object files, placed in the build directory regardless of source path
OBJECTS = $(addprefix $(OUTDIR)/,$(notdir $(SOURCES:.c=.o)))

# Setup OpenOCD paths
OPENOCD=$(OPENOCD_PATH)/bin/openocd
BRDPATH=$(OPENOCD_PATH)/share/openocd/scripts/board/ek-tm4c123gxl.cfg

# default: build bin
all: $(OUTDIR)/$(TARGET).bin

$(OUTDIR)/%.o: src/%.c | $(OUTDIR)
	$(CC) -o $@ $^ $(CFLAGS)

$(OUTDIR)/a.out: $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(OUTDIR)/main.bin: $(OUTDIR)/a.out
	$(OBJCOPY) -O binary $< $@

# create the output directory
$(OUTDIR):
	$(MKDIR) $(OUTDIR)

clean:
	-$(RM) $(OUTDIR)/*

up: all
	$(LM4_TOOLS_PATH)/lm4flash/lm4flash $(OUTDIR)/$(TARGET).bin

debug:
	$(ARM_CS_TOOLS_PATH)/bin/arm-none-eabi-gdb -ex 'target extended-remote | $(OPENOCD) -f $(BRDPATH) -c "gdb_port pipe; log_output openocd.log"; monitor reset halt; load;' build/a.out

.PHONY: all clean up debug
