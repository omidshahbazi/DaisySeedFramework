#ifdef ON_WINDOWS

#include "DaisySeedFramework/USB/WindowsUSBDevice.h"

WindowsUSBDevice::WindowsUSBDevice(void)
	: m_Interfaces{}
{}

void WindowsUSBDevice::Start(uint8_t Index, const USBDeviceProfile& Profile)
{
	for (uint8_t i = 0; i < Profile.ClassNodeCount; ++i)
	{
		const USBClassNode& node = Profile.ClassNodes[i];
		switch (node.Class)
		{
		case USBDeviceClasses::CDC:
		{
			m_Interfaces.Extend(1);
			uint8_t index = m_Interfaces.GetSize() - 1;

			WindowsUSBCDCInterface& ref = m_Interfaces[index];
			new (&ref) WindowsUSBCDCInterface;
			ref.Start(index, node.CDC);

			break;
		}

		default:
			NOT_IMPLEMENTED();
		}
	}
}

void WindowsUSBDevice::Stop(void)
{
	for (uint8_t i = 0; i < m_Interfaces.GetSize(); ++i)
		m_Interfaces[i].Stop();

	m_Interfaces.Clear();
}

void WindowsUSBDevice::Update(void)
{
	for (uint8_t i = 0; i < m_Interfaces.GetSize(); ++i)
		m_Interfaces[i].Update();
}

#endif