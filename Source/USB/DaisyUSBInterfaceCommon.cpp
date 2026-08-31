#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBInterfaceCommon.h"
#include "DaisySeedFramework/USB/DaisyUSB.h"

DaisyUSBInterfaceCommon::DaisyUSBInterfaceCommon(DaisyUSB* USB, uint16 InterfaceIndexMask, uint8 EndpointCommand, uint8 EndpointIn, uint8 EndpointOut)
	: m_USB(USB),
	m_InterfaceIndexMask(InterfaceIndexMask),
	m_EndpointCommand(EndpointCommand),
	m_EndpointIn(EndpointIn),
	m_EndpointOut(EndpointOut)
{}

void DaisyUSBInterfaceCommon::OpenEndpoints(void)
{
	m_USB->OpenEndpoint(m_EndpointCommand, USB_EP_MAX_PACKET_INTR, USBEpAttr::Interrupt);
	m_USB->OpenEndpoint(m_EndpointOut, MaxPacketSize, USBEpAttr::Bulk);
	m_USB->OpenEndpoint(m_EndpointIn, MaxPacketSize, USBEpAttr::Bulk);
}

uint16 DaisyUSBInterfaceCommon::EndpointReceiveCount(void)
{
	return m_USB->DeviceReceiveCount(m_EndpointOut);
}

void DaisyUSBInterfaceCommon::EndpointReceive(uint8* Buffer, uint16 Length)
{
	m_USB->DeviceReceive(Buffer, Length, m_EndpointOut);
}

void DaisyUSBInterfaceCommon::EndpointTransmit(const uint8* Buffer, uint16 Length)
{
	m_USB->DeviceTransmit(Buffer, Length, m_EndpointIn);
}

bool DaisyUSBInterfaceCommon::MatchByInterfaceIndex(uint8 Index)
{
	return ((m_InterfaceIndexMask & (1 << Index)) != 0);
}

bool DaisyUSBInterfaceCommon::MatchByEndpoint(uint8 Endpoint)
{
	//return (m_EndpointCommand == Endpoint || m_EndpointOut == Endpoint || m_EndpointIn == Endpoint);
	return ((m_EndpointCommand & 0x7F) == Endpoint ||
		(m_EndpointOut & 0x7F) == Endpoint ||
		(m_EndpointIn & 0x7F) == Endpoint);
}

#endif