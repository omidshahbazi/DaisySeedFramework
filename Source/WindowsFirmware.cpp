#ifdef ON_WINDOWS

#include "DaisySeedFramework/WindowsFirmware.h"
#include <DigitalSignalProcessing/IHAL.h>
#include <DigitalSignalProcessing/Memory.h>

WindowsFirmware::WindowsFirmware(IHAL* HAL)
	: m_HAL(HAL),
	m_IsInitialized(false),
	m_Size(0),
	m_Offset(0)
{}

bool WindowsFirmware::Initialize(void)
{
	ASSERT(!m_IsInitialized, "DaisyFirmware is already initialized");

	m_IsInitialized = true;

	return true;
}

bool WindowsFirmware::StartUpdate(uint32_t Size)
{
	ASSERT(m_IsInitialized, "DaisyFirmware is already initialized");

	if (Size < QSPI_PAGE_SIZE || QSPI_PROGRAM_SIZE < Size)
		return false;

	m_Size = Size;
	m_Offset = 0;

	return true;
}

bool WindowsFirmware::WritePage(const uint8_t* Buffer, uint16_t Length, uint32_t Checksum)
{
	ASSERT(m_IsInitialized, "DaisyFirmware is already initialized");

	if (CRC32(Buffer, Length) != Checksum)
		return false;

	ASSERT(m_Offset + Length <= m_Size, "Out of bounds for this update.");

	m_Offset += Length;

	return true;
}

void WindowsFirmware::EndUpdate(void)
{
	ASSERT(m_IsInitialized, "DaisyFirmware is already initialized");
	ASSERT(m_Offset == m_Size, "Missing data detected.");

	m_HAL->Reset(false);
}

#endif