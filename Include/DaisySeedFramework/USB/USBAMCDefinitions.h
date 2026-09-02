#pragma once
#ifndef USB_AMC_DEFINITIONS_H
#define USB_AMC_DEFINITIONS_H

#include "USBDefinitions.h"

// Subclass codes used on the standard Interface Descriptor for a CDC
// interface. CTRL marks the Communications/Control interface (the one
// carrying line-coding requests); STRM marks the Data/Streaming interface
// (the one carrying the raw bulk IN/OUT traffic).
enum class AMCSubClasses : uint8
{
	CTRL = 0x01, // Communications Class Interface (control/notification)
	STRM = 0x02, // Data Interface (bulk data transfer)
};

// bDescriptorSubtype values for class-specific descriptors that live inside
// the Audio Control (AC) interface. These identify which kind of
// class-specific descriptor follows the common header/type/subtype bytes.
#define AC_DESC_HEADER            0x01 // Class-specific AC Interface Header Descriptor
#define AC_DESC_INPUT_TERMINAL    0x02 // Input Terminal Descriptor (where audio enters the topology)
#define AC_DESC_OUTPUT_TERMINAL   0x03 // Output Terminal Descriptor (where audio leaves the topology)
#define AC_DESC_FEATURE_UNIT      0x06 // Feature Unit Descriptor (mute/volume control point)

// bDescriptorSubtype values for class-specific descriptors that live inside
// an Audio Streaming (AS) interface.
#define AS_DESC_GENERAL            0x01 // Class-specific AS Interface Descriptor (general streaming info)
#define AS_DESC_FORMAT_TYPE        0x02 // Format Type Descriptor (sample rate/bit depth/channels)
#define AS_DESC_EP_GENERAL         0x01 // Class-specific isochronous Audio Data Endpoint Descriptor subtype

// bFormatType / wFormatTag values used in the Format Type / AS General
// descriptors to say the stream carries plain PCM samples (as opposed to
// e.g. AC-3 or MPEG).
#define AUDIO_FORMAT_TYPE_I        0x01   // Format Type I: uncompressed PCM-style audio
#define AUDIO_FORMAT_TAG_PCM       0x0001 // wFormatTag value meaning "PCM"

// wTerminalType values placed on Input/Output Terminal Descriptors. They
// tell the host what kind of real-world thing each terminal represents,
// which is what makes Windows label the endpoint "Speakers" or "Microphone"
// instead of a generic name.
enum class TerminalTypes : uint16
{
	USBStreaming = 0x0101, // The "terminal" on the USB side of the topology (where samples enter/leave over USB)
	Microphone = 0x0201,   // A microphone-type input terminal (capture path)
	Speaker = 0x0301,      // A speaker-type output terminal (playback path)
};

// Control Selector values placed in the high byte of wValue on class-specific
// control requests, identifying *which* control on the entity is being
// addressed. Note CS_MUTE_CONTROL and CS_SAM_FREQ_CONTROL share the same
// numeric value (0x01) because they apply to different entity types
// (Feature Unit vs. Endpoint) and are never ambiguous in context.
#define CS_SAM_FREQ_CONTROL       0x01  // Sampling Frequency Control, sent to an Endpoint (recipient = ENDPOINT)
#define CS_MUTE_CONTROL           0x01  // Mute Control, sent to a Feature Unit (recipient = INTERFACE)
#define CS_VOLUME_CONTROL         0x02  // Volume Control, sent to a Feature Unit (recipient = INTERFACE)

// UAC1 class-specific request codes (bRequest field of the setup packet).
// Unlike UAC2, which reuses the same SET_CUR/GET_CUR values for every
// control, UAC1 encodes direction into the request code itself.
#define UAC1_SET_CUR   0x01 // Host -> device: set the current value of a control
#define UAC1_GET_CUR   0x81 // Device -> host: report the current value of a control

// Fixed Entity/Terminal/Unit ID numbers used inside a single AC interface's
// topology graph. These IDs only need to be unique *within* one AC
// interface (each AudioControl interface has its own ID namespace), so the
// same constants are safely reused across multiple AMC instances.
#define IT_USB_STREAMING_ID 2   // Input Terminal: audio coming from the host (playback path)
#define OT_SPEAKER_ID        3  // Output Terminal: audio going out to the speaker (playback path)
#define FU_SPEAKER_ID        4  // Feature Unit: mute/volume control on the playback path
#define IT_MIC_ID            5  // Input Terminal: audio coming from the microphone/DSP (capture path)
#define OT_USB_STREAMING_ID  6  // Output Terminal: audio going out to the host (capture path)
#define FU_MIC_ID            7  // Feature Unit: mute/volume control on the capture path

// Synchronization type, encoded into bits D3..D2 of an isochronous
// endpoint's bmAttributes byte. This tells the host how the device's
// sample clock relates to the USB bus clock.
enum class EndpointSyncTypes : uint16
{
	None = 0x00,        // No synchronization information provided
	Async = 0x04,        // Device runs its own clock; packet size may vary to absorb drift
	Adaptive = 0x08,     // Device adapts its clock to match the host's feedback
	Synchronous = 0x0C   // Device is slaved directly to the host's SOF timing
};

