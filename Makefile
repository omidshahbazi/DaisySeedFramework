#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
# Target Name
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
ifndef TARGET
TARGET = UnknownName
endif
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
# Paths
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
MODULE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
LIBDAISY_DIR ?= $(MODULE_DIR)libDaisy
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core

#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
# Optimization Level
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
ifndef OPT
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
endif
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
# Config File
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CONFIG_HEADER_FILER = Config.h
all: $(CONFIG_HEADER_FILER)
.PHONY: all $(CONFIG_HEADER_FILER)

$(CONFIG_HEADER_FILER):
	@echo "//Auto-Generated file by MakeFile" > $(CONFIG_HEADER_FILER)
	
	@echo "#define ON_DAISY_SEED" >> $(CONFIG_HEADER_FILER)
	@echo "#define OPTIMIZE_LEVEL_$(shell echo $(subst -O,,$(OPT)) | tr '[:lower:]' '[:upper:]') //$(OPT)" >> $(CONFIG_HEADER_FILER)

ifdef DEBUGGER_PRESENT
	@echo "#define DEBUGGER_PRESENT" >> $(CONFIG_HEADER_FILER)
endif

ifdef DEBUG
	@echo "#undef DEBUG" >> $(CONFIG_HEADER_FILER)
	@echo "#define DEBUG" >> $(CONFIG_HEADER_FILER)
endif

ifdef USE_FLASH
	@echo "#define USE_FLASH" >> $(CONFIG_HEADER_FILER)
else
ifdef USE_SRAM
	@echo "#define USE_SRAM" >> $(CONFIG_HEADER_FILER)
else
	@echo "#define USE_QSPI" >> $(CONFIG_HEADER_FILER)
endif
endif
#////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
# Target Storage Type
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
ifdef USE_FLASH
APP_TYPE = BOOT_NONE
else
ifdef USE_SRAM
APP_TYPE = BOOT_SRAM
else
APP_TYPE = BOOT_QSPI
endif
endif
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
# Linker File
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
ifndef SKIP_INTERNAL_LINKER_SCRIPT
SKIP_INTERNAL_LINKER_SCRIPT = 0
endif

LINKER_SCRIPT_FILE = $(abspath Linker.lds)
all: $(LINKER_SCRIPT_FILE)
.PHONY: all $(LINKER_SCRIPT_FILE)

$(LINKER_SCRIPT_FILE):
	@echo "/*Auto-Generated file by MakeFile*/" > $(LINKER_SCRIPT_FILE)
	
ifeq ($(SKIP_INTERNAL_LINKER_SCRIPT), 0)
ifdef USE_FLASH
	@echo "INCLUDE \"$(SYSTEM_FILES_DIR)/STM32H750IB_flash.lds\"" >> $(LINKER_SCRIPT_FILE)
else
ifdef USE_SRAM
	@echo "INCLUDE \"$(SYSTEM_FILES_DIR)/STM32H750IB_sram.lds\"" >> $(LINKER_SCRIPT_FILE)
else
	@echo "INCLUDE \"$(SYSTEM_FILES_DIR)/STM32H750IB_qspi.lds\"" >> $(LINKER_SCRIPT_FILE)
endif
endif
endif

ifdef CUSTOM_LINKER_SCRIPT_FILE
	@echo "INCLUDE \"$(CUSTOM_LINKER_SCRIPT_FILE)\"" >> $(LINKER_SCRIPT_FILE)
endif

LDSCRIPT = $(LINKER_SCRIPT_FILE)
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
# Bootloader
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
ifdef INTERNAL_BOOTLOADER
ifdef FAST_BOOTLOADER
BOOT_BIN = $(SYSTEM_FILES_DIR)/dsy_bootloader_v6_4-intdfu-10ms.bin
else
BOOT_BIN = $(SYSTEM_FILES_DIR)/dsy_bootloader_v6_4-intdfu-2000ms.bin
endif
else
ifdef FAST_BOOTLOADER
BOOT_BIN = $(SYSTEM_FILES_DIR)/dsy_bootloader_v6_4-extdfu-10ms.bin
else
BOOT_BIN = $(SYSTEM_FILES_DIR)/dsy_bootloader_v6_4-extdfu-2000ms.bin
endif
endif
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
# Source Files
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
C_INCLUDES += -isystem include/
CPP_SOURCES += $(shell find src -name '*.cpp')

LDFLAGS += -u _printf_float

# Add ARM-CMSIS
C_DEFS += -DARM_MATH_LOOPUNROLL

C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/CommonTables/arm_const_structs.c
C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/CommonTables/arm_common_tables.c

C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/StatisticsFunctions/arm_max_f32.c

C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mag_f32.c

C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/FilteringFunctions/arm_biquad_cascade_df1_init_f32.c
C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/FilteringFunctions/arm_biquad_cascade_df1_f32.c

C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/FilteringFunctions/arm_fir_init_f32.c
C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/FilteringFunctions/arm_fir_f32.c

C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/TransformFunctions/arm_cfft_init_f32.c
C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/TransformFunctions/arm_cfft_f32.c
C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix8_f32.c
C_SOURCES += ${LIBDAISY_DIR}/Drivers/CMSIS-DSP/Source/TransformFunctions/arm_bitreversal2.c
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////

include $(SYSTEM_FILES_DIR)/Makefile

ifeq ($(SINGLE_THREADED_COMPILE), 0)
MAKEFLAGS += -j4
endif