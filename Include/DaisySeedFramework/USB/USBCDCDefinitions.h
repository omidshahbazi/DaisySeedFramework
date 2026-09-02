#pragma once
#ifndef USB_CDC_DEFINITIONS_H
#define USB_CDC_DEFINITIONS_H

#include "USBDefinitions.h"

// Communications Device Class (CDC) specific constants and subtypes
#define CDC_CS_INTERFACE             0x24 // Class-specific interface descriptor type (same value as USBDescType::CDCFunc)
#define CDC_SCS_HEADER               0x00 // bDescriptorSubtype: Header Functional Descriptor
#define CDC_SCS_ACM                  0x02 // bDescriptorSubtype: Abstract Control Management Functional Descriptor
#define CDC_SCS_UNION                0x06 // bDescriptorSubtype: Union Functional Descriptor

// CDC class-specific control request codes (bRequest field of the setup packet)
#define USB_CDC_REQ_SET_LINE_CODING           0x20 // Host -> device: set baud rate/stop bits/parity/data bits
#define USB_CDC_REQ_GET_LINE_CODING           0x21 // Device -> host: report current baud rate/stop bits/parity/data bits
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE    0x22 // Host -> device: set DTR/RTS state (used to detect terminal open/close)

// CDC version and feature capability defaults
#define USB_CDC_BCD_VERSION          0x0110 // bcdCDC value: CDC specification version 1.10, placed in the Header Functional Descriptor
#define USB_CDC_CAP_FEATURE          0x02   // bmCapabilities bit: device supports Set_Line_Coding/Set_Control_Line_State/Get_Line_Coding + Serial_State notification

// bInterfaceSubClass values for the CDC Communications interface, defining
// which specific CDC model the device implements (ACM = the common "virtual
// COM port" model used here; the rest are listed for completeness).
enum class CDCSubClasses : uint8
{
	None = 0x00,  // No subclass specified
	DLCM = 0x01,  // Direct Line Control Model
	ACM = 0x02,   // Abstract Control Model (the classic USB-serial/COM-port model)
	TCM = 0x03,   // Telephone Control Model
	MCCM = 0x04,  // Multi-Channel Control Model
	CAPI = 0x05,  // CAPI Control Model
	ECM = 0x06,   // Ethernet Networking Control Model
	ANCM = 0x07,  // ATM Networking Control Model
	WMC = 0x08,   // Wireless Handset Control Model
	MDLM = 0x09,  // Mobile Direct Line Model
	WHCM = 0x0A,  // Wireless Handset Control Model (device management)
	DM = 0x0B,    // Device Management Model
	MDLMCC = 0x0C, // Mobile Direct Line Model - Control/Command
	OBEX = 0x0D,  // OBEX (Object Exchange) Model
	EEM = 0x0E,   // Ethernet Emulation Model
	NCM = 0x0F,   // Network Control Model
	MBIM = 0x10   // Mobile Broadband Interface Model
};

// bInterfaceProtocol values for the CDC Communications interface, describing
// the command-set/protocol spoken over the control channel (AT = the classic
// "AT command" style used by ACM devices, as implemented here).
enum class CDCProtocols : uint8
{
	None = 0x00,      // No protocol specified
	AT = 0x01,         // AT commands (ITU-T V.250 etc.) - what a virtual COM port uses
	PCCA1 = 0x02,      // PCCA-101 AT commands
	PCCA2 = 0x03,      // PCCA-101 AT commands with wireless extensions (annex O)
	GSM = 0x04,        // GSM 07.07 AT commands
	HAYES = 0x05,      // 3GPP 27.007 AT commands
	CAPI = 0x06,       // C-API commands
	ENET = 0x07,       // Ethernet Emulation Model
	Q921 = 0x08,       // ISDN Q.921 management protocol
	V42BIS = 0x09,     // Data compression procedures
	EUROISDN = 0x0A,   // Euro-ISDN protocol control
	V24 = 0x0B,        // V.24 rate adaptation to ISDN
	CAPICMD = 0x0C,    // CAPI commands, host driver-defined
	HOSTBASED = 0x0D,  // Host-based driver protocol
	PUFC = 0x0E,       // Protocol unit functional descriptors on Communications class interface
	VENDOR = 0xFF      // Vendor-specific protocol
};

BEGIN_PACK(1);
// CDC Header Functional Descriptor: the first class-specific descriptor in a
// CDC Communications interface, declaring which CDC spec revision it follows.
struct USBCDCHeaderFunctionalDescriptor
{
public:
	uint8 bFunctionLength;         // Total size of this descriptor: 5 bytes
	USBDescTypes bDescriptorType;   // Always CDCFunc (0x24), the class-specific interface descriptor type
	uint8 bDescriptorSubtype;      // CDC_SCS_HEADER
	uint16 bcdCDC;                  // CDC specification release number in BCD (see USB_CDC_BCD_VERSION)
};
END_PACK();

BEGIN_PACK(1);
// CDC Abstract Control Management (ACM) Functional Descriptor: advertises
// which ACM-specific requests/notifications this interface supports.
struct USBCDCACMFunctionalDescriptor
{
public:
	uint8 bFunctionLength;         // Total size of this descriptor: 4 bytes
	USBDescTypes bDescriptorType;   // Always CDCFunc (0x24)
	uint8 bDescriptorSubtype;      // CDC_SCS_ACM
	uint8 bmCapabilities;           // Bitmap of supported ACM features (see USB_CDC_CAP_FEATURE)
};
END_PACK();

BEGIN_PACK(1);
// CDC Union Functional Descriptor: links the Communications (control)
// interface to the Data interface it manages, so the host knows they belong
// to the same logical function.
struct USBCDCUnionFunctionalDescriptor
{
public:
	uint8 bFunctionLength;         // Total size of this descriptor: 5 bytes
	USBDescTypes bDescriptorType;   // Always CDCFunc (0x24)
	uint8 bDescriptorSubtype;      // CDC_SCS_UNION
	uint8 bMasterInterface;         // Interface number of the Communications (control) interface
	uint8 bSlaveInterface0;         // Interface number of the associated Data interface
};
END_PACK();

BEGIN_PACK(1);
// Line Coding structure exchanged via SET_LINE_CODING/GET_LINE_CODING,
// describing the serial port settings the host application has configured
// (this is metadata only - the pedal doesn't need to actually run a UART
// at this rate, since the underlying transport is USB, not a real serial line).
struct USBCDCLineCoding
{
public:
	uint32 dwDTERate;    // Baud rate in bits per second, e.g. 115200
	uint8  bCharFormat;   // Stop bits: 0=1 stop bit, 1=1.5 stop bits, 2=2 stop bits
	uint8  bParityType;   // Parity: 0=None, 1=Odd, 2=Even, 3=Mark, 4=Space
	uint8  bDataBits;     // Data bits per character: 5, 6, 7, 8, or 16
};
END_PACK();


#endif