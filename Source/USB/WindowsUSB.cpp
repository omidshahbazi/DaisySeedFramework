#ifdef ON_WINDOWS

#include "DaisySeedFramework/USB/WindowsUSB.h"

WindowsUSB::WindowsUSB(void)
{}

void WindowsUSB::Start(const USBProfile& Profile)
{
	if (Profile.Mode == USBModes::Device)
		m_Device.Start(0, Profile.Device);
	else
		ASSERT(false, "Host mode is not supported on Windows");
}

void WindowsUSB::Stop(void)
{
	m_Device.Stop();
}

void WindowsUSB::Update(void)
{
	m_Device.Update();
}

#endif
