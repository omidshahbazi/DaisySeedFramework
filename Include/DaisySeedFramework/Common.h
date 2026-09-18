#pragma once

#include <DigitalSignalProcessing/Common.h>

#if !defined(ON_WINDOWS) && !defined(ON_HARDWARE)
#error "Neither of ON_WINDOWS or ON_HARDWARE are defined"
#endif

enum class GPIOPins
{
	Pin0 = 0, // Digital
	Pin1,	  // Digital
	Pin2,	  // Digital
	Pin3,	  // Digital
	Pin4,	  // Digital
	Pin5,	  // Digital
	Pin6,	  // Digital
	Pin7,	  // Digital
	Pin8,	  // Digital
	Pin9,	  // Digital
	Pin10,	  // Digital
	Pin11,	  // Digital
	Pin12,	  // Digital
	Pin13,	  // Digital
	Pin14,	  // Digital
	Pin15,	  // Analog/Digital
	Pin16,	  // Analog/Digital
	Pin17,	  // Analog/Digital
	Pin18,	  // Analog/Digital
	Pin19,	  // Analog/Digital
	Pin20,	  // Analog/Digital
	Pin21,	  // Analog/Digital
	Pin22,	  // Analog/Digital
	Pin23,	  // Analog/Digital
	Pin24,	  // Analog/Digital
	Pin25,	  // Analog/Digital
	Pin26,	  // Digital
	Pin27,	  // Digital
	Pin28,	  // Analog/Digital
	Pin29,	  // Digital
	Pin30,	  // Digital
	COUNT
};

enum class AnalogPins
{
	Pin0 = (uint8_t)GPIOPins::Pin15,
	Pin1 = (uint8_t)GPIOPins::Pin16,
	Pin2 = (uint8_t)GPIOPins::Pin17,
	Pin3 = (uint8_t)GPIOPins::Pin18,
	Pin4 = (uint8_t)GPIOPins::Pin19,
	Pin5 = (uint8_t)GPIOPins::Pin20,
	Pin6 = (uint8_t)GPIOPins::Pin21,
	Pin7 = (uint8_t)GPIOPins::Pin22,
	Pin8 = (uint8_t)GPIOPins::Pin23,
	Pin9 = (uint8_t)GPIOPins::Pin24,
	Pin10 = (uint8_t)GPIOPins::Pin25,
	Pin11 = (uint8_t)GPIOPins::Pin28,
	COUNT = 12
};

#define STACK_POINTER_SIZE PointerSize
#define RESET_HANDLER_POINTER_SIZE PointerSize
#define ENTRY_POINT_SIZE STACK_POINTER_SIZE + RESET_HANDLER_POINTER_SIZE

#define SRAM_TOTAL_SIZE (480 KB)
#define SDRAM_TOTAL_SIZE (64 MB)

#if defined(ON_WINDOWS)

#define DEFINE_LARGE_MEMORY_BUFFER(Name, Size)  \
	static constexpr uint32_t Name##_Size = Size; \
	uint8_t g_##Name[Name##_Size] = {0};

#elif defined(ON_HARDWARE)
#include <libDaisy/src/dev/sdram.h>


#define DEFINE_LARGE_MEMORY_BUFFER(Name, Size)  \
	static constexpr uint32_t Name##_Size = Size; \
	uint8_t DSY_SDRAM_BSS g_##Name[Name##_Size];
#endif

// Memory layout constants for the external QSPI flash on Daisy Seed
// (IS25LP064A, 8MB), used when building with APP_TYPE=BOOT_SRAM.
//
// Layout:
//   0x90000000 +---------------------------+  <- QSPI chip base
//              | Bootloader-reserved        |  QSPI_BOOTLOADER_RESERVED_SIZE
//   0x90040000 +---------------------------+  <- QSPI_PROGRAM_START_ADDRESS
//              | Application image          |  QSPI_PROGRAM_SIZE
//              +---------------------------+  <- QSPI_USER_DATA_START_ADDRESS
//              | Free for your own use      |  QSPI_USER_DATA_SIZE
//              | (presets, samples, etc.)   |
//   0x90800000 +---------------------------+  <- QSPI end (QSPI_TOTAL_SIZE)
//
// Note: the bootloader *binary* itself (~128KB) lives on internal MCU
// flash (0x08000000, 128KB), not on QSPI at all. The 256KB reserved here
// is just the offset the official electro-smith bootloader is hardcoded
// to start reading the application image from
// (daisy::System::kQspiBootloaderOffset) -- there's no separate "64KB"
// zone inside it, so don't write anywhere in this range.

// This is reserved for the bootloader. Matches
// daisy::System::kQspiBootloaderOffset (0x40000) -- do not write here.
#define QSPI_BOOTLOADER_RESERVED_SIZE (256 KB)

// Even in SRAM mode, the app image is stored in QSPI and copied into SRAM
// at boot time to run faster (in QSPI/BOOT_QSPI mode, code instead runs
// directly from QSPI, which is slower).
// https://docs.daisy.audio/tutorials/_a7_Getting-Started-Daisy-Bootloader/
//   QSPI base address        = 0x90000000
//   Bootloader-reserved size = 256KB
//   Program start address    = 0x90040000
//
// Default program size gives 768KB (1MB total budget minus the 256KB
// reserved for the bootloader) for the application image.
#define QSPI_DEFAULT_PROGRAM_SIZE ((1 MB) - QSPI_BOOTLOADER_RESERVED_SIZE)
#ifndef QSPI_PROGRAM_SIZE
#define QSPI_PROGRAM_SIZE QSPI_DEFAULT_PROGRAM_SIZE
#endif

#define QSPI_TOTAL_SIZE (8 MB)

#define QSPI_PAGE_SIZE (256)

// Minimum erase granularity on the IS25LP064A (erases happen in 4K, 32K,
// or 64K increments -- 4K is the smallest). This is NOT the write page
// size: QSPIHandle::WritePage() writes in 256-byte pages internally,
// regardless of this constant.
#define QSPI_MIN_ERASE_SIZE (4 KB)
#define QSPI_MAX_ERASE_SIZE (64 KB)

// Address (relative to QSPI base, 0x90000000) where free space for your
// own data starts -- right after the reserved bootloader region and the
// application image.
#ifndef QSPI_USER_DATA_START_ADDRESS
#define QSPI_USER_DATA_START_ADDRESS (QSPI_BOOTLOADER_RESERVED_SIZE + QSPI_PROGRAM_SIZE)
#endif
static_assert(QSPI_USER_DATA_START_ADDRESS >= QSPI_BOOTLOADER_RESERVED_SIZE, "Invalid QSPI_USER_DATA_START_ADDRESS defined");

#define QSPI_USER_DATA_SIZE (QSPI_TOTAL_SIZE - QSPI_USER_DATA_START_ADDRESS)

#define QSPI_END_ADDRESS QSPI_TOTAL_SIZE

#define DEFINE_LINKER_STORAGE_SECTION(Name) __attribute__((section(Name)))

#define BYTES_TO_WORDS(SizeInBytes) (((SizeInBytes) + 1) / sizeof(uint32_t))
#define BYTES_TO_DWORDS(SizeInBytes) (((SizeInBytes) + 3) / sizeof(uint32_t))