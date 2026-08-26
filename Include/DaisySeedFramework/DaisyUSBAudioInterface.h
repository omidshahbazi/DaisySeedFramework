#pragma once
#ifndef DAISY_USB_AUDIO_INTERFACE_H
#define DAISY_USB_AUDIO_INTERFACE_H

#include <DigitalSignalProcessing/IUSBAudioInterface.h>

class DaisyUSBAudioInterface : public IUSBAudioInterface
{
public:
	void Start(USBInterfaces Interface) override;

	void Stop(void) override;

	void Push(const uint8* BufferL, const uint8* BufferR, uint8 Count) override;

	//private:
		//USBD_HandleTypeDef m_Device;
};

#endif