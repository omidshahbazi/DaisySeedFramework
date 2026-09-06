#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBHost.h"
#include <DigitalSignalProcessing/Debug.h>
#include <DigitalSignalProcessing/StringUtils.h>
#include <DigitalSignalProcessing/Math.h>
#include <DigitalSignalProcessing/Memory.h>

static DaisyUSBHost* s_Instance[(uint8)Peripherals::COUNT] = {};

extern "C"
{
}

#define CHECK_CALL(Expr) ASSERT((Expr) == HAL_OK, #Expr);

DaisyUSBHost::DaisyUSBHost(Peripherals Peripheral)
	: m_Peripheral(Peripheral),
	m_IsRunning(false)
{
	s_Instance[(uint8)Peripheral] = this;
}

void DaisyUSBHost::Start(const USBHostProfile& Profile)
{
	ASSERT(!m_IsRunning, "Interface has already started.");
	ASSERT(0 < Profile.SupportedClassCount && Profile.SupportedClassCount <= MaxClassCount, "Invalid SupportedClassCount");

	m_Profile = Profile;

	ASSERT(false, "Not implemented");

	if (m_Peripheral == Peripherals::HighSpeed)
	{
		m_HostHandle.Instance = USB_OTG_HS;
		m_HostHandle.Init.speed = HCD_SPEED_HIGH;
		m_HostHandle.Init.phy_itface = PCD_PHY_ULPI;
	}
	else
	{
		m_HostHandle.Instance = USB_OTG_FS;
		m_HostHandle.Init.speed = HCD_SPEED_FULL;
		m_HostHandle.Init.phy_itface = PCD_PHY_EMBEDDED;
	}

	m_HostHandle.Init.Host_channels = USB_HCD_CHANNELS_DEFAULT;
	m_HostHandle.Init.Sof_enable = DISABLE;
	m_HostHandle.Init.vbus_sensing_enable = DISABLE;

	CHECK_CALL(HAL_HCD_Init(&m_HostHandle));
	CHECK_CALL(HAL_HCD_Start(&m_HostHandle));

	m_IsRunning = true;
}

void DaisyUSBHost::Stop(void)
{
	ASSERT(m_IsRunning, "Interface is not started.");

	CHECK_CALL(HAL_HCD_Stop(&m_HostHandle));
	CHECK_CALL(HAL_HCD_DeInit(&m_HostHandle));

	m_IsRunning = false;
}

void DaisyUSBHost::OnHALHandleRequest(void)
{
	HAL_HCD_IRQHandler(&m_HostHandle);
}

#endif