#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBCDCInterface.h"

DaisyUSBCDCInterface::DaisyUSBCDCInterface(DaisyUSBDevice* Device, const Configs& Configs, const CDCClassConfig& Class)
	: DaisyUSBInterfaceCommon(Device, Configs),
	m_Class(Class),
	m_LineState(0),
	m_IsHostConnected(false),
	m_TransmitHandler(Configs.MaxTransmitPacketSize)
{}

void DaisyUSBCDCInterface::Transmit(const uint8* Buffer, uint16 Length)
{
	ASSERT(m_IsHostConnected, "Host is not connected");

	m_TransmitHandler.Set(Buffer, Length);

	EndpointTransmit(m_TransmitHandler.GetBuffer(), m_TransmitHandler.GetLength());

	m_TransmitHandler.MoveForward();
}

bool DaisyUSBCDCInterface::OnSetupStage(const USBDeviceSetupPacket* Setup)
{
	switch (Setup->bRequest)
	{
	case USB_CDC_REQ_SET_LINE_CODING:
	{
		if (Setup->wLength > 0)
			DeviceReceive(&m_CDCLineCoding);

		DeviceTransmitAck();

		break;
	}

	case USB_CDC_REQ_GET_LINE_CODING:
	{
		DeviceTransmit(&m_CDCLineCoding);
		DeviceReceiveAck();

		break;
	}

	case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
	{
		m_LineState = (uint8)(Setup->wValue & 0xFF);

		bool dtr = ((m_LineState & 0x01) != 0);
		//bool rts = ((m_LineState & 0x02) != 0);

		if (dtr)
			m_IsHostConnected = true;
		else
			m_IsHostConnected = false;

		DeviceTransmitAck();

		break;
	}

	default:
		return false;
	}

	return true;
}

void DaisyUSBCDCInterface::OnSetupCompleted(void)
{
	const Configs& configs = GetConfigs();

	//Order matters here, it must be ordered by the actual number of Command and In
	{
		if (TO_ENDPOINT_NUMBER(configs.EndpointCommand) != 0)
			AllocateTransmitBuffer(configs.EndpointCommand, (uint16)PacketSizes::Max);

		if (TO_ENDPOINT_NUMBER(configs.EndpointIn) != 0)
			AllocateTransmitBuffer(configs.EndpointIn, configs.MaxTransmitPacketSize);
	}

	if (TO_ENDPOINT_NUMBER(configs.EndpointCommand) != 0)
		OpenEndpoint(configs.EndpointCommand, USB_EP_MAX_PACKET_INTR, USBEndpointAttributes::Interrupt);

	if (TO_ENDPOINT_NUMBER(configs.EndpointOut) != 0)
		OpenEndpoint(configs.EndpointOut, configs.MaxReceivePacketSize, USBEndpointAttributes::Bulk);

	if (TO_ENDPOINT_NUMBER(configs.EndpointIn) != 0)
		OpenEndpoint(configs.EndpointIn, configs.MaxTransmitPacketSize, USBEndpointAttributes::Bulk);

	EndpointPrepareReceive(m_ReceiveBuffer, configs.MaxReceivePacketSize);
}

void DaisyUSBCDCInterface::OnDataInStage(void)
{
	if (m_TransmitHandler.HasMore())
	{
		EndpointTransmit(m_TransmitHandler.GetBuffer(), m_TransmitHandler.GetLength());

		m_TransmitHandler.MoveForward();
	}
}

void DaisyUSBCDCInterface::OnDataOutStage(void)
{
	uint16 len = EndpointReceiveCount();
	if (len > 0)
	{
		m_IsHostConnected = true;

		EndpointReceive(m_ReceiveBuffer, len);

		m_ReceiveCallback(m_ReceiveBuffer, len);
	}

	EndpointPrepareReceive(m_ReceiveBuffer, GetConfigs().MaxReceivePacketSize);
}

void DaisyUSBCDCInterface::BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex) const
{
	const Configs& configs = GetConfigs();

	uint8* buffer = EP0Buffer.configDescs;

	USBInterfaceAssociationDescriptor* iad = reinterpret_cast<USBInterfaceAssociationDescriptor*>(buffer + BufferOffset);
	iad->bLength = sizeof(USBInterfaceAssociationDescriptor);
	iad->bDescriptorType = USBDescTypes::InterfaceAssociation;
	iad->bFirstInterface = InterfaceIndex;
	iad->bInterfaceCount = CalculateRequiredInterfaceCount(m_Class);
	iad->bFunctionClass = USBSDeviceClasses::CDC;
	iad->bFunctionSubClass = (uint8)CDCSubClasses::ACM;
	iad->bFunctionProtocol = (uint8)CDCProtocols::AT;
	iad->iFunction = 0;
	BufferOffset += sizeof(USBInterfaceAssociationDescriptor);

	USBInterfaceDescriptor* ctrlIf = reinterpret_cast<USBInterfaceDescriptor*>(buffer + BufferOffset);
	ctrlIf->bLength = sizeof(USBInterfaceDescriptor);
	ctrlIf->bDescriptorType = USBDescTypes::Interface;
	ctrlIf->bInterfaceNumber = InterfaceIndex;
	ctrlIf->bAlternateSetting = 0;
	ctrlIf->bNumEndpoints = 1;
	ctrlIf->bInterfaceClass = USBSDeviceClasses::CDC;
	ctrlIf->bInterfaceSubClass = (uint8)CDCSubClasses::ACM;
	ctrlIf->bInterfaceProtocol = (uint8)CDCProtocols::AT;
	ctrlIf->iInterface = 0;
	BufferOffset += sizeof(USBInterfaceDescriptor);

	USBCDCHeaderFunctionalDescriptor* header = reinterpret_cast<USBCDCHeaderFunctionalDescriptor*>(buffer + BufferOffset);
	header->bFunctionLength = sizeof(USBCDCHeaderFunctionalDescriptor);
	header->bDescriptorType = USBDescTypes::CS_INTERFACE;
	header->bDescriptorSubtype = CDC_SCS_HEADER;
	header->bcdCDC = USB_CDC_BCD_VERSION;
	BufferOffset += sizeof(USBCDCHeaderFunctionalDescriptor);

	USBCDCACMFunctionalDescriptor* acm = reinterpret_cast<USBCDCACMFunctionalDescriptor*>(buffer + BufferOffset);
	acm->bFunctionLength = sizeof(USBCDCACMFunctionalDescriptor);
	acm->bDescriptorType = USBDescTypes::CS_INTERFACE;
	acm->bDescriptorSubtype = CDC_SCS_ACM;
	acm->bmCapabilities = USB_CDC_CAP_FEATURE;
	BufferOffset += sizeof(USBCDCACMFunctionalDescriptor);

	USBCDCUnionFunctionalDescriptor* cdcUnion = reinterpret_cast<USBCDCUnionFunctionalDescriptor*>(buffer + BufferOffset);
	cdcUnion->bFunctionLength = sizeof(USBCDCUnionFunctionalDescriptor);
	cdcUnion->bDescriptorType = USBDescTypes::CS_INTERFACE;
	cdcUnion->bDescriptorSubtype = CDC_SCS_UNION;
	cdcUnion->bMasterInterface = InterfaceIndex;
	cdcUnion->bSlaveInterface0 = InterfaceIndex + 1;
	BufferOffset += sizeof(USBCDCUnionFunctionalDescriptor);

	USBEndpointDescriptor* ctrlEp = reinterpret_cast<USBEndpointDescriptor*>(buffer + BufferOffset);
	ctrlEp->bLength = sizeof(USBEndpointDescriptor);
	ctrlEp->bDescriptorType = USBDescTypes::Endpoint;
	ctrlEp->bEndpointAddress = configs.EndpointCommand;
	ctrlEp->bmAttributes = (uint8)USBEndpointAttributes::Interrupt;
	ctrlEp->wMaxPacketSize = USB_EP_MAX_PACKET_INTR;
	ctrlEp->bInterval = USB_EP_INTERVAL_FS;
	BufferOffset += sizeof(USBEndpointDescriptor);

	USBInterfaceDescriptor* dataIf = reinterpret_cast<USBInterfaceDescriptor*>(buffer + BufferOffset);
	dataIf->bLength = sizeof(USBInterfaceDescriptor);
	dataIf->bDescriptorType = USBDescTypes::Interface;
	dataIf->bInterfaceNumber = InterfaceIndex + 1;
	dataIf->bAlternateSetting = 0;
	dataIf->bNumEndpoints = 2;
	dataIf->bInterfaceClass = USBSDeviceClasses::Data;
	dataIf->bInterfaceSubClass = (uint8)CDCSubClasses::None;
	dataIf->bInterfaceProtocol = (uint8)CDCProtocols::None;
	dataIf->iInterface = 0;
	BufferOffset += sizeof(USBInterfaceDescriptor);

	USBEndpointDescriptor* outEp = reinterpret_cast<USBEndpointDescriptor*>(buffer + BufferOffset);
	outEp->bLength = sizeof(USBEndpointDescriptor);
	outEp->bDescriptorType = USBDescTypes::Endpoint;
	outEp->bEndpointAddress = configs.EndpointOut;
	outEp->bmAttributes = (uint8)USBEndpointAttributes::Bulk;
	outEp->wMaxPacketSize = (uint16)m_Class.ReceiveBufferSize;
	outEp->bInterval = 0;
	BufferOffset += sizeof(USBEndpointDescriptor);

	USBEndpointDescriptor* inEp = reinterpret_cast<USBEndpointDescriptor*>(buffer + BufferOffset);
	inEp->bLength = sizeof(USBEndpointDescriptor);
	inEp->bDescriptorType = USBDescTypes::Endpoint;
	inEp->bEndpointAddress = configs.EndpointIn;
	inEp->bmAttributes = (uint8)USBEndpointAttributes::Bulk;
	inEp->wMaxPacketSize = (uint16)m_Class.SendBufferSize;
	inEp->bInterval = 0;
	BufferOffset += sizeof(USBEndpointDescriptor);
}

#endif