#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSB.h"
#include <DigitalSignalProcessing/Debug.h>

static DaisyUSB* s_Instance[(uint8_t)Peripherals::COUNT] = {};

extern "C"
{
	void OTG_FS_IRQHandler(void)
	{
		s_Instance[(uint8_t)Peripherals::FullSpeed]->OnHALHandleRequest();
	}

	void OTG_HS_IRQHandler(void)
	{
		s_Instance[(uint8_t)Peripherals::HighSpeed]->OnHALHandleRequest();
	}
}

DaisyUSB::DaisyUSB(Peripherals Peripheral)
	: m_Peripheral(Peripheral),
	m_IsRunning(false)
{
	s_Instance[(uint8_t)Peripheral] = this;
}

void DaisyUSB::Update(void)
{
	if (m_Profile.Mode == USBModes::Device)
		m_Device.Update();
	else
		m_Host.Update();
}

void DaisyUSB::Start(const USBProfile& Profile)
{
	ASSERT(!m_IsRunning, "Interface has already started.");

	m_Profile = Profile;

	if (Profile.Mode == USBModes::Device)
	{
		new (&m_Device) DaisyUSBDevice(m_Peripheral);
		m_Device.Start(Profile.Device);
	}
	else
	{
		new (&m_Host) DaisyUSBHost(m_Peripheral);
		m_Host.Start(Profile.Host);
	}

	m_IsRunning = true;
}

void DaisyUSB::Stop(void)
{
	ASSERT(m_IsRunning, "Interface is not started.");

	if (m_Profile.Mode == USBModes::Device)
		m_Device.Stop();
	else
		m_Host.Stop();

	HAL_Delay(100);

	m_IsRunning = false;
}

void DaisyUSB::OnHALHandleRequest(void)
{
	if (m_Profile.Mode == USBModes::Device)
		m_Device.OnHALHandleRequest();
	else
		m_Host.OnHALHandleRequest();
}

#endif