// Bits within the class-specific Audio Data Endpoint Descriptor's
// bmAttributes byte (UAC1). These advertise which extra endpoint-level
// controls the host may query/set.
#define UAC1_EP_ATTR_SAMPLING_FREQ  0x01 // D0: endpoint supports Sampling Frequency Control
#define UAC1_EP_ATTR_PITCH          0x02 // D1: endpoint supports Pitch Control
#define UAC1_EP_ATTR_MAX_PACKETS    0x80 // D7: endpoint only ever sends max-size packets

// wChannelConfig bitmap values (UAC1 uses a 16-bit field, unlike UAC2's
// 32-bit one). Each bit marks a fixed speaker position; Windows uses this
// to label channels "Left"/"Right" instead of "Channel 1"/"Channel 2".
// Unknown (0x0000) is a valid, common choice when the channels don't map
// to a standard spatial position (e.g. a guitar pedal's dry/wet outputs).
enum class ChannelOutputPositions : uint16
{
	Unknown = 0x0000,                          // No fixed spatial position declared
	FrontLeft = 0x0001,                        // Front-left speaker position
	FrontRight = 0x0002,                       // Front-right speaker position
	FrontLeftAndRight = FrontLeft | FrontRight, // Both front channels combined
	Stereo = FrontLeftAndRight,                 // Alias for the common stereo pair
};

BEGIN_PACK(1);
// UAC1 Input Terminal Descriptor: declares where audio enters the AC
// topology (either "from the host" for playback, or "from a mic/DSP" for
// capture) and how many channels it carries.
struct UAC1InputTerminalDescriptor
{
	uint8 bLength;                   // Total size of this descriptor: 12 bytes
	USBDescTypes bDescriptorType;     // Always CDCFunc (0x24), the class-specific interface descriptor type
	uint8 bDescriptorSubtype;        // AC_DESC_INPUT_TERMINAL
	uint8 bTerminalID;                // Unique ID of this terminal within the AC interface (see IT_*_ID above)
	uint16 wTerminalType;              // What kind of terminal this is (see TerminalTypes)
	uint8 bAssocTerminal;              // ID of an associated output terminal, or 0 if none
	uint8 bNrChannels;                 // Number of logical audio channels this terminal carries
	uint16 wChannelConfig;             // Spatial channel layout bitmap (see ChannelOutputPositions)
	uint8 iChannelNames;               // String descriptor index naming the channels, or 0 for none
	uint8 iTerminal;                   // String descriptor index naming this terminal, or 0 for none
};
END_PACK();

BEGIN_PACK(1);
// UAC1 Output Terminal Descriptor: declares where audio leaves the AC
// topology (either "to the speaker" for playback, or "to the host" for
// capture) and which entity feeds it.
struct UAC1OutputTerminalDescriptor
{
	uint8 bLength;                   // Total size of this descriptor: 9 bytes
	USBDescTypes bDescriptorType;     // Always CDCFunc (0x24)
	uint8 bDescriptorSubtype;        // AC_DESC_OUTPUT_TERMINAL
	uint8 bTerminalID;                // Unique ID of this terminal within the AC interface (see OT_*_ID above)
	uint16 wTerminalType;              // What kind of terminal this is (see TerminalTypes)
	uint8 bAssocTerminal;              // ID of an associated input terminal, or 0 if none
	uint8 bSourceID;                   // ID of the entity (usually a Feature Unit) that feeds this terminal
	uint8 iTerminal;                   // String descriptor index naming this terminal, or 0 for none
};
END_PACK();

BEGIN_PACK(1);
// UAC1 "AS General" Descriptor: the first class-specific descriptor inside
// an Audio Streaming interface, linking it back to a terminal in the AC
// interface and declaring the sample format tag used on the wire.
struct UAC1StreamingInterfaceDescriptor
{
	uint8 bLength;                   // Total size of this descriptor: 7 bytes
	USBDescTypes bDescriptorType;     // Always CDCFunc (0x24)
	uint8 bDescriptorSubtype;        // AS_DESC_GENERAL
	uint8 bTerminalLink;              // ID of the AC terminal this stream connects to
	uint8 bDelay;                     // Interface delay in number of frames, usually 0
	uint16 wFormatTag;                 // Sample data format on the wire (see AUDIO_FORMAT_TAG_PCM)
};
END_PACK();

BEGIN_PACK(1);
// UAC1 class-specific isochronous Audio Data Endpoint Descriptor: follows
// the standard Endpoint Descriptor and advertises extra endpoint-level
// controls/behaviour that don't fit in the standard descriptor.
struct UAC1IsoEndpointDescriptor
{
	uint8 bLength;                   // Total size of this descriptor: 7 bytes
	USBDescTypes bDescriptorType;     // CDCEndpointFunc (0x25), the class-specific *endpoint* descriptor type
	uint8 bDescriptorSubtype;        // AS_DESC_EP_GENERAL
	uint8 bmAttributes;               // Extra endpoint controls bitmap (see UAC1_EP_ATTR_* above)
	uint8 bLockDelayUnits;            // Units used to express wLockDelay: 0=undefined, 1=ms, 2=decoded PCM samples
	uint16 wLockDelay;                  // Time it takes this endpoint to lock to a new clock, in bLockDelayUnits
};
END_PACK();

#endif