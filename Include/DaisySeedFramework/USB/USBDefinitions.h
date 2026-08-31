#pragma once
#ifndef USB_DEFINITIONS_H
#define USB_DEFINITIONS_H

#include <DigitalSignalProcessing/DataTypes.h>
#include <DigitalSignalProcessing/Common.h>

// Request type mask and standard request directions
#define USB_REQ_TYPE_MASK            0x60 // Mask for request type bits
#define USB_REQ_DIR_HOST_TO_DEVICE   0x00 // Request direction: Host to Device
#define USB_REQ_DIR_DEVICE_TO_HOST   0x80 // Request direction: Device to Host

// String descriptor index definitions
#define USB_STRING_INDEX_LANGID       0x00 // Language ID string index
#define USB_STRING_INDEX_MANUFACTURER 0x01 // Manufacturer string index
#define USB_STRING_INDEX_PRODUCT      0x02 // Product string index
#define USB_STRING_INDEX_SERIAL       0x03 // Serial number string index

// Default configurations and language ID values
#define USB_LANGID_ENGLISH_US        0x0409 // English (United States) language ID
#define USB_VERSION_2_0              0x0200 // USB 2.0 protocol version specification
#define USB_EP0_OUT                  0x00   // Control endpoint 0 OUT address
#define USB_EP0_IN                   0x80   // Control endpoint 0 IN address
#define USB_EP_COUNT_DEFAULT         9      // Default endpoint count for initialization
#define USB_HCD_CHANNELS_DEFAULT     16     // Default host channels count
#define USB_CONFIG_VALUE_DEFAULT     1      // Default active configuration value

// Endpoint packet size and polling interval defaults
#define USB_EP_MAX_PACKET_INTR       8  // Maximum packet size for interrupt endpoints
#define USB_EP_INTERVAL_FS           10 // Polling interval for Full-Speed endpoints (ms)
#define USB_EP_INTERVAL_HS           6  // Polling interval for High-Speed endpoints (microframes)

#define TO_ENDPOINT_NUMBER(Endpoint) ((uint8)(Endpoint & 0x7F))
#define TO_OUT_ENDPOINT(EndpointNumber) ((uint8)(USB_EP0_OUT | EndpointNumber))
#define TO_IN_ENDPOINT(EndpointNumber) ((uint8)(USB_EP0_IN | EndpointNumber))

// Global sizing constants for buffers and strings
//static constexpr uint8 MaxPacketSize = 64;     // Maximum control/bulk endpoint packet size
static constexpr uint8 USBMaxStringLength = 31; // Maximum character length for string descriptors

// USB standard descriptor types enumeration
enum class USBDescType : uint8
{
	Device = 0x01,                // Device descriptor type
	Configuration = 0x02,         // Configuration descriptor type
	String = 0x03,                // String descriptor type
	Interface = 0x04,             // Interface descriptor type
	Endpoint = 0x05,              // Endpoint descriptor type
	InterfaceAssociation = 0x0B,  // Interface Association Descriptor (IAD) type
	CDCFunc = 0x24                // Class-specific functional descriptor type
};

// USB device class codes enumeration
enum class USBSDeviceClass : uint8
{
	CDC = 0x02,   // Communications Device Class
	Data = 0x0A,  // Data Interface Class
	Misc = 0xEF   // Miscellaneous Device Class (often used for IAD)
};

// USB device subclass codes enumeration
enum class USBDeviceSubClass : uint8
{
	None = 0x00,    // No subclass specified
	Common = 0x02   // Common subclass designation
};

// USB device protocol codes enumeration
enum class USBDeviceProtocol : uint8
{
	None = 0x00,    // No protocol specified
	IAD = 0x01,     // Interface Association Descriptor protocol implementation
	Vendor = 0xFF   // Vendor-specific protocol
};

// USB endpoint attributes/transfer types enumeration
enum class USBEpAttr : uint8
{
	Control = 0x00,    // Control transfer type
	Isochronous = 0x01,// Isochronous transfer type
	Bulk = 0x02,       // Bulk transfer type
	Interrupt = 0x03   // Interrupt transfer type
};

