#ifdef ON_WINDOWS

#include "DaisySeedFramework/USB/WindowsUSBDevice.h"

WindowsUSBDevice::WindowsUSBDevice(void)
	: m_Interfaces{},
	m_DeviceCount(0)
{}

void WindowsUSBDevice::Start(uint8 Index, const USBDeviceProfile& Profile)
{
	for (uint8 i = 0; i < Profile.ClassNodeCount; ++i)
	{
		const USBClassNode& node = Profile.ClassNodes[i];
		switch (node.Class)
		{
		case USBDeviceClasses::CDC:
			new (&m_Interfaces[m_DeviceCount]) WindowsUSBCDCInterface;
			m_Interfaces[m_DeviceCount].Start(m_DeviceCount, node.CDC);
			break;

		default:
			ASSERT(false, "Unsupported class type %i", node.Class);
			break;
		}

		m_DeviceCount++;
	}
}

void WindowsUSBDevice::Stop(void)
{
	for (uint8 i = 0; i < m_DeviceCount; ++i)
		m_Interfaces[i].Stop();

	m_DeviceCount = 0;
}

void WindowsUSBDevice::Update(void)
{
	for (uint8 i = 0; i < m_DeviceCount; ++i)
		m_Interfaces[i].Update();
}

#endif