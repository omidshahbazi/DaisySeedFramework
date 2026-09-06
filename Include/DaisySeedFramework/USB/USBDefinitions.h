#pragma once
#ifndef USB_DEFINITIONS_H
#define USB_DEFINITIONS_H

#include <DigitalSignalProcessing/DataTypes.h>
#include <DigitalSignalProcessing/Common.h>

// bmRequestType field layout: D6..5 select the request type, D4..0 select
// the recipient. USB_REQ_TYPE_MASK isolates the type bits; the request
// direction (D7) is checked separately against the DIR_* values below.
#define USB_REQ_TYPE_MASK            0x60 // Mask isolating the Type bits (D6..5) of bmRequestType
#define USB_REQ_DIR_HOST_TO_DEVICE   0x00 // bmRequestType D7=0: data flows host -> device (OUT)
#define USB_REQ_DIR_DEVICE_TO_HOST   0x80 // bmRequestType D7=1: data flows device -> host (IN)
#define USB_REQ_RECIPIENT_MASK       0x1F // Mask isolating the Recipient bits (D4..0) of bmRequestType

// Fixed string descriptor indices used by the standard Device Descriptor's
// iManufacturer/iProduct/iSerialNumber fields (index 0 is reserved for the
// language ID list, not an actual text string).
#define USB_STRING_INDEX_LANGID			0x00 // Index 0: not text, returns the list of supported language IDs
#define USB_STRING_INDEX_MANUFACTURER	0x01 // Index of the manufacturer name string
#define USB_STRING_INDEX_PRODUCT		0x02 // Index of the product name string
#define USB_STRING_INDEX_SERIAL			0x03 // Index of the serial number string
#define USB_STRING_TERMINAL_1			0x04 // Index of the first terminal string
#define USB_STRING_TERMINAL_2			0x05 // Index of the second terminal string
#define USB_STRING_TERMINAL_3			0x06 // Index of the third terminal string
#define USB_STRING_TERMINAL_4			0x07 // Index of the fourth terminal string

// Default configuration and language ID values
#define USB_LANGID_ENGLISH_US        0x0409 // Language ID: English (United States), used for all string descriptors here
#define USB_VERSION_2_0              0x0200 // bcdUSB value: USB 2.0 (device descriptor claims this even when running Full-Speed only)
#define USB_EP0_OUT                  0x00   // Address of the control endpoint's OUT direction
#define USB_EP0_IN                   0x80   // Address of the control endpoint's IN direction (bit 7 = direction)
#define USB_EP_COUNT_DEFAULT         9      // Endpoint count passed to HAL_PCD_Init (used as an upper bound for endpoint allocation)
#define USB_HCD_CHANNELS_DEFAULT     16     // Host-mode channel count passed to HAL_HCD_Init (unused while Device mode is the only implemented path)
#define USB_CONFIG_VALUE_DEFAULT     1      // bConfigurationValue advertised in the Configuration Descriptor (device only ever has one configuration)

// Endpoint packet size and polling interval defaults
#define USB_EP_MAX_PACKET_INTR       8  // wMaxPacketSize used for interrupt (CDC notification) endpoints
#define USB_EP_INTERVAL_FS           10 // bInterval for Full-Speed endpoints, in milliseconds
#define USB_EP_INTERVAL_HS           6  // bInterval for High-Speed endpoints, in microframes (125us units)

// Endpoint address helpers. A USB endpoint "address" packs a direction bit
// (D7) and a physical endpoint number (D6..0) into one byte; these macros
// convert between the two representations.
#define TO_ENDPOINT_NUMBER(Endpoint) ((uint8)(Endpoint & 0x7F))         // Strip the direction bit, leaving the raw endpoint number
#define TO_OUT_ENDPOINT(EndpointNumber) ((uint8)(USB_EP0_OUT | EndpointNumber)) // Build an OUT-direction endpoint address from a raw number
#define TO_IN_ENDPOINT(EndpointNumber) ((uint8)(USB_EP0_IN | EndpointNumber))   // Build an IN-direction endpoint address from a raw number

// Global sizing constants for buffers and strings
static constexpr uint8 USBMaxStringLength = 31; // Maximum number of UTF-16 characters a string descriptor here can hold