// USB configuration power attribute masks enumeration
enum class USBConfigAttr : uint8
{
	SelfPoweredMask = 0xC0, // Device is self-powered (bit 6 set, bit 7 reserved)
	BusPoweredMask = 0x80   // Device is bus-powered
};

BEGIN_PACK(1);
// Standard USB setup packet structure for control transfers
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
// Standard USB Device Descriptor structure
struct USBDeviceDescriptor
{
public:
	uint8 bLength;
	USBDescType bDescriptorType;
	uint16 bcdUSB;
	USBSDeviceClass bDeviceClass;
	USBDeviceSubClass bDeviceSubClass;
	USBDeviceProtocol bDeviceProtocol;
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
// Standard USB String Descriptor structure
struct USBStringDescriptor
{
public:
	uint8 bLength;
	USBDescType bDescriptorType;
	uint16 wData[USBMaxStringLength];
};
END_PACK();

BEGIN_PACK(1);
// Standard USB Configuration Descriptor structure
struct USBConfigurationDescriptor
{
public:
	uint8 bLength;
	USBDescType bDescriptorType;
	uint16 wTotalLength;
	uint8 bNumInterfaces;
	uint8 bConfigurationValue;
	uint8 iConfiguration;
	USBConfigAttr bmAttributes;
	uint8 bMaxPower;
};
END_PACK();

BEGIN_PACK(1);
// USB Interface Association Descriptor (IAD) structure
struct USBInterfaceAssociationDescriptor
{
public:
	uint8  bLength;
	USBDescType  bDescriptorType;
	uint8  bFirstInterface;
	uint8  bInterfaceCount;
	USBSDeviceClass  bFunctionClass;
	uint8  bFunctionSubClass;
	uint8  bFunctionProtocol;
	uint8  iFunction;
};
END_PACK();

BEGIN_PACK(1);
// Standard USB Interface Descriptor structure
struct USBInterfaceDescriptor
{
public:
	uint8  bLength;
	USBDescType  bDescriptorType;
	uint8  bInterfaceNumber;
	uint8  bAlternateSetting;
	uint8  bNumEndpoints;
	USBSDeviceClass  bInterfaceClass;
	uint8  bInterfaceSubClass;
	uint8  bInterfaceProtocol;
	uint8  iInterface;
};
END_PACK();

BEGIN_PACK(1);
// Standard USB Endpoint Descriptor structure
struct USBEndpointDescriptor
{
public:
	uint8  bLength;
	USBDescType  bDescriptorType;
	uint8  bEndpointAddress;
	USBEpAttr  bmAttributes;
	uint16 wMaxPacketSize;
	uint8  bInterval;
};
END_PACK();

BEGIN_PACK(1);
// Endpoint 0 control buffer container union
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

class BufferTransmitHandler
{
public:
	BufferTransmitHandler(uint16 ChunkSize)
		:m_ChunkSize(ChunkSize)
	{}

	template<typename T>
	void Set(const T* Buffer)
	{
		Set(Buffer, sizeof(T));
	}

	template<typename T>
	void Set(const T* Buffer, uint16 Length)
	{
		m_BufferStart = reinterpret_cast<const uint8*>(Buffer);
		m_RemainingLength = Length;
	}

	void MoveForward(void)
	{
		if (m_RemainingLength < m_ChunkSize)
		{
			m_BufferStart = nullptr;
			m_RemainingLength = 0;
		}
		else
		{
			m_BufferStart += m_ChunkSize;
			m_RemainingLength -= m_ChunkSize;
		}
	}

	const uint8* GetBuffer(void) const
	{
		return m_BufferStart;
	}

	uint16 GetLength(void) const
	{
		if (m_RemainingLength > m_ChunkSize)
			return m_ChunkSize;

		return m_RemainingLength;
	}

	bool HasMore(void) const
	{
		return (m_RemainingLength != 0);
	}

private:
	uint16 m_ChunkSize;
	const uint8* m_BufferStart;
	uint16 m_RemainingLength;
};


#endif