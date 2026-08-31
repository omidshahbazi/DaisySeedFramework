#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBCDCInterface.h"
#include "DaisySeedFramework/USB/DaisyUSB.h"

DaisyUSBCDCInterface::DaisyUSBCDCInterface(DaisyUSB* USB, uint16 InterfaceIndexMask, uint8 EndpointCommand, uint8 EndpointIn, uint8 EndpointOut)
	: DaisyUSBInterfaceCommon(USB, InterfaceIndexMask, EndpointCommand, EndpointIn, EndpointOut),
	m_LineState(0),
	m_IsHostConnected(false)
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
			GetUSB()->DeviceReceive(&m_CDCLineCoding);

		GetUSB()->DeviceTransmitAck();

		break;
	}

	case USB_CDC_REQ_GET_LINE_CODING:
	{
		GetUSB()->DeviceTransmit(&m_CDCLineCoding);
		GetUSB()->DeviceReceiveAck();

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

		GetUSB()->DeviceTransmitAck();

		break;
	}

	default:
		return false;
	}

	return true;
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

	EndpointReceive(m_ReceiveBuffer, MaxPacketSize);
}

void DaisyUSBCDCInterface::OnReady(void)
{
	OpenEndpoints();

	EndpointReceive(m_ReceiveBuffer, MaxPacketSize);
}

void DaisyUSBCDCInterface::BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& Offset, uint8 InterfaceIndex, uint8 Interval, const CDCClassConfig& Config)
{
	uint8* buffer = EP0Buffer.configDescs;

	USBInterfaceAssociationDescriptor* iad = reinterpret_cast<USBInterfaceAssociationDescriptor*>(buffer + Offset);
	iad->bLength = sizeof(USBInterfaceAssociationDescriptor);
	iad->bDescriptorType = USBDescType::InterfaceAssociation;
	iad->bFirstInterface = InterfaceIndex;
	iad->bInterfaceCount = 2;
	iad->bFunctionClass = USBSDeviceClass::CDC;
	iad->bFunctionSubClass = (uint8)CDCSubClass::ACM;
	iad->bFunctionProtocol = (uint8)CDCProtocol::AT;
	iad->iFunction = 0;
	Offset += sizeof(USBInterfaceAssociationDescriptor);

	USBInterfaceDescriptor* ctrlIf = reinterpret_cast<USBInterfaceDescriptor*>(buffer + Offset);
	ctrlIf->bLength = sizeof(USBInterfaceDescriptor);
	ctrlIf->bDescriptorType = USBDescType::Interface;
	ctrlIf->bInterfaceNumber = InterfaceIndex;
	ctrlIf->bAlternateSetting = 0;
	ctrlIf->bNumEndpoints = 1;
	ctrlIf->bInterfaceClass = USBSDeviceClass::CDC;
	ctrlIf->bInterfaceSubClass = (uint8)CDCSubClass::ACM;
	ctrlIf->bInterfaceProtocol = (uint8)CDCProtocol::AT;
	ctrlIf->iInterface = 0;
	Offset += sizeof(USBInterfaceDescriptor);

	USBCDCHeaderFunctionalDescriptor* header = reinterpret_cast<USBCDCHeaderFunctionalDescriptor*>(buffer + Offset);
	header->bFunctionLength = sizeof(USBCDCHeaderFunctionalDescriptor);
	header->bDescriptorType = USBDescType::CDCFunc;
	header->bDescriptorSubtype = CDC_SCS_HEADER;
	header->bcdCDC = USB_CDC_BCD_VERSION;
	Offset += sizeof(USBCDCHeaderFunctionalDescriptor);

	USBCDCACMFunctionalDescriptor* acm = reinterpret_cast<USBCDCACMFunctionalDescriptor*>(buffer + Offset);
	acm->bFunctionLength = sizeof(USBCDCACMFunctionalDescriptor);
	acm->bDescriptorType = USBDescType::CDCFunc;
	acm->bDescriptorSubtype = CDC_SCS_ACM;
	acm->bmCapabilities = USB_CDC_CAP_FEATURE;
	Offset += sizeof(USBCDCACMFunctionalDescriptor);

	USBCDCUnionFunctionalDescriptor* cdcUnion = reinterpret_cast<USBCDCUnionFunctionalDescriptor*>(buffer + Offset);
	cdcUnion->bFunctionLength = sizeof(USBCDCUnionFunctionalDescriptor);
	cdcUnion->bDescriptorType = USBDescType::CDCFunc;
	cdcUnion->bDescriptorSubtype = CDC_SCS_UNION;
	cdcUnion->bMasterInterface = InterfaceIndex;
	cdcUnion->bSlaveInterface0 = InterfaceIndex + 1;
	Offset += sizeof(USBCDCUnionFunctionalDescriptor);

	USBEndpointDescriptor* ctrlEp = reinterpret_cast<USBEndpointDescriptor*>(buffer + Offset);
	ctrlEp->bLength = sizeof(USBEndpointDescriptor);
	ctrlEp->bDescriptorType = USBDescType::Endpoint;
	ctrlEp->bEndpointAddress = Config.CustomEndpointCommand;
	ctrlEp->bmAttributes = USBEpAttr::Interrupt;
	ctrlEp->wMaxPacketSize = USB_EP_MAX_PACKET_INTR;
	ctrlEp->bInterval = Interval;
	Offset += sizeof(USBEndpointDescriptor);

	USBInterfaceDescriptor* dataIf = reinterpret_cast<USBInterfaceDescriptor*>(buffer + Offset);
	dataIf->bLength = sizeof(USBInterfaceDescriptor);
	dataIf->bDescriptorType = USBDescType::Interface;
	dataIf->bInterfaceNumber = InterfaceIndex + 1;
	dataIf->bAlternateSetting = 0;
	dataIf->bNumEndpoints = 2;
	dataIf->bInterfaceClass = USBSDeviceClass::Data;
	dataIf->bInterfaceSubClass = (uint8)CDCSubClass::None;
	dataIf->bInterfaceProtocol = (uint8)CDCSubClass::None;
	dataIf->iInterface = 0;
	Offset += sizeof(USBInterfaceDescriptor);

	USBEndpointDescriptor* outEp = reinterpret_cast<USBEndpointDescriptor*>(buffer + Offset);
	outEp->bLength = sizeof(USBEndpointDescriptor);
	outEp->bDescriptorType = USBDescType::Endpoint;
	outEp->bEndpointAddress = Config.CustomEndpointOut;
	outEp->bmAttributes = USBEpAttr::Bulk;
	outEp->wMaxPacketSize = MaxPacketSize;
	outEp->bInterval = 0;
	Offset += sizeof(USBEndpointDescriptor);

	USBEndpointDescriptor* inEp = reinterpret_cast<USBEndpointDescriptor*>(buffer + Offset);
	inEp->bLength = sizeof(USBEndpointDescriptor);
	inEp->bDescriptorType = USBDescType::Endpoint;
	inEp->bEndpointAddress = Config.CustomEndpointIn;
	inEp->bmAttributes = USBEpAttr::Bulk;
	inEp->wMaxPacketSize = MaxPacketSize;
	inEp->bInterval = 0;
	Offset += sizeof(USBEndpointDescriptor);
}

#endif