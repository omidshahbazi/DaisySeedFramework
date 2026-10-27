#include "DaisySeedFramework/Firmware.h"
#include <DigitalSignalProcessing/IHAL.h>
#include <DigitalSignalProcessing/Memory.h>

Firmware::Firmware(IHAL* HAL)
	: m_HAL(HAL),
	m_IsInitialized(false),
	m_Size(0),
	m_Offset(0)
#ifdef FIRMWARE_SAFE_UPDATE
	, m_EntryPointData{}
#endif
{}

bool Firmware::Initialize(void)
{
	ASSERT(!m_IsInitialized, "Firmware is already initialized");

	m_IsInitialized = true;

	return true;
}

bool Firmware::StartUpdate(uint32_t Size)
{
	ASSERT(m_IsInitialized, "Firmware is not initialized");

#ifdef FIRMWARE_SAFE_UPDATE
	if (Size < SafeUpdateOffset)
		return false;
#endif

	if (QSPI_PROGRAM_SIZE < Size)
		return false;

	m_Size = Size;
	m_Offset = 0;

	uint16_t blockCount = (Size + QSPI_MAX_ERASE_SIZE - 1) / QSPI_MAX_ERASE_SIZE;
	for (uint16_t i = 0; i < blockCount; ++i)
	{
		uint32_t address = QSPI_BOOTLOADER_RESERVED_SIZE + (i * QSPI_MAX_ERASE_SIZE);

		if (m_HAL->ErasePersistentData(address, QSPI_MAX_ERASE_SIZE))
			return false;
	}

	return true;
}

bool Firmware::WritePage(const uint8_t* Buffer, uint16_t Length, uint32_t Checksum)
{
	ASSERT(m_IsInitialized, "Firmware is not initialized");

	if (CRC32(Buffer, Length) != Checksum)
		return false;

	ASSERT(m_Offset + Length <= m_Size, "Out of bounds for this update.");

	uint32_t address = QSPI_BOOTLOADER_RESERVED_SIZE + m_Offset;
	uint16_t writeLength = Length;

#ifdef FIRMWARE_SAFE_UPDATE
	if (m_Offset < SafeUpdateOffset)
	{
		Memory::Copy(Buffer, m_EntryPointData, SafeUpdateOffset);

		address += SafeUpdateOffset;
		writeLength -= SafeUpdateOffset;
		Buffer += SafeUpdateOffset;
	}
#endif

	ASSERT(writeLength >= QSPI_PAGE_SIZE, "writeLength cannot be less than page-size");

	if (m_HAL->WritePersistentData(address, Buffer, writeLength))
		return false;

	m_Offset += Length;

	return true;
}

void Firmware::EndUpdate(void)
{
	ASSERT(m_IsInitialized, "Firmware is not initialized");
	ASSERT(m_Offset == m_Size, "Missing data detected.");

#ifdef FIRMWARE_SAFE_UPDATE
	ASSERT(m_HAL->WritePersistentData(QSPI_BOOTLOADER_RESERVED_SIZE, m_EntryPointData, SafeUpdateOffset), "Failed to write the first page.");
#endif

	m_HAL->Reset(false);
}