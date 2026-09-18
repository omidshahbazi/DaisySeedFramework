#ifdef ON_HARDWARE

#include "DaisySeedFramework/DaisyFirmware.h"
#include "DaisySeedFramework/DaisySeedHAL.h"
#include <DigitalSignalProcessing/Memory.h>

DaisyFirmware::DaisyFirmware(DaisySeedHAL* HAL)
	: m_HAL(HAL),
	m_IsInitialized(false),
	m_Size(0),
	m_Offset(0)
#ifdef FIRMWARE_SAFE_UPDATE
	, m_EntryPointData{}
#endif
{}

bool DaisyFirmware::Initialize(void)
{
	ASSERT(!m_IsInitialized, "DaisyFirmware is already initialized");

	m_IsInitialized = true;

	return true;
}

bool DaisyFirmware::StartUpdate(uint32_t Size)
{
	ASSERT(m_IsInitialized, "DaisyFirmware is already initialized");

#ifdef FIRMWARE_SAFE_UPDATE
	if (Size < SafeUpdateOffset)
		return false;
#endif

	if (QSPI_PROGRAM_SIZE < Size)
		return false;

	m_Size = Size;
	m_Offset = 0;

	daisy::QSPIHandle& qspi = m_HAL->GetQSPI();

	uint16_t blockCount = (Size + QSPI_MAX_ERASE_SIZE - 1) / QSPI_MAX_ERASE_SIZE;
	for (uint16_t i = 0; i < blockCount; ++i)
	{
		uint32_t address = QSPI_BOOTLOADER_RESERVED_SIZE + (i * QSPI_MAX_ERASE_SIZE);

		if (qspi.Erase(address, address + QSPI_MAX_ERASE_SIZE) != daisy::QSPIHandle::Result::OK)
			return false;
	}

	return true;
}

bool DaisyFirmware::WritePage(const uint8_t* Buffer, uint16_t Length, uint32_t Checksum)
{
	ASSERT(m_IsInitialized, "DaisyFirmware is already initialized");

	if (CRC32(Buffer, Length) != Checksum)
		return false;

	ASSERT(m_Offset + Length <= m_Size, "Out of bounds for this update.");

	daisy::QSPIHandle& qspi = m_HAL->GetQSPI();

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

	if (qspi.Write(address, writeLength, const_cast<uint8_t*>(Buffer)) != daisy::QSPIHandle::Result::OK)
		return false;

	m_Offset += Length;

	return true;
}

void DaisyFirmware::EndUpdate(void)
{
	ASSERT(m_IsInitialized, "DaisyFirmware is already initialized");
	ASSERT(m_Offset == m_Size, "Missing data detected.");

#ifdef FIRMWARE_SAFE_UPDATE
	daisy::QSPIHandle& qspi = m_HAL->GetQSPI();
	ASSERT(qspi.Write(QSPI_BOOTLOADER_RESERVED_SIZE, SafeUpdateOffset, m_EntryPointData) == daisy::QSPIHandle::Result::OK, "Failed to write the first page.");
#endif

	m_HAL->Reset(false);
}

#endif