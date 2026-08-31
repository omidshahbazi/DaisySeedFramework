#pragma once
#ifndef USB_AMC_DEFINITIONS_H
#define USB_AMC_DEFINITIONS_H

#include "USBDefinitions.h"

#define AUDIO_CATEGORY_IO_BOX            0x08

#define AUDIO_FUNCTION_SUBCLASS_UNDEFINED   0x00
#define AUDIO_FUNCTION_PROTOCOL_AF_2_0      0x20  // UAC2 protocol code

// Audio Control interface subclass/protocol
#define AUDIO_SUBCLASS_CONTROL              0x01
#define AUDIO_SUBCLASS_STREAMING            0x02
#define AUDIO_PROTOCOL_UAC2                 0x20

// AC/AS class-specific descriptor subtypes
#define AC_DESC_HEADER            0x01
#define AC_DESC_INPUT_TERMINAL    0x02
#define AC_DESC_OUTPUT_TERMINAL   0x03
#define AC_DESC_FEATURE_UNIT      0x06
#define AC_DESC_CLOCK_SOURCE      0x0A

#define AS_DESC_GENERAL            0x01
#define AS_DESC_FORMAT_TYPE        0x02

#define AUDIO_FORMAT_TYPE_I        0x01
#define AUDIO_DATA_FORMAT_PCM      0x00000001

#define AUDIO_TERMINAL_USB_STREAMING  0x0101
#define AUDIO_TERMINAL_SPEAKER        0x0301
#define AUDIO_TERMINAL_MIC            0x0201

#define AC_FEATURE_MUTE_CONTROL    0x01
#define AC_FEATURE_VOLUME_CONTROL  0x02

// class-specific control selectors (SET_CUR/GET_CUR روی EP0)
#define CS_SAM_FREQ_CONTROL       0x01  // Clock Source
#define CS_MUTE_CONTROL           0x01  // Feature Unit
#define CS_VOLUME_CONTROL         0x02  // Feature Unit

#define CLOCK_SOURCE_ID		1
#define IT_USB_STREAMING_ID 2   // ورودی از هاست (playback path)
#define OT_SPEAKER_ID		3
#define FU_SPEAKER_ID		4
#define IT_MIC_ID			5   // ورودی از میکروفون/DSP (capture path)
#define OT_USB_STREAMING_ID	6
#define FU_MIC_ID			7

#define UAC2_REQ_CUR    0x01
#define UAC2_REQ_RANGE  0x02

#define USB_EP_SYNC_TYPE_NONE       0x00
#define USB_EP_SYNC_TYPE_ASYNC      0x04
#define USB_EP_SYNC_TYPE_ADAPTIVE   0x08
#define USB_EP_SYNC_TYPE_SYNC       0x0C

enum class ChannelOutputPosition : uint32
{
	Unknown = 0x00000000,

	FrontLeft = 0x00000001, // D0
	FrontRight = 0x00000002, // D1
	FrontCenter = 0x00000004, // D2
	LowFrequencyEffects = 0x00000008, // D3
	BackLeft = 0x00000010, // D4
	BackRight = 0x00000020, // D5
	FrontLeftOfCenter = 0x00000040, // D6
	FrontRightOfCenter = 0x00000080, // D7
	BackCenter = 0x00000100, // D8
	SideLeft = 0x00000200, // D9
	SideRight = 0x00000400, // D10
	TopCenter = 0x00000800, // D11
	TopFrontLeft = 0x00001000, // D12
	TopFrontCenter = 0x00002000, // D13
	TopFrontRight = 0x00004000, // D14
	TopBackLeft = 0x00008000, // D15
	TopBackCenter = 0x00010000, // D16
	TopBackRight = 0x00020000, // D17

	FrontLeftAndRight = FrontLeft | FrontRight,
	Stereo = FrontLeftAndRight,
};

BEGIN_PACK(1);
struct UACHeaderDescriptor
{
	uint8 bLength;
	USBDescType bDescriptorType;    // CDCFunc (0x24) به‌عنوان class-specific
	uint8 bDescriptorSubtype;       // AC_DESC_HEADER
	uint16 bcdADC;                  // 0x0200 برای UAC2
	uint8 bCategory;
	uint16 wTotalLength;
	uint8 bmControls;
};
END_PACK();

BEGIN_PACK(1);
struct UACClockSourceDescriptor
{
	uint8 bLength;
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;       // AC_DESC_CLOCK_SOURCE
	uint8 bClockID;
	uint8 bmAttributes;             // 0x01 = Internal Fixed Clock
	uint8 bmControls;
	uint8 bAssocTerminal;
	uint8 iClockSource;
};
END_PACK();

BEGIN_PACK(1);
struct UACInputTerminalDescriptor
{
	uint8 bLength;
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;       // AC_DESC_INPUT_TERMINAL
	uint8 bTerminalID;
	uint16 wTerminalType;
	uint8 bAssocTerminal;
	uint8 bCSourceID;               // به کدوم Clock Source وصله
	uint8 bNrChannels;
	uint32 bmChannelConfig;
	uint8 iChannelNames;
	uint16 bmControls;
	uint8 iTerminal;
};
END_PACK();

BEGIN_PACK(1);
struct UACOutputTerminalDescriptor
{
	uint8 bLength;
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;       // AC_DESC_OUTPUT_TERMINAL
	uint8 bTerminalID;
	uint16 wTerminalType;
	uint8 bAssocTerminal;
	uint8 bSourceID;                // ورودیش از کدوم Unit/Terminal میاد
	uint8 bCSourceID;
	uint16 bmControls;
	uint8 iTerminal;
};
END_PACK();

BEGIN_PACK(1);
struct UACFeatureUnitDescriptor
{
	uint8 bLength;
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;       // AC_DESC_FEATURE_UNIT
	uint8 bUnitID;
	uint8 bSourceID;
	uint32 bmaControls0;            // کنترل master channel (mute/volume)
	uint8 iFeature;
};
END_PACK();

BEGIN_PACK(1);
struct UACStreamingInterfaceDescriptor  // AS General
{
	uint8 bLength;
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;       // AS_DESC_GENERAL
	uint8 bTerminalLink;
	uint8 bmControls;
	uint8 bFormatType;
	uint32 bmFormats;               // AUDIO_DATA_FORMAT_PCM
	uint8 bNrChannels;
	uint32 bmChannelConfig;
	uint8 iChannelNames;
};
END_PACK();

BEGIN_PACK(1);
struct UACFormatTypeIDescriptor
{
	uint8 bLength;
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;       // AS_DESC_FORMAT_TYPE
	uint8 bFormatType;              // AUDIO_FORMAT_TYPE_I
	uint8 bSubslotSize;             // بایت به‌ازای هر سمپل (2 برای 16bit، 3 برای 24bit)
	uint8 bBitResolution;
};
END_PACK();

BEGIN_PACK(1);
struct UACIsoEndpointDescriptor  // class-specific AS Isochronous Audio Data Endpoint
{
	uint8 bLength;
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;       // 0x01
	uint8 bmAttributes;
	uint8 bmControls;
	uint8 bLockDelayUnits;
	uint16 wLockDelay;
};
END_PACK();

BEGIN_PACK(1);
struct UACFreqSubRange
{
	uint32 dMIN;
	uint32 dMAX;
	uint32 dRES;
};
END_PACK();

#endif