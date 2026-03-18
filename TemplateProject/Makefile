# Project Name
TARGET = GuitarMultiEffect

# https://forum.electro-smith.com/t/debugging-app-in-sram-daisy/3213
# https://github.com/bkshepherd/DaisySeedProjects/blob/main/Software/GuitarPedal/Makefile
# https://forum.electro-smith.com/t/out-of-flash-memory-walkthrough-with-samples/4370
ifdef DEBUGGER_PRESENT
ifdef OPTIMIZATION_OFF
OPT = -O0
else
OPT = -Og
endif
else
ifdef DEBUG
OPT = -Os
else
OPT = -O3
endif
endif

CONFIG_HEADER=Config.h
all: $(CONFIG_HEADER)
.PHONY: all $(CONFIG_HEADER)

$(CONFIG_HEADER):
	@echo "//Auto-Generated file by MakeFile" > $(CONFIG_HEADER)
	
	@echo "#define OPTIMIZE_LEVEL_$(shell echo $(subst -O,,$(OPT)) | tr '[:lower:]' '[:upper:]') //$(OPT)" >> $(CONFIG_HEADER)

ifdef DEBUGGER_PRESENT
	@echo "#define DEBUGGER_PRESENT" >> $(CONFIG_HEADER)
endif

ifdef DEBUG
	@echo "#undef DEBUG" >> $(CONFIG_HEADER)
	@echo "#define DEBUG" >> $(CONFIG_HEADER)
endif

ifdef USE_FLASH
	@echo "#define USE_FLASH" >> $(CONFIG_HEADER)
else
ifdef USE_SRAM
	@echo "#define USE_SRAM" >> $(CONFIG_HEADER)
else
	@echo "#define USE_QSPI" >> $(CONFIG_HEADER)
endif
endif

# https://discord.com/channels/1037767234803740694/1040000925122711612/1151155051411288095
ifdef USE_FLASH
APP_TYPE = BOOT_NONE
else
ifdef USE_SRAM
APP_TYPE = BOOT_SRAM
else
APP_TYPE = BOOT_QSPI
endif
endif

# Sources
C_INCLUDES += -isystem include/
CPP_SOURCES += $(shell find src -name '*.cpp')

LDFLAGS += -u _printf_float

# Library Locations
LIBDAISY_DIR ?= include/framework/libDaisy

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core

include $(SYSTEM_FILES_DIR)/Makefile

MAKEFLAGS += -j4