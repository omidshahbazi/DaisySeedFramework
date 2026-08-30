#pragma once
#ifndef USB_CDC_DEFINITIONS_H
#define USB_CDC_DEFINITIONS_H

#include "USBDefinitions.h"
#include <DigitalSignalProcessing/DataTypes.h>
#include <DigitalSignalProcessing/Common.h>

// Communications Device Class (CDC) specific constants and subtypes
#define CDC_CS_INTERFACE             0x24 // Descriptor subtype class-specific interface
#define CDC_SCS_HEADER               0x00 // Header functional descriptor subtype
#define CDC_SCS_ACM                  0x02 // Abstract Control Model functional descriptor subtype
#define CDC_SCS_UNION                0x06 // Union functional descriptor subtype

// CDC class-specific control request codes
#define USB_CDC_REQ_SET_LINE_CODING           0x20 // Set line coding request
#define USB_CDC_REQ_GET_LINE_CODING           0x21 // Get line coding request
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE    0x22 // Set control line state request

// CDC version and feature capability defaults
#define USB_CDC_BCD_VERSION          0x0110 // CDC class specification version 1.10
#define USB_CDC_CAP_FEATURE          0x02   // Capabilities feature support flag

// CDC class subclass codes enumeration
enum class CDCSubClass : uint8
{
	None = 0x00,
	DLCM = 0x01,
	ACM = 0x02,
	TCM = 0x03,
	MCCM = 0x04,
	CAPI = 0x05,
	ECM = 0x06,
	ANCM = 0x07,
	WMC = 0x08,
	MDLM = 0x09,
	WHCM = 0x0A,
	DM = 0x0B,
	MDLMCC = 0x0C,
	OBEX = 0x0D,
	EEM = 0x0E,
	NCM = 0x0F,
	MBIM = 0x10
};

// CDC class protocol codes enumeration
enum class CDCProtocol : uint8
{
	None = 0x00,
	AT = 0x01,
	PCCA1 = 0x02,
	PCCA2 = 0x03,
	GSM = 0x04,
	HAYES = 0x05,
	CAPI = 0x06,
	ENET = 0x07,
	Q921 = 0x08,
	V42BIS = 0x09,
	EUROISDN = 0x0A,
	V24 = 0x0B,
	CAPICMD = 0x0C,
	HOSTBASED = 0x0D,
	PUFC = 0x0E,
	VENDOR = 0xFF
};

BEGIN_PACK(1);
// CDC Header Functional Descriptor structure
struct USBCDCHeaderFunctionalDescriptor
{
public:
	uint8 bFunctionLength;
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;
	uint16 bcdCDC;
};
END_PACK();

BEGIN_PACK(1);
// CDC Abstract Control Management (ACM) Functional Descriptor structure
struct USBCDCACMFunctionalDescriptor
{
public:
	uint8 bFunctionLength;
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;
	uint8 bmCapabilities;
};
END_PACK();

BEGIN_PACK(1);
// CDC Union Functional Descriptor structure
struct USBCDCUnionFunctionalDescriptor
{
public:
	uint8 bFunctionLength;
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;
	uint8 bMasterInterface;
	uint8 bSlaveInterface0;
};
END_PACK();

BEGIN_PACK(1);
// Line Coding configuration structure for serial communication
struct USBCDCLineCoding
{
public:
	uint32 dwDTERate;
	uint8  bCharFormat; // Stop bits
	uint8  bParityType; // Parity
	uint8  bDataBits;   // Data bits
};
END_PACK();


#endif