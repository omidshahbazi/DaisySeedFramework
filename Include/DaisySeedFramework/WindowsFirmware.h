#pragma once
#ifndef WINDOWS_FIRMWARE_H
#define WINDOWS_FIRMWARE_H

#include "Common.h"
#include <DigitalSignalProcessing/IFirmware.h>

class IHAL;

class WindowsFirmware : public IFirmware
{
public:
	WindowsFirmware(IHAL* HAL);

	bool Initialize(void) override;

	bool StartUpdate(uint32_t Size) override;
	bool WritePage(const uint8_t* Buffer, uint16_t Length, uint32_t Checksum) override;
	void EndUpdate(void) override;

	float GetProgress(void) const override
	{
		return ((float)m_Offset / m_Size);
	}

private:
	IHAL* m_HAL;
	bool m_IsInitialized;
	uint32_t m_Size;
	uint32_t m_Offset;
};

#endif