#pragma once
#ifndef USB_AMC_DEFINITIONS_H
#define USB_AMC_DEFINITIONS_H

#include "USBDefinitions.h"

#define AUDIO_CATEGORY_IO_BOX            0x08

#define AUDIO_SUBCLASS_CONTROL              0x01
#define AUDIO_SUBCLASS_STREAMING            0x02

// AC/AS class-specific descriptor subtypes (بین UAC1/UAC2 مشترک)
#define AC_DESC_HEADER            0x01
#define AC_DESC_INPUT_TERMINAL    0x02
#define AC_DESC_OUTPUT_TERMINAL   0x03
#define AC_DESC_FEATURE_UNIT      0x06

#define AS_DESC_GENERAL            0x01
#define AS_DESC_FORMAT_TYPE        0x02
#define AS_DESC_EP_GENERAL         0x01   // class-specific ISO endpoint subtype

#define AUDIO_FORMAT_TYPE_I        0x01
#define AUDIO_FORMAT_TAG_PCM       0x0001

#define AUDIO_TERMINAL_USB_STREAMING  0x0101
#define AUDIO_TERMINAL_SPEAKER        0x0301
#define AUDIO_TERMINAL_MIC            0x0201

// class-specific control selectors
#define CS_SAM_FREQ_CONTROL       0x01  // روی Endpoint
#define CS_MUTE_CONTROL           0x01  // روی Feature Unit
#define CS_VOLUME_CONTROL         0x02  // روی Feature Unit

// UAC1 class-specific request codes (بر خلاف UAC2، GET/SET کدهای جدا دارن)
#define UAC1_SET_CUR   0x01
#define UAC1_GET_CUR   0x81

#define CLOCK_SOURCE_ID_UNUSED_IN_UAC1  // یادآوری: UAC1 اصلاً Clock Source Entity نداره

#define IT_USB_STREAMING_ID 2   // ورودی از هاست (playback path)
#define OT_SPEAKER_ID        3
#define FU_SPEAKER_ID        4
#define IT_MIC_ID            5   // ورودی از میکروفون/DSP (capture path)
#define OT_USB_STREAMING_ID  6
#define FU_MIC_ID            7

#define USB_EP_SYNC_TYPE_NONE       0x00
#define USB_EP_SYNC_TYPE_ASYNC      0x04
#define USB_EP_SYNC_TYPE_ADAPTIVE   0x08
#define USB_EP_SYNC_TYPE_SYNC       0x0C

// Audio Data Endpoint Descriptor (UAC1) bmAttributes bits
#define UAC1_EP_ATTR_SAMPLING_FREQ  0x01
#define UAC1_EP_ATTR_PITCH          0x02
#define UAC1_EP_ATTR_MAX_PACKETS    0x80

enum class ChannelOutputPosition : uint16   // UAC1: فقط 2 بایته، نه 4
{
	Unknown = 0x0000,
	FrontLeft = 0x0001,
	FrontRight = 0x0002,
	FrontLeftAndRight = FrontLeft | FrontRight,
	Stereo = FrontLeftAndRight,
};

BEGIN_PACK(1);
struct UAC1InputTerminalDescriptor
{
	uint8 bLength;                   // 12
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;        // AC_DESC_INPUT_TERMINAL
	uint8 bTerminalID;
	uint16 wTerminalType;
	uint8 bAssocTerminal;
	uint8 bNrChannels;
	uint16 wChannelConfig;
	uint8 iChannelNames;
	uint8 iTerminal;
};
END_PACK();

BEGIN_PACK(1);
struct UAC1OutputTerminalDescriptor
{
	uint8 bLength;                   // 9
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;        // AC_DESC_OUTPUT_TERMINAL
	uint8 bTerminalID;
	uint16 wTerminalType;
	uint8 bAssocTerminal;
	uint8 bSourceID;
	uint8 iTerminal;
};
END_PACK();

BEGIN_PACK(1);
struct UAC1StreamingInterfaceDescriptor  // AS General
{
	uint8 bLength;                   // 7
	USBDescType bDescriptorType;
	uint8 bDescriptorSubtype;        // AS_DESC_GENERAL
	uint8 bTerminalLink;
	uint8 bDelay;
	uint16 wFormatTag;               // AUDIO_FORMAT_TAG_PCM
};
END_PACK();

BEGIN_PACK(1);
struct UAC1IsoEndpointDescriptor    // class-specific AS Isochronous Audio Data Endpoint
{
	uint8 bLength;                   // 7
	USBDescType bDescriptorType;     // CDCEndpointFunc (0x25)
	uint8 bDescriptorSubtype;        // AS_DESC_EP_GENERAL
	uint8 bmAttributes;
	uint8 bLockDelayUnits;
	uint16 wLockDelay;
};
END_PACK();

#endif