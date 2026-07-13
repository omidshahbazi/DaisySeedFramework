include Makefile.Common

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
# Linker File
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
SKIP_INTERNAL_LINKER_SCRIPT ?= 0

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
# Source Files
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
INCLUDE_FILES += -isystem ../../include/
CPP_SOURCE_FILES += $(shell find ../../src -name '*.cpp')

# Add USBD
C_SOURCE_FILES += Libraries/USBD/source/usbd_audio.c
C_SOURCE_FILES += Libraries/USBD/source/usbd_audio_if.c
C_SOURCE_FILES += Libraries/USBD/source/usbd_conf.c
C_SOURCE_FILES += Libraries/USBD/source/usbd_desc.c
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BUILD_DIRECTORY = ../../build

C_DEFS += $(DEFINES)

include $(SYSTEM_FILES_DIR)/Makefile