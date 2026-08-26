#pragma once
#ifndef DAISY_USB_INTERFACE_H
#define DAISY_USB_INTERFACE_H

#include "StaticVector.h"
#include <DigitalSignalProcessing/IUSBInterface.h>
#include "DaisyInclude.h"

class DaisyUSBInterface : public IUSBInterface
{
	friend class DaisySeedHAL;

private:
	typedef StaticVector<uint8, 1024> BufferType;

private:
	DaisyUSBInterface(daisy::DaisySeed* Hardware);

	void Start(USBInterfaces Interface) override;

	void Stop(void) override;

	void Update(void);

public:
	void Transmit(const uint8* Buffer, uint16 Length) const override
	{
		TransmitFragmented(Buffer, Length);
	}

	void SetCallback(EventHandler Callback) override
	{
		m_Callback = Callback;
	}

private:
	void HandleIncomings(void);

	void TransmitFragmented(const uint8* Buffer, uint16 Length, uint16 Delay = 100) const;

	static void Callback(uint8* Buffer, uint32_t* Length);

private:
	daisy::DaisySeed* m_Hardware;
	bool m_IsStarted;
	EventHandler m_Callback;
	BufferType* m_Buffer;
};

#endif