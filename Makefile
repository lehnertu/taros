#******************************************************************************
# Makefile for TAROS 
# Teensy Autonomous Robot Operating System
#
# Board : Linux
#
# 25.01.2022  Ulf Lehnert
#******************************************************************************

# on fwl78
HOME                = /home/ulf
PROJECT_HOME        = $(HOME)/Programming/taros

# on fwl56
# HOME                = /home/lehnertu
# PROJECT_HOME        = $(HOME)/Programming/taros

TARGET              = DemoSystem
BOARD_ID            = LINUX

CORE_SRC            = $(PROJECT_HOME)/core
CORE_BIN            = $(CORE_SRC)

USR_SRC             = $(PROJECT_HOME)/src
USR_BIN             = $(USR_SRC)

# libraries not yet handled
LIBS_LOCAL_BASE     = $(PROJECT_HOME)/lib
LIBS_LOCAL          = 

#******************************************************************************
# Hardware references
#******************************************************************************

FLAGS_CPU   = 
FLAGS_OPT   = -O2
FLAGS_COM   = -g -Wall
FLAGS_LSP   = 

FLAGS_CPP   = -std=gnu++14 -fexceptions -fpermissive -fno-rtti -fno-threadsafe-statics -felide-constructors -Wno-error=narrowing
FLAGS_C     = 
FLAGS_LD    = -Wl,--gc-sections,--relax

# standard libraries
LIBS        = -lm -lrt -lstdc++

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPP_FLAGS = compiler options for C and C++
CPP_FLAGS   = $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_COM) $(DEFINES) $(FLAGS_CPP)

# compiler options for C only
C_FLAGS     = $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_COM) $(DEFINES) $(FLAGS_C)

# flags for the linker
LD_FLAGS    := $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_LD)

# additional libraries to link
# LIBS +=

# names for the compiler programs
CC = gcc
CXX = g++
SIZE = size

#******************************************************************************
# Source and Include Files
#******************************************************************************
# Recursively create list of source and object files in USR_SRC and CORE_SRC
# and corresponding subdirectories.

rwildcard =$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

#User Sources -----------------------------------------------------------------
USR_HEADERS     = $(call rwildcard,$(USR_SRC)/,*.h)
USR_C_FILES     = $(call rwildcard,$(USR_SRC)/,*.c)
USR_CPP_FILES   = $(call rwildcard,$(USR_SRC)/,*.cpp)
USR_OBJ         = $(USR_C_FILES:$(USR_SRC)/%.c=$(USR_BIN)/%.o) $(USR_CPP_FILES:$(USR_SRC)/%.cpp=$(USR_BIN)/%.o)

#Core Sources -----------------------------------------------------------------
CORE_HEADERS     = $(call rwildcard,$(CORE_SRC)/,*.h)
CORE_C_FILES     = $(call rwildcard,$(CORE_SRC)/,*.c)
CORE_CPP_FILES   = $(call rwildcard,$(CORE_SRC)/,*.cpp)
CORE_OBJ         = $(USR_C_FILES:$(CORE_SRC)/%.c=$(CORE_BIN)/%.o) $(CORE_CPP_FILES:$(CORE_SRC)/%.cpp=$(CORE_BIN)/%.o)

# Includes -------------------------------------------------------------
INCLUDE         = -I$(USR_SRC) -I$(CORE_SRC)

#******************************************************************************
# Rules:
#******************************************************************************

.PHONY: all upload clean distclean

all: $(TARGET)

$(TARGET): $(USR_OBJ) $(CORE_OBJ)

clean:
	rm -f $(USR_BIN)/*.o
	rm -f $(CORE_BIN)/*.o
	rm -f $(TARGET)
	@echo "cleaned from binaries of user code."

# Handle user sources ---------------------------------------------------------
$(USR_BIN)/%.o: $(USR_SRC)/%.c $(USR_HEADERS) $(CORE_HEADERS)
	@echo [compile] $(CC) $(C_FLAGS) $(INCLUDE) -o "$@" -c $<
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o "$@" -c $<

$(USR_BIN)/%.o: $(USR_SRC)/%.cpp $(USR_HEADERS) $(CORE_HEADERS)
	@echo [compile] $(CXX) $(CPP_FLAGS) $(INCLUDE) -o "$@" -c $<
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o "$@" -c $<

# Handle core sources ---------------------------------------------------------
$(CORE_BIN)/%.o: $(CORE_SRC)/%.c $(CORE_HEADERS)
	@echo [compile] $(CC) $(C_FLAGS) $(INCLUDE) -o "$@" -c $<
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o "$@" -c $<

$(CORE_BIN)/%.o: $(CORE_SRC)/%.cpp $(CORE_HEADERS)
	@echo [compile] $(CXX) $(CPP_FLAGS) $(INCLUDE) -o "$@" -c $<
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o "$@" -c $<

# Linking ---------------------------------------------------------------------
$(TARGET): $(LIB_OBJ) $(USR_OBJ) $(CORE_OBJ)
	@echo [linking] $(CC) $(LD_FLAGS) -o "$@" $(USR_OBJ) $(CORE_OBJ) $(LIBS)
	@$(CC) $(LD_FLAGS) -o "$@" $(USR_OBJ) $(CORE_OBJ) $(LIBS)

