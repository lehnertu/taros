#******************************************************************************
# Makefile for Flight Controller
#
# Board : Teensy 4.1
#
# 25.01.2022  Ulf Lehnert
#******************************************************************************

# on fwl78
HOME                = /home/ulf
PROJECT_HOME        = $(HOME)/Programming/TeensyFlightController
ARDUINO_HOME        = $(HOME)/Elektronik/Arduino/arduino-1.8.13

# on fwl56
# HOME                = /home/lehnertu
# PROJECT_HOME        = $(HOME)/Programming/TeensyFlightController
# ARDUINO_HOME        = $(HOME)/Programming/arduino-1.8.19

TARGET              = FlightController
BOARD_ID            = TEENSY41

USR_SRC             = $(PROJECT_HOME)/src
USR_BIN             = $(USR_SRC)

CORE_SRC            = $(PROJECT_HOME)/core
CORE_BIN            = $(CORE_SRC)
CORE_LIB            = $(BOARD_ID)_lib.a

# libraries not yet handled
LIBS_SHARED_BASE    = $(ARDUINO_HOME)/libraries
LIBS_SHARED         = 
LIBS_LOCAL_BASE     = $(PROJECT_HOME)/lib
LIBS_LOCAL          = 

COMPILERPATH        = $(ARDUINO_HOME)/hardware/tools/arm/bin
# path location for Teensy Loader, teensy_post_compile and teensy_reboot (on Linux)
TOOLSPATH           = $(ARDUINO_HOME)/hardware/tools

#******************************************************************************
# Hardware references
#******************************************************************************

# Use these lines for Teensy 4.1
MCU         = IMXRT1062
MCU_LD      = $(CORE_SRC)/imxrt1062_t41.ld
# options needed by many Arduino libraries to configure for Teensy model
MCU_DEF     = -DTEENSYDUINO=147 -DARDUINO=10807 -DARDUINO_TEENSY41 -DF_CPU=600000000
DEFINES     = -D__$(MCU)__ $(MCU_DEF) -DUSB_SERIAL -DLAYOUT_US_ENGLISH -DUSING_MAKEFILE

# for Cortex M7 with single & double precision FPU
FLAGS_CPU   = -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16
FLAGS_OPT   = -O2
FLAGS_COM   = -g -Wall -ffunction-sections -fdata-sections -nostdlib -MMD
FLAGS_LSP   = 

FLAGS_CPP   = -std=gnu++14 -fno-exceptions -fpermissive -fno-rtti -fno-threadsafe-statics -felide-constructors -Wno-error=narrowing
FLAGS_C     = 
FLAGS_S     = -x assembler-with-cpp
FLAGS_LD    = -Wl,--gc-sections,--relax -T$(MCU_LD)

# standard libraries
LIBS        = $(CORE_BIN)/$(CORE_LIB) -larm_cortexM7lfsp_math -lm -lstdc++

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPP_FLAGS = compiler options for C and C++
CPP_FLAGS   = $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_COM) $(DEFINES) $(FLAGS_CPP)
S_FLAGS     = $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_COM) $(DEFINES) $(FLAGS_S)

# compiler options for C only
C_FLAGS     = $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_COM) $(DEFINES) $(FLAGS_C)

# flags for the linker
LD_FLAGS    := $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_LD)

# additional libraries to link
# LIBS +=

# names for the compiler programs
CC = $(COMPILERPATH)/arm-none-eabi-gcc
CXX = $(COMPILERPATH)/arm-none-eabi-g++
OBJCOPY = $(COMPILERPATH)/arm-none-eabi-objcopy
SIZE = $(COMPILERPATH)/arm-none-eabi-size
AR = $(COMPILERPATH)/arm-none-eabi-gcc-ar

#******************************************************************************
# Source and Include Files
#******************************************************************************
# Recursively create list of source and object files in USR_SRC and CORE_SRC
# and corresponding subdirectories.

rwildcard =$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

#User Sources -----------------------------------------------------------------
USR_C_FILES     = $(call rwildcard,$(USR_SRC)/,*.c)
USR_CPP_FILES   = $(call rwildcard,$(USR_SRC)/,*.cpp)
USR_OBJ         = $(USR_C_FILES:$(USR_SRC)/%.c=$(USR_BIN)/%.o) $(USR_CPP_FILES:$(USR_SRC)/%.cpp=$(USR_BIN)/%.o)

# Core library sources --------------------------------------------------------
CORE_CPP_FILES  = $(call rwildcard,$(CORE_SRC)/,*.cpp)
CORE_C_FILES    = $(call rwildcard,$(CORE_SRC)/,*.c)
CORE_S_FILES    = $(call rwildcard,$(CORE_SRC)/,*.S)
CORE_OBJ        = $(CORE_S_FILES:$(CORE_SRC)/%.S=$(CORE_BIN)/%.o) $(CORE_C_FILES:$(CORE_SRC)/%.c=$(CORE_BIN)/%.o) $(CORE_CPP_FILES:$(CORE_SRC)/%.cpp=$(CORE_BIN)/%.o)

# Includes -------------------------------------------------------------
INCLUDE         = -I$(USR_SRC) -I$(CORE_SRC)

#******************************************************************************
# Rules:
#******************************************************************************

.PHONY: all upload clean distclean

all: $(TARGET).hex

$(TARGET).elf: $(CORE_LIB) $(USR_OBJ)

# Core library ----------------------------------------------------------------
$(CORE_BIN)/%.o: $(CORE_SRC)/%.S
	@echo [asm] $(CC) $(S_FLAGS) $(INCLUDE) -o $@ -c $<
	@"$(CC)" $(S_FLAGS) $(INCLUDE) -o $@ -c $<

$(CORE_BIN)/%.o: $(CORE_SRC)/%.c
	@echo [compile] $(CC) $(C_FLAGS) $(INCLUDE) -o $@ -c $<
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o $@ -c $<

$(CORE_BIN)/%.o: $(CORE_SRC)/%.cpp
	@echo [compile] $(CXX) $(CPP_FLAGS) $(INCLUDE) -o $@ -c $<
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o $@ -c $<

$(CORE_LIB): $(CORE_OBJ)
	@echo [assembling core library] $(AR) rcs $(CORE_BIN)/$(CORE_LIB) $(CORE_OBJ)
	@$(AR) rcs $(CORE_BIN)/$(CORE_LIB) $(CORE_OBJ)
	
upload:
	$(TOOLSPATH)/teensy_post_compile -file=$(TARGET) -path=$(shell pwd) -tools=$(TOOLSPATH)
	-$(TOOLSPATH)/teensy_reboot

clean:
	rm -f $(USR_BIN)/*.o $(USR_BIN)/*.d
	rm -f $(CORE_BIN)/*.o $(CORE_BIN)/*.d
	rm -f $(TARGET).elf
	@echo "cleaned from binaries of user code."

distclean:
	rm -f $(USR_BIN)/*.o $(USR_BIN)/*.d
	rm -f $(CORE_BIN)/*.o $(CORE_BIN)/*.d $(CORE_BIN)/$(CORE_LIB)
	rm -f $(TARGET).elf $(TARGET).hex 
	@echo "cleaning done."

# Handle user sources ---------------------------------------------------------
$(USR_BIN)/%.o: $(USR_SRC)/%.c
	@echo [compile] $(CC) $(C_FLAGS) $(INCLUDE) -o "$@" -c $<
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o "$@" -c $<

$(USR_BIN)/%.o: $(USR_SRC)/%.cpp
	@echo [compile] $(CXX) $(CPP_FLAGS) $(INCLUDE) -o "$@" -c $<
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o "$@" -c $<

# Linking ---------------------------------------------------------------------
$(TARGET).elf: $(CORE_LIB) $(LIB_OBJ) $(USR_OBJ)
	@echo [linking] $(CC) $(LD_FLAGS) -o "$@" $(USR_OBJ) $(LIBS)
	@$(CC) $(LD_FLAGS) -o "$@" $(USR_OBJ) $(LIBS)

%.hex: %.elf
	@echo [HEX] $(SIZE) "$<"
	@$(SIZE) "$<"
	@echo [HEX] $(OBJCOPY) -O ihex -R.eeprom "$<" "$@"
	@$(OBJCOPY) -O ihex -R.eeprom "$<" "$@"

