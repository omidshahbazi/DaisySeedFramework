#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBInterfaceCommon.h"
#include "DaisySeedFramework/USB/DaisyUSB.h"

DaisyUSBInterfaceCommon::DaisyUSBInterfaceCommon(DaisyUSB* USB, const Configs& Configs)
	: m_USB(USB),
	m_Configs(Configs),
	m_InterfaceIndexMask(0)
{
	ASSERT(TO_ENDPOINT_NUMBER(m_Configs.EndpointCommand) < USB_EP_COUNT_DEFAULT, "Ran out of endpoints");
	ASSERT(TO_ENDPOINT_NUMBER(m_Configs.EndpointOut) < USB_EP_COUNT_DEFAULT, "Ran out of endpoints");
	ASSERT(TO_ENDPOINT_NUMBER(m_Configs.EndpointIn) < USB_EP_COUNT_DEFAULT, "Ran out of endpoints");

	for (uint8 i = 0; i < m_Configs.InterfaceIndexCount; ++i)
		m_InterfaceIndexMask |= (1 << (m_Configs.InterfaceIndexStart + i));
}

bool DaisyUSBInterfaceCommon::MatchByInterfaceIndex(uint8 Index) const
{
	return ((m_InterfaceIndexMask & (1 << Index)) != 0);
}

bool DaisyUSBInterfaceCommon::MatchByEndpoint(uint8 Endpoint) const
{
	//return (m_Configs.EndpointCommand == Endpoint || m_Configs.EndpointOut == Endpoint || m_Configs.EndpointIn == Endpoint);
	return (TO_ENDPOINT_NUMBER(m_Configs.EndpointCommand) == Endpoint ||
		TO_ENDPOINT_NUMBER(m_Configs.EndpointOut) == Endpoint ||
		TO_ENDPOINT_NUMBER(m_Configs.EndpointIn) == Endpoint);
}

void DaisyUSBInterfaceCommon::EndpointPrepareReceive(uint8* Buffer, uint16 Length)
{
	EndpointReceive(Buffer, Length);
}

uint16 DaisyUSBInterfaceCommon::EndpointReceiveCount(void)
{
	return m_USB->DeviceReceiveCount(m_Configs.EndpointOut);
}

void DaisyUSBInterfaceCommon::EndpointReceive(uint8* Buffer, uint16 Length)
{
	m_USB->DeviceReceive(Buffer, Length, m_Configs.EndpointOut);
}

void DaisyUSBInterfaceCommon::EndpointTransmit(const uint8* Buffer, uint16 Length, bool ClearDCache)
{
	m_USB->DeviceTransmit(Buffer, Length, m_Configs.EndpointIn, ClearDCache);
}

void DaisyUSBInterfaceCommon::EndpointFlush(void)
{
	m_USB->FlushEndpoint(m_Configs.EndpointIn);
}

#endif