// bDescriptorType values: identifies which kind of descriptor a given block
// of bytes is (standard descriptors defined by the USB spec, plus the two
// class-specific "functional descriptor" types used by CDC/Audio).
enum class USBDescTypes : uint8
{
	Device = 0x01,                // Standard Device Descriptor
	Configuration = 0x02,         // Standard Configuration Descriptor
	String = 0x03,                // Standard String Descriptor
	Interface = 0x04,             // Standard Interface Descriptor
	Endpoint = 0x05,              // Standard Endpoint Descriptor
	InterfaceAssociation = 0x0B,  // Interface Association Descriptor (IAD), groups related interfaces into one function
	CS_INTERFACE = 0x24,               // Class-specific *interface* descriptor (CS_INTERFACE) - used by both CDC and Audio functional descriptors
	CS_ENDPOINT = 0x25,       // Class-specific *endpoint* descriptor (CS_ENDPOINT) - used by Audio's isochronous endpoint descriptor
};

// bFunctionClass/bInterfaceClass values: identifies which USB-IF-defined
// class an interface (or IAD-grouped function) implements. This is what
// tells the host which built-in driver to bind (e.g. usbaudio.sys for Audio).
enum class USBSDeviceClasses : uint8
{
	None = 0x00, // No class specified at this level (class defined per-interface instead)
	Audio = 0x01, // Audio Interface Class
	CDC = 0x02,   // Communications Device Class (Communications/Control interface)
	Data = 0x0A,  // CDC Data Interface Class (the paired bulk data interface)
	Misc = 0xEF   // Miscellaneous Device Class - used at the Device Descriptor level when the device relies on IADs
};

// bDeviceSubClass values, only meaningful when bDeviceClass = Misc; declares
// that the device follows the "Common Class" IAD convention.
enum class USBDeviceSubClasses : uint8
{
	None = 0x00,    // No subclass specified
	Common = 0x02   // Interface Association Descriptor (IAD) convention subclass
};

// bDeviceProtocol values, only meaningful when bDeviceClass = Misc; declares
// that IADs are used to group the device's interfaces into functions.
enum class USBDeviceProtocols : uint8
{
	None = 0x00,    // No protocol specified
	IAD = 0x01,     // Device uses Interface Association Descriptors
	Vendor = 0xFF   // Vendor-specific protocol
};

// bmAttributes transfer-type bits (D1..0) of a standard Endpoint Descriptor.
// Sync-type and usage-type bits for isochronous endpoints are layered on
// top of Isochronous separately (see EndpointSyncTypes in USBAMCDefinitions.h).
enum class USBEndpointAttributes : uint8
{
	Control = 0x00,    // Control transfer type (used only by EP0)
	Isochronous = 0x01,// Isochronous transfer type (used by Audio streaming endpoints)
	Bulk = 0x02,       // Bulk transfer type (used by CDC data endpoints)
	Interrupt = 0x03   // Interrupt transfer type (used by CDC notification endpoint)
};

// bmAttributes power-source bits of the standard Configuration Descriptor.
enum class USBConfigAttributes : uint8
{
	SelfPoweredMask = 0xC0, // D7 (reserved, must be 1) + D6 (Self Powered) both set
	BusPoweredMask = 0x80   // D7 (reserved, must be 1) only; device draws power from the bus
};

BEGIN_PACK(1);
// The 8-byte packet that begins every USB control transfer, carried in the
// SETUP stage. Every class/vendor/standard request is described by this
// same structure.
struct USBDeviceSetupPacket
{
public:
	uint8 bmRequestType; // Direction (D7) + Type (D6..5) + Recipient (D4..0) - see USB_REQ_* masks above
	uint8 bRequest;       // Which request this is (meaning depends on Type: standard/class/vendor)
	uint16 wValue;         // Request-specific parameter (e.g. descriptor type/index, or a control selector)
	uint16 wIndex;         // Usually an interface or endpoint number/entity ID, depending on recipient
	uint16 wLength;        // Number of bytes to transfer in the following Data stage, or 0 for no data stage
};
END_PACK();

BEGIN_PACK(1);
// Standard USB Device Descriptor: the first thing the host reads, describing
// the device as a whole (VID/PID, USB version, overall class, string indices).
struct USBDeviceDescriptor
{
public:
	uint8 bLength;                        // Total size of this descriptor: 18 bytes
	USBDescTypes bDescriptorType;          // Always USBDescTypes::Device
	uint16 bcdUSB;                          // USB specification release number in BCD (see USB_VERSION_2_0)
	USBSDeviceClasses bDeviceClass;         // Overall device class, or None/Misc if defined per-interface
	USBDeviceSubClasses bDeviceSubClass;    // Overall device subclass (only meaningful alongside Misc)
	USBDeviceProtocols bDeviceProtocol;     // Overall device protocol (only meaningful alongside Misc)
	uint8 bMaxPacketSize0;                 // Max packet size for endpoint 0 (control), in bytes
	uint16 idVendor;                        // USB Vendor ID (VID)
	uint16 idProduct;                       // USB Product ID (PID)
	uint16 bcdDevice;                       // Device release/version number in BCD
	uint8 iManufacturer;                   // String descriptor index for the manufacturer name, or 0
	uint8 iProduct;                        // String descriptor index for the product name, or 0
	uint8 iSerialNumber;                   // String descriptor index for the serial number, or 0
	uint8 bNumConfigurations;              // Number of possible configurations (always 1 here)
};
END_PACK();

BEGIN_PACK(1);
// Standard USB String Descriptor. Index 0 is special: instead of text, its
// wData array holds a list of supported language IDs.
struct USBStringDescriptor
{
public:
	uint8 bLength;                   // Total size of this descriptor in bytes (2 + 2*character count)
	USBDescTypes bDescriptorType;    // Always USBDescTypes::String
	uint16 wData[USBMaxStringLength]; // UTF-16LE character data (or language ID list for index 0)
};
END_PACK();

BEGIN_PACK(1);
// Standard USB Configuration Descriptor: the header of the whole
// configuration block, followed immediately by every interface/endpoint
// descriptor that makes up this configuration.
struct USBConfigurationDescriptor
{
public:
	uint8 bLength;                   // Total size of this descriptor alone: 9 bytes
	USBDescTypes bDescriptorType;    // Always USBDescTypes::Configuration
	uint16 wTotalLength;              // Total size of this descriptor plus every interface/endpoint descriptor that follows it
	uint8 bNumInterfaces;             // Number of interfaces in this configuration
	uint8 bConfigurationValue;        // Value the host uses in SET_CONFIGURATION to select this configuration
	uint8 iConfiguration;             // String descriptor index naming this configuration, or 0 for none
	USBConfigAttributes bmAttributes; // Power source bits (see USBConfigAttributes)
	uint8 bMaxPower;                  // Max current draw from the bus, in units of 2mA
};
END_PACK();

BEGIN_PACK(1);
// USB Interface Association Descriptor (IAD): groups a run of consecutive
// interfaces (e.g. a CDC Control+Data pair, or an Audio Control+Streaming
// set) into a single logical function for composite device binding.
struct USBInterfaceAssociationDescriptor
{
public:
	uint8  bLength;                    // Total size of this descriptor: 8 bytes
	USBDescTypes  bDescriptorType;     // Always USBDescTypes::InterfaceAssociation
	uint8  bFirstInterface;             // Interface number of the first interface in this function
	uint8  bInterfaceCount;             // How many consecutive interfaces belong to this function
	USBSDeviceClasses  bFunctionClass;  // Class code for the function as a whole
	uint8  bFunctionSubClass;           // Subclass code for the function as a whole
	uint8  bFunctionProtocol;           // Protocol code for the function as a whole
	uint8  iFunction;                   // String descriptor index naming this function, or 0 for none
};
END_PACK();

BEGIN_PACK(1);
// Standard USB Interface Descriptor: describes one interface (and, via
// bAlternateSetting, one specific alternate configuration of it).
struct USBInterfaceDescriptor
{
public:
	uint8  bLength;                    // Total size of this descriptor: 9 bytes
	USBDescTypes  bDescriptorType;     // Always USBDescTypes::Interface
	uint8  bInterfaceNumber;            // Zero-based index of this interface within the configuration
	uint8  bAlternateSetting;           // Which alternate setting of this interface number this describes
	uint8  bNumEndpoints;               // Number of endpoints used by this alternate setting (excluding EP0)
	USBSDeviceClasses  bInterfaceClass; // Class code for this interface
	uint8  bInterfaceSubClass;          // Subclass code for this interface
	uint8  bInterfaceProtocol;          // Protocol code for this interface
	uint8  iInterface;                  // String descriptor index naming this interface, or 0 for none
};
END_PACK();

BEGIN_PACK(1);
// Standard USB Endpoint Descriptor: describes one non-control endpoint
// belonging to the interface/alt-setting it follows in the descriptor stream.
struct USBEndpointDescriptor
{
public:
	uint8  bLength;                 // Total size of this descriptor: 7 bytes
	USBDescTypes  bDescriptorType;  // Always USBDescTypes::Endpoint
	uint8  bEndpointAddress;         // Direction bit + endpoint number (see TO_IN_ENDPOINT/TO_OUT_ENDPOINT)
	uint8  bmAttributes;        // Transfer type, plus sync/usage bits for isochronous endpoints
	uint16 wMaxPacketSize;            // Maximum packet size this endpoint accepts/sends, in bytes
	uint8  bInterval;                 // Polling interval (interrupt/isochronous) - see USB_EP_INTERVAL_*
};
END_PACK();

BEGIN_PACK(1);
// Shared scratch buffer for building responses to control requests on EP0.
// The three members overlay the same memory since only one kind of
// descriptor is ever being assembled/sent at a time.
struct alignas(4) EP0Buffer
{
public:
	union
	{
		USBDeviceDescriptor deviceDesc;   // Used while responding to GET_DESCRIPTOR(Device)
		USBStringDescriptor stringDesc;   // Used while responding to GET_DESCRIPTOR(String)
		uint8 configDescs[512];            // Used while building/responding to GET_DESCRIPTOR(Configuration) - raw bytes since its contents are variable-length and class-specific
	};
};
END_PACK();

// Helper for streaming a buffer out over a control or bulk/interrupt
// endpoint in fixed-size chunks (since HAL_PCD_EP_Transmit sends at most one
// max-packet-size chunk per call). Tracks how much of the buffer is left and
// advances through it one "ChunkSize" step at a time.
class BufferTransmitHandler
{
public:
	// ChunkSize should match the endpoint's wMaxPacketSize this handler feeds.
	BufferTransmitHandler(uint16 ChunkSize)
		:m_ChunkSize(ChunkSize)
	{}

	// Clears any in-progress transfer, leaving the handler idle (HasMore() == false).
	void Reset(void)
	{
		Set<uint8>(nullptr, 0);
	}

	// Starts a new transfer of exactly sizeof(T) bytes from Buffer.
	template<typename T>
	void Set(const T* Buffer)
	{
		Set(Buffer, sizeof(T));
	}

	// Starts a new transfer of Length bytes from Buffer.
	template<typename T>
	void Set(const T* Buffer, uint16 Length)
	{
		m_BufferStart = reinterpret_cast<const uint8*>(Buffer);
		m_RemainingLength = Length;
	}

	// Advances past the chunk that was just sent, ready for GetBuffer()/GetLength()
	// to return the next chunk (or to report HasMore() == false when done).
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

	// Pointer to the start of the current (not-yet-sent) chunk.
	const uint8* GetBuffer(void) const
	{
		return m_BufferStart;
	}

	// Size in bytes of the current chunk: ChunkSize, or less for the final short chunk.
	uint16 GetLength(void) const
	{
		if (m_RemainingLength > m_ChunkSize)
			return m_ChunkSize;

		return m_RemainingLength;
	}

	// True while there is still data left to send.
	bool HasMore(void) const
	{
		return (m_RemainingLength != 0);
	}

private:
	uint16 m_ChunkSize;              // Max bytes sent per Transmit call, normally the endpoint's wMaxPacketSize
	const uint8* m_BufferStart;      // Start of the not-yet-sent remainder of the buffer
	uint16 m_RemainingLength;        // Bytes left to send, including the current chunk
};

enum class Peripherals
{
	FullSpeed = 0,
	Internal = FullSpeed,

	//It's not actually HighSpeed, but the USB peripheral is capable of running at HighSpeed, and the USB_OTG_HS peripheral is not available on the Daisy Seed.
	HighSpeed,
	External = HighSpeed,

	COUNT
};

#endif