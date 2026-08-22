#pragma once
#ifndef DAISY_USB_AUDIO_INTERFACE_H
#define DAISY_USB_AUDIO_INTERFACE_H

#include <DigitalSignalProcessing/IUSBAudioInterface.h>
#include "Libraries/USBD/include/usbd_audio_if.h"

extern "C"
{
#include "usbd_core.h"
#include "Libraries/USBD/include/usbd_desc.h"
#include "Libraries/USBD/include/usbd_audio.h"
}

template <bool External, typename T, uint8 FrameLength>
class DaisyUSBAudioInterface : public IUSBAudioInterface<T>
{
public:
	void Start(void) override
	{
		if constexpr (External)
			USBD_Init(&m_Device, &HS_Desc, DEVICE_HS);
		else
			USBD_Init(&m_Device, &FS_Desc, DEVICE_FS);

		USBD_RegisterClass(&m_Device, &USBD_AUDIO);

		USBD_AUDIO_RegisterInterface(&m_Device, &USBD_AUDIO_fops);

		USBD_Start(&m_Device);
	}

	void Stop(void) override
	{
		USBD_Stop(&m_Device);

		USBD_DeInit(&m_Device);

		// HAL_NVIC_DisableIRQ(OTG_FS_IRQn);

		// HAL_PCD_DeInit((PCD_HandleTypeDef *)m_Device->pData);
	}

	void Transmit(const T *BufferL, const T *BufferR, uint8 Count) override
	{
		static int16 buffer[FrameLength * 2];

		for (uint8 i = 0; i < Count; ++i)
		{
			buffer[i * 2 + 0] = (int16)(Math::ClampSignal(BufferL[i]) * 32767);
			buffer[i * 2 + 1] = (int16)(Math::ClampSignal(BufferR[i]) * 32767);
		}

		AudioIF_PushSamples(buffer, Count);
	}

private:
	USBD_HandleTypeDef m_Device;
};

#endif