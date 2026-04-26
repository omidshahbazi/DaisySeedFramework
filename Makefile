ifndef TARGET
TARGET = UnknownName
endif

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

MODULE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

# Library Locations
LIBDAISY_DIR ?= $(MODULE_DIR)/libDaisy

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core

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

include $(SYSTEM_FILES_DIR)/Makefile

ifndef SINGLE_THREADED_COMPILE
MAKEFLAGS += -j4
endif