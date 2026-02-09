# Project Name
TARGET = GuitarMultiEffect

# https://forum.electro-smith.com/t/debugging-app-in-sram-daisy/3213
# https://github.com/bkshepherd/DaisySeedProjects/blob/main/Software/GuitarPedal/Makefile
# https://forum.electro-smith.com/t/out-of-flash-memory-walkthrough-with-samples/4370
ifeq ($(DEBUG), 1)
OPT = -Os
else
OPT = -O3
endif

# https://discord.com/channels/1037767234803740694/1040000925122711612/1151155051411288095
ifeq ($(USE_SRAM), 1)
APP_TYPE = BOOT_SRAM
C_DEFS += -DUSE_SRAM=1
else
APP_TYPE = BOOT_QSPI
C_DEFS += -DUSE_SRAM=0
endif

# Sources
CPP_SOURCES = src/main.cpp

LDFLAGS += -u _printf_float

# Library Locations
LIBDAISY_DIR ?= ../../DaisySeedFramework/libDaisy

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core

include $(SYSTEM_FILES_DIR)/Makefile