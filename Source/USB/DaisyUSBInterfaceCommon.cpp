#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBInterfaceCommon.h"
#include "DaisySeedFramework/USB/DaisyUSBDevice.h"

DaisyUSBInterfaceCommon::DaisyUSBInterfaceCommon(DaisyUSBDevice* Device, const Configs& Configs)
	: m_Device(Device),
	m_Configs(Configs),
	m_InterfaceIndexMask(0),
	m_PendingReceiveType(0),
	m_PendingReceiveBuffer{}
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

void DaisyUSBInterfaceCommon::AllocateTransmitBuffer(uint8 Endpoint, uint16 Size)
{
	m_Device->AllocateTransmitBuffer(Endpoint, Size);
}

void DaisyUSBInterfaceCommon::OpenEndpoint(uint8 Endpoint, uint16 Length, USBEndpointAttributes Type)
{
	m_Device->OpenEndpoint(Endpoint, Length, Type);
}

void DaisyUSBInterfaceCommon::CloseEndpoint(uint8 Endpoint)
{
	m_Device->CloseEndpoint(Endpoint);
}

void DaisyUSBInterfaceCommon::DeviceReceive(uint8* Buffer, uint16 Length, uint8 Endpoint)
{
	m_Device->DeviceReceive(Buffer, Length, Endpoint);
}

void DaisyUSBInterfaceCommon::DeviceReceiveAck(void)
{
	m_Device->DeviceReceiveAck();
}

void DaisyUSBInterfaceCommon::DeviceTransmit(const uint8* Buffer, uint16 Length, uint8 Endpoint, bool ClearDCache)
{
	m_Device->DeviceTransmit(Buffer, Length, Endpoint, ClearDCache);
}

void DaisyUSBInterfaceCommon::DeviceTransmitAck(void)
{
	m_Device->DeviceTransmitAck();
}

uint16 DaisyUSBInterfaceCommon::EndpointReceiveCount(void)
{
	return m_Device->DeviceReceiveCount(m_Configs.EndpointOut);
}

void DaisyUSBInterfaceCommon::EndpointReceiveFlush(void)
{
	m_Device->FlushEndpoint(m_Configs.EndpointOut);
}

void DaisyUSBInterfaceCommon::EndpointTransmitFlush(void)
{
	m_Device->FlushEndpoint(m_Configs.EndpointIn);
}

#endif