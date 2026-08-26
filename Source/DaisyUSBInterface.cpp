#ifdef ON_HARDWARE

#include "DaisySeedFramework/DaisyUSBInterface.h"

DaisyUSBInterface::DaisyUSBInterface(daisy::DaisySeed* Hardware)
	: m_Hardware(Hardware),
	m_IsStarted(false),
	m_Buffer(nullptr)
{}

void DaisyUSBInterface::Start(USBInterfaces Interface)
{
	//ASSERT(!m_IsStarted, "It's already started");

	//if (m_Buffer == nullptr)
	//	m_Buffer = Memory::Allocate<BufferType>(true);

	//new (m_Buffer) BufferType();

	//daisy::UsbHandle::UsbPeriph periph = daisy::UsbHandle::UsbPeriph::FS_INTERNAL;
	//if (Interface == USBInterfaces::External)
	//	periph = daisy::UsbHandle::UsbPeriph::FS_EXTERNAL;

	//m_Hardware->usb_handle.Init(periph);
	//m_Hardware->usb_handle.SetReceiveCallback(Callback, periph);

	//m_IsStarted = true;
}

void DaisyUSBInterface::Stop(void)
{
	//ASSERT(m_IsStarted, "It's not started");

	//daisy::UsbHandle::UsbPeriph periph = daisy::UsbHandle::UsbPeriph::FS_INTERNAL;
	//if constexpr (External)
	//	periph = daisy::UsbHandle::UsbPeriph::FS_EXTERNAL;

	//m_Hardware->usb_handle.SetReceiveCallback(nullptr, periph);
	//m_Hardware->usb_handle.DeInit(periph);

	//HandleIncomings();

	//m_IsStarted = false;
}

void DaisyUSBInterface::Update(void)
{
	//if (!m_IsStarted)
	//	return;

	//HandleIncomings();
}

void DaisyUSBInterface::HandleIncomings(void)
{
	//if (m_Buffer->IsEmpty())
	//	return;

	//if (m_Callback != nullptr)
	//	m_Callback(m_Buffer->GetData(), m_Buffer->GetSize());

	//m_Buffer->Clear();
}

void DaisyUSBInterface::TransmitFragmented(const uint8* Buffer, uint16 Length, uint16 Delay) const
{
	//uint16 index = 0;
	//while (index < Length)
	//{
	//	const uint16 CountPerStep = 64;

	//	uint16 countPerStep = (uint16)Math::Min(CountPerStep, Length - index);

	//	if constexpr (External)
	//		m_Hardware->usb_handle.TransmitExternal(const_cast<uint8*>(Buffer + index), countPerStep);
	//	else
	//		m_Hardware->usb_handle.TransmitInternal(const_cast<uint8*>(Buffer + index), countPerStep);

	//	index += CountPerStep;

	//	m_Hardware->DelayMs(Delay);
	//}
}

void DaisyUSBInterface::Callback(uint8* Buffer, uint32_t* Length)
{
	//if (Buffer == nullptr)
	//	return;

	//if (*Length == 0)
	//	return;

	//m_Buffer->Clear();

	//m_Buffer->PushBack(Buffer, *Length);
}

#endif