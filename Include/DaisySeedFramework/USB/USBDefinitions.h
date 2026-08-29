#pragma once
#ifndef USB_DEFINITIONS_H
#define USB_DEFINITIONS_H

#include <DigitalSignalProcessing/DataTypes.h>
#include <DigitalSignalProcessing/Common.h>

// --- USB Request Types (bmRequestType) ---
#define USB_REQ_TYPE_MASK            0x60
//#define USB_REQ_TYPE_STANDARD        0x00
//#define USB_REQ_TYPE_CLASS           0x20
//#define USB_REQ_TYPE_VENDOR          0x40

// --- USB Standard Requests (bRequest) ---
//#define USB_REQ_GET_STATUS           0x00
//#define USB_REQ_CLEAR_FEATURE        0x01
//#define USB_REQ_SET_FEATURE          0x03
//#define USB_REQ_SET_ADDRESS          0x05
//#define USB_REQ_GET_DESCRIPTOR       0x06
//#define USB_REQ_SET_DESCRIPTOR       0x07
//#define USB_REQ_GET_CONFIGURATION    0x08
//#define USB_REQ_SET_CONFIGURATION    0x09
//#define USB_REQ_GET_INTERFACE        0x0A
//#define USB_REQ_SET_INTERFACE        0x0B

// --- USB Descriptor Types ---
//#define USB_DESC_TYPE_DEVICE         0x01
//#define USB_DESC_TYPE_CONFIGURATION  0x02
//#define USB_DESC_TYPE_STRING         0x03
//#define USB_DESC_TYPE_INTERFACE      0x04
//#define USB_DESC_TYPE_ENDPOINT       0x05

// --- USB String Descriptor Indexes ---
#define USB_STRING_INDEX_LANGID       0x00
#define USB_STRING_INDEX_MANUFACTURER 0x01
#define USB_STRING_INDEX_PRODUCT      0x02
#define USB_STRING_INDEX_SERIAL       0x03

// --- USB Standard Values ---
#define USB_LANGID_ENGLISH_US        0x0409
#define USB_VERSION_2_0              0x0200

// --- USB Device Class / SubClass / Protocol (IAD) ---
#define USB_CLASS_MISC               0xEF
#define USB_SUBCLASS_COMMON          0x02
#define USB_PROTOCOL_IAD             0x01

// --- USB Endpoints ---
#define USB_EP0_OUT                  0x00
#define USB_EP0_IN                   0x80

// --- USB Class Code Definitions ---
#define USB_CLASS_CDC                0x02
#define USB_CLASS_DATA               0x0A

// --- CDC SubClass & Protocol ---
#define CDC_SUBCLASS_ACM             0x02
#define CDC_PROTOCOL_AT              0x01

// --- CDC Descriptor Types & Subtypes ---
#define CDC_CS_INTERFACE             0x24
#define CDC_SCS_HEADER               0x00
#define CDC_SCS_ACM                  0x02
#define CDC_SCS_UNION                0x06

// --- USB Request Direction (bmRequestType Bit 7) ---
//#define USB_REQ_DIR_MASK             0x80
#define USB_REQ_DIR_HOST_TO_DEVICE   0x00
#define USB_REQ_DIR_DEVICE_TO_HOST   0x80

#define USB_CDC_REQ_SET_LINE_CODING           0x20
#define USB_CDC_REQ_GET_LINE_CODING           0x21
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE    0x22

static constexpr uint8 MaxPacketSize = 64;

BEGIN_PACK(1);
struct USBDeviceSetupPacket
{
public:
	uint8 bmRequestType;
	uint8 bRequest;
	uint16 wValue;
	uint16 wIndex;
	uint16 wLength;
};
END_PACK();

BEGIN_PACK(1);
struct USBDeviceDescriptor
{
public:
	uint8 bLength;
	uint8 bDescriptorType;
	uint16 bcdUSB;
	uint8 bDeviceClass;   // Miscellaneous (IAD)
	uint8 bDeviceSubClass;
	uint8 bDeviceProtocol;
	uint8 bMaxPacketSize0;
	uint16 idVendor;
	uint16 idProduct;
	uint16 bcdDevice;
	uint8 iManufacturer;
	uint8 iProduct;
	uint8 iSerialNumber;
	uint8 bNumConfigurations;
};
END_PACK();

BEGIN_PACK(1);
struct USBStringDescriptor
{
public:
	uint8 bLength;
	uint8 bDescriptorType;
	uint16 wData[31];
};
END_PACK();

BEGIN_PACK(1);
struct USBConfigurationDescriptor
{
public:
	uint8 bLength;
	uint8 bDescriptorType;
	uint16 wTotalLength;
	uint8 bNumInterfaces;
	uint8 bConfigurationValue;
	uint8 iConfiguration;
	uint8 bmAttributes;
	uint8 bMaxPower;
};
END_PACK();

BEGIN_PACK(1);
struct USBInterfaceAssociationDescriptor
{
public:
	uint8  bLength;
	uint8  bDescriptorType;
	uint8  bFirstInterface;
	uint8  bInterfaceCount;
	uint8  bFunctionClass;
	uint8  bFunctionSubClass;
	uint8  bFunctionProtocol;
	uint8  iFunction;
};
END_PACK();

BEGIN_PACK(1);
struct USBInterfaceDescriptor
{
public:
	uint8  bLength;
	uint8  bDescriptorType;
	uint8  bInterfaceNumber;
	uint8  bAlternateSetting;
	uint8  bNumEndpoints;
	uint8  bInterfaceClass;
	uint8  bInterfaceSubClass;
	uint8  bInterfaceProtocol;
	uint8  iInterface;
};
END_PACK();

BEGIN_PACK(1);
struct USBEndpointDescriptor
{
public:
	uint8  bLength;
	uint8  bDescriptorType;
	uint8  bEndpointAddress;
	uint8  bmAttributes;
	uint16 wMaxPacketSize;
	uint8  bInterval;
};
END_PACK();

BEGIN_PACK(1);
struct USBCDCHeaderFunctionalDescriptor
{
public:
	uint8 bFunctionLength;
	uint8 bDescriptorType;
	uint8 bDescriptorSubtype;
	uint16 bcdCDC;
};
END_PACK();

BEGIN_PACK(1);
struct USBCDCACMFunctionalDescriptor
{
public:
	uint8 bFunctionLength;
	uint8 bDescriptorType;
	uint8 bDescriptorSubtype;
	uint8 bmCapabilities;
};
END_PACK();

BEGIN_PACK(1);
struct USBCDCUnionFunctionalDescriptor
{
public:
	uint8 bFunctionLength;
	uint8 bDescriptorType;
	uint8 bDescriptorSubtype;
	uint8 bMasterInterface;
	uint8 bSlaveInterface0;
};
END_PACK();

//BEGIN_PACK(1);
//struct USBAudioConfiguration
//{
//	USBConfigurationDescriptor config;
//
//	USBInterfaceDescriptor     controlInterface;   
//	AudioHeaderDescriptor      audioHeader;        
//	AudioInputTerminal         inputTerminal;      
//	AudioFeatureUnit           featureUnit;        
//	AudioOutputTerminal        outputTerminal;     
//
//	USBInterfaceDescriptor     streamingInterface; 
//	AudioFormatDescriptor      audioFormat;        
//	USBEndpointDescriptor      streamingEndpoint;  
//	AudioEndpointDescriptor    audioDataEndpoint;  
//};
//END_PACK();

BEGIN_PACK(1);
struct alignas(4) EP0Buffer
{
public:
	union
	{
		USBDeviceDescriptor deviceDesc;
		USBStringDescriptor stringDesc;

		uint8 configDescs[512];
	};
};
END_PACK();

#endif