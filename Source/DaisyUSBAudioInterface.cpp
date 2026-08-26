#ifdef ON_HARDWARE

#include "DaisySeedFramework/DaisyUSBAudioInterface.h"
//#include "Libraries/USBD/include/usbd_audio_if.h"

extern "C"
{
	//#include "usbd_core.h"
	//#include "Libraries/USBD/include/usbd_desc.h"
	//#include "Libraries/USBD/include/usbd_audio.h"
}

void DaisyUSBAudioInterface::Start(USBInterfaces Interface) 
{
	//if (Interface == USBInterfaces::Internal)
	//	USBD_Init(&m_Device, &FS_Desc, DEVICE_FS);
	//else
	//	USBD_Init(&m_Device, &HS_Desc, DEVICE_HS);

	//USBD_RegisterClass(&m_Device, &USBD_AUDIO);

	//USBD_AUDIO_RegisterInterface(&m_Device, &USBD_AUDIO_fops);

	//USBD_Start(&m_Device);
}

void DaisyUSBAudioInterface::Stop(void) 
{
	//USBD_Stop(&m_Device);

	//USBD_DeInit(&m_Device);

	//// HAL_NVIC_DisableIRQ(OTG_FS_IRQn);

	//// HAL_PCD_DeInit((PCD_HandleTypeDef *)m_Device->pData);
}

void DaisyUSBAudioInterface::Push(const uint8* BufferL, const uint8* BufferR, uint8 Count) 
{
	//static uint8 buffer[FrameLength * 2];

	//for (uint8 i = 0; i < Count; ++i)
	//{
	//	buffer[i * 2 + 0] = (int16)(Math::ClampSignal(BufferL[i]) * 32767);
	//	buffer[i * 2 + 1] = (int16)(Math::ClampSignal(BufferR[i]) * 32767);
	//}

	//AudioIF_PushSamples(buffer, Count);
}

#endif