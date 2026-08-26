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
	Pin0 = (uint8)GPIOPins::Pin15,
	Pin1 = (uint8)GPIOPins::Pin16,
	Pin2 = (uint8)GPIOPins::Pin17,
	Pin3 = (uint8)GPIOPins::Pin18,
	Pin4 = (uint8)GPIOPins::Pin19,
	Pin5 = (uint8)GPIOPins::Pin20,
	Pin6 = (uint8)GPIOPins::Pin21,
	Pin7 = (uint8)GPIOPins::Pin22,
	Pin8 = (uint8)GPIOPins::Pin23,
	Pin9 = (uint8)GPIOPins::Pin24,
	Pin10 = (uint8)GPIOPins::Pin25,
	Pin11 = (uint8)GPIOPins::Pin28,
	COUNT = 12
};

#define SDRAM_TOTAL_SIZE 64 MB

#if defined(ON_WINDOWS)

#define DEFINE_LARGE_MEMORY_BUFFER(Name, Size)  \
	static constexpr uint32 Name##_Size = Size; \
	uint8 g_##Name[Name##_Size] = {0};

#elif defined(ON_HARDWARE)
#include <libDaisy/src/dev/sdram.h>


#define DEFINE_LARGE_MEMORY_BUFFER(Name, Size)  \
	static constexpr uint32 Name##_Size = Size; \
	uint8 DSY_SDRAM_BSS g_##Name[Name##_Size];
#endif

//This is reserved for Bootloader
#define QSPI_RESERVED_SIZE 256 KB

// Even in SRAM mode, we it uses QSPI to store program and loads it into the SRAM in runtime to run faster
// But in QSPI mode, it runs the code directly from QSPI which is slower
// https://daisy.audio/tutorials/_a7_Getting-Started-Daisy-Bootloader/#custom-linkers
// QSPI Start Address=0x90000000
// 64KB Reserved (Still can be used, better not to)
// Program Start Address = 0x90040000
// So we would still have at least 1MB of space for code
#define DEFAULT_PROGRAM_SIZE 1 MB - QSPI_RESERVED_SIZE
#ifndef QSPI_PROGRAM_SIZE
#define QSPI_PROGRAM_SIZE DEFAULT_PROGRAM_SIZE
#endif

#define QSPI_TOTAL_SIZE 8 MB
#define QSPI_PAGE_SIZE 4 KB

#ifndef QSPI_START_ADDRESS
#define QSPI_START_ADDRESS QSPI_RESERVED_SIZE + QSPI_PROGRAM_SIZE
#endif
static_assert(QSPI_START_ADDRESS >= QSPI_RESERVED_SIZE, "Invalid QSPI_START_ADDRESS defined");

#define QSPI_AVAILABLE_SIZE QSPI_TOTAL_SIZE - QSPI_START_ADDRESS

#define QSPI_END_ADDRESS QSPI_TOTAL_SIZE

#define DEFINE_LINKER_STORAGE_SECTION(Name) __attribute__((section(Name)))