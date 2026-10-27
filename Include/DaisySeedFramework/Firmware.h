#pragma once
#ifndef FIRMWARE_H
#define FIRMWARE_H

#define FIRMWARE_SAFE_UPDATE

#include "Common.h"
#include <DigitalSignalProcessing/IFirmware.h>

class IHAL;

class Firmware : public IFirmware
{
#ifdef FIRMWARE_SAFE_UPDATE
private:
	static constexpr uint16_t SafeUpdateOffset = QSPI_PAGE_SIZE;
#endif

public:
	Firmware(IHAL* HAL);

	bool Initialize(void) override;

	bool StartUpdate(uint32_t Size) override;
	bool WritePage(const uint8_t* Buffer, uint16_t Length, uint32_t Checksum) override;
	void EndUpdate(void) override;

private:
	IHAL* m_HAL;
	bool m_IsInitialized;
	uint32_t m_Size;
	uint32_t m_Offset;

#ifdef FIRMWARE_SAFE_UPDATE
	uint8_t m_EntryPointData[SafeUpdateOffset];
#endif
};

#endif