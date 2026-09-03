#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBAMCInterface.h"
#include "DaisySeedFramework/USB/DaisyUSB.h"
#include <DigitalSignalProcessing/Memory.h>

DaisyUSBAMCInterface::DaisyUSBAMCInterface(DaisyUSB* USB, const Configs& Configs, const AMCClassConfig& Class)
	: DaisyUSBInterfaceCommon(USB, Configs),
	m_Class(Class),
	m_OutAltSetting(0),
	m_InAltSetting(0),
	m_OutInterfaceIndex(0),
	m_InInterfaceIndex(0),
	m_TransmitBuffer(nullptr),

	m_CurrentOutSampleRate(Class.SupportedSampleRates[Class.DefaultSampleRateIndex]),
	m_CurrentInSampleRate(Class.SupportedSampleRates[Class.DefaultSampleRateIndex]),
	m_CurrentOutBitDepth(Class.SupportedBitDepths[Class.DefaultBitDepthIndex]),
	m_CurrentInBitDepth(Class.SupportedBitDepths[Class.DefaultBitDepthIndex]),
	m_CurrentIsOutMuted(false),
	m_CurrentIsInMuted(false)
{
	m_TransmitBuffer = Memory::Allocate<uint8>(Configs.TransmitPacketSize, true);

	CalculateStreamingInterfaceIndices(GetConfigs(), Class, m_OutInterfaceIndex, m_InInterfaceIndex);
}

bool DaisyUSBAMCInterface::OnSetupStage(const USBDeviceSetupPacket* Setup)
{
	uint8 recipient = Setup->bmRequestType & USB_REQ_RECIPIENT_MASK;
	uint8 controlSelector = (uint8)(Setup->wValue >> 8);

	// ========================================================================
	// 1. Sampling Frequency Control (Target: ENDPOINT)
	// ========================================================================
	if (recipient == USB_REQ_RECIPIENT_ENDPOINT)
	{
		uint8 epAddress = (uint8)(Setup->wIndex & 0xFF);
		const Configs& configs = GetConfigs();

		if (controlSelector == CS_SAM_FREQ_CONTROL)
		{
			if (Setup->bRequest == UAC1_GET_CUR)
			{
				uint24 val;
				if (epAddress == configs.EndpointOut)
					val = m_CurrentOutSampleRate;
				else if (epAddress == configs.EndpointIn)
					val = m_CurrentInSampleRate;

				GetUSB()->DeviceTransmit(val.bytes, sizeof(val.bytes));

				GetUSB()->DeviceReceiveAck();
				return true;
			}
			else if (Setup->bRequest == UAC1_SET_CUR)
			{
				uint24 val;
				GetUSB()->DeviceReceive(val.bytes, sizeof(val.bytes));

				if (!IsSampleRateSupported(val))
					return false;

				if (epAddress == configs.EndpointOut)
				{
					m_CurrentOutSampleRate = val;

					m_ControlChangedCallback(ControlTypes::OutSampleRate);
				}
				else if (epAddress == configs.EndpointIn)
				{
					m_CurrentInSampleRate = val;

					m_ControlChangedCallback(ControlTypes::InSampleRate);
				}

				GetUSB()->DeviceTransmitAck();
				return true;
			}
		}
	}
	// ========================================================================
	// 2. Feature Unit Controls: Mute & Volume (Target: INTERFACE)
	// ========================================================================
	else if (recipient == USB_REQ_RECIPIENT_INTERFACE)
	{
		uint8 entityID = (uint8)(Setup->wIndex >> 8);
		if (entityID != FU_OUTPUT_ID && entityID != FU_INPUT_ID)
			return false;

		bool isSpeaker = (entityID == FU_OUTPUT_ID);

		if (controlSelector == CS_MUTE_CONTROL && m_Class.EnableHardwareMute)
		{
			if (Setup->bRequest == UAC1_SET_CUR)
			{
				uint8 muteVal = 0;
				GetUSB()->DeviceReceive(&muteVal, 1);

				if (isSpeaker)
				{
					m_CurrentIsOutMuted = (muteVal != 0);

					m_ControlChangedCallback(ControlTypes::OutMute);
				}
				else
				{
					m_CurrentIsInMuted = (muteVal != 0);

					m_ControlChangedCallback(ControlTypes::InMute);
				}

				GetUSB()->DeviceTransmitAck();
				return true;
			}
			else if (Setup->bRequest == UAC1_GET_CUR)
			{
				uint8 muteVal;
				if (isSpeaker)
					muteVal = m_CurrentIsOutMuted;
				else
					muteVal = m_CurrentIsInMuted;

				GetUSB()->DeviceTransmit(&muteVal, 1);

				GetUSB()->DeviceReceiveAck();
				return true;
			}
		}
		else if (controlSelector == CS_VOLUME_CONTROL && m_Class.EnableHardwareVolumeControl)
		{
			const uint16 VOLUME_STEPS = 256; // 1/256 dB steps

			switch (Setup->bRequest)
			{
			case UAC1_SET_CUR:
			{
				int16 volumeDbRaw = 0;
				GetUSB()->DeviceReceive(reinterpret_cast<uint8*>(&volumeDbRaw), sizeof(int16));

				if (isSpeaker)
				{
					m_CurrentOutVolume = dBGain((float)volumeDbRaw / VOLUME_STEPS);

					m_ControlChangedCallback(ControlTypes::OutVolume);
				}
				else
				{
					m_CurrentInVolume = dBGain((float)volumeDbRaw / VOLUME_STEPS);

					m_ControlChangedCallback(ControlTypes::InVolume);
				}

				GetUSB()->DeviceTransmitAck();
				return true;
			}
			case UAC1_GET_CUR:
			{
				int16 volumeDbRaw;
				if (isSpeaker)
					volumeDbRaw = m_CurrentOutVolume * VOLUME_STEPS;
				else
					volumeDbRaw = m_CurrentInVolume * VOLUME_STEPS;

				GetUSB()->DeviceTransmit(reinterpret_cast<uint8*>(&volumeDbRaw), sizeof(int16));

				GetUSB()->DeviceReceiveAck();
				return true;
			}
			case UAC1_GET_MIN:
			{
				int16 minVol = m_Class.MinimumVolume * VOLUME_STEPS;
				GetUSB()->DeviceTransmit(reinterpret_cast<uint8*>(&minVol), sizeof(int16));

				GetUSB()->DeviceTransmitAck();
				return true;
			}
			case UAC1_GET_MAX:
			{
				int16 maxVol = m_Class.MaximumVolume * VOLUME_STEPS;
				GetUSB()->DeviceTransmit(reinterpret_cast<uint8*>(&maxVol), sizeof(int16));

				GetUSB()->DeviceTransmitAck();
				return true;
			}
			case UAC1_GET_RES:
			{
				int16 resVol = m_Class.VolumeResolution * VOLUME_STEPS;
				GetUSB()->DeviceTransmit(reinterpret_cast<uint8*>(&resVol), sizeof(int16));

				GetUSB()->DeviceTransmitAck();
				return true;
			}
			}
		}
	}

	return false;
}

void DaisyUSBAMCInterface::OnSetupCompleted(void)
{
	DaisyUSB* usb = GetUSB();
	const Configs& configs = GetConfigs();

	if (TO_ENDPOINT_NUMBER(configs.EndpointIn) != 0)
		usb->AllocateTransmitBuffer(configs.EndpointIn, configs.TransmitPacketSize);
}

void DaisyUSBAMCInterface::OnDataInStage(void)
{
	TransmitBuffer();
}

void DaisyUSBAMCInterface::OnDataOutStage(void)
{
	const Configs& configs = GetConfigs();

	uint16 len = EndpointReceiveCount();

	if (len > 0)
	{
		uint8 rxBuffer[192]; // ظرفیت کافی برای 48kHz 16-bit Stereo (96 بایت در هر ms)

		EndpointReceive(rxBuffer, len);

		m_RxAudioFIFO.Push(rxBuffer, len);
	}
}

bool DaisyUSBAMCInterface::OnSetInterface(uint8 InterfaceIndex, uint8 AlternateSetting)
{
	if (AlternateSetting > 1)
		return false;

	const Configs& configs = GetConfigs();
	const uint8 type = (uint8)USBEndpointAttributes::Isochronous | (uint8)EndpointSyncTypes::Async;

	if (InterfaceIndex == m_OutInterfaceIndex)
	{
		if (m_OutAltSetting == AlternateSetting)
			return true;

		if (AlternateSetting == 1)
			GetUSB()->OpenEndpoint(configs.EndpointOut, configs.ReceivePacketSize, type);
		else
			GetUSB()->CloseEndpoint(configs.EndpointOut);

		m_RxAudioFIFO.Clear();

		m_OutAltSetting = AlternateSetting;
	}
	else if (InterfaceIndex == m_InInterfaceIndex)
	{
		if (m_InAltSetting == AlternateSetting)
			return true;

		m_TransmitFIFO.Clear();

		if (AlternateSetting == 0)
			GetUSB()->CloseEndpoint(configs.EndpointIn);
		else
		{
			GetUSB()->OpenEndpoint(configs.EndpointIn, configs.TransmitPacketSize, type);

			TransmitBuffer();
		}

		m_InAltSetting = AlternateSetting;
	}

	return true;
}

uint8 DaisyUSBAMCInterface::GetCurrentAltSetting(uint8 InterfaceIndex) const
{
	if (InterfaceIndex == m_OutInterfaceIndex)
		return m_OutAltSetting;

	if (InterfaceIndex == m_InInterfaceIndex)
		return m_InAltSetting;

	return 0;
}

void DaisyUSBAMCInterface::BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, const USBClassNode& Class)
{
	const AMCClassConfig& Config = Class.AMC;

	uint8* buffer = EP0Buffer.configDescs;

	bool hasOutput = (Config.OutputChannelCount > 0);
	bool hasInput = (Config.InputChannelCount > 0);

	USBInterfaceAssociationDescriptor* iad = reinterpret_cast<USBInterfaceAssociationDescriptor*>(buffer + BufferOffset);
	iad->bLength = sizeof(USBInterfaceAssociationDescriptor);
	iad->bDescriptorType = USBDescTypes::InterfaceAssociation;
	iad->bFirstInterface = InterfaceIndex;
	iad->bInterfaceCount = CalculateRequiredInterfaceCount(Config);
	iad->bFunctionClass = USBSDeviceClasses::Audio;
	iad->bFunctionSubClass = (uint8)AMCSubClasses::CTRL;
	iad->bFunctionProtocol = 0x00; // UAC1: no protocol code
	iad->iFunction = 0;
	BufferOffset += sizeof(USBInterfaceAssociationDescriptor);

	// ---------- AC Interface (standard) ----------
	USBInterfaceDescriptor* acIf = reinterpret_cast<USBInterfaceDescriptor*>(buffer + BufferOffset);
	acIf->bLength = sizeof(USBInterfaceDescriptor);
	acIf->bDescriptorType = USBDescTypes::Interface;
	acIf->bInterfaceNumber = InterfaceIndex;
	acIf->bAlternateSetting = 0;
	acIf->bNumEndpoints = 0;
	acIf->bInterfaceClass = USBSDeviceClasses::Audio;
	acIf->bInterfaceSubClass = (uint8)AMCSubClasses::CTRL;
	acIf->bInterfaceProtocol = 0x00;
	acIf->iInterface = 0;
	BufferOffset += sizeof(USBInterfaceDescriptor);

	uint16 acDescStart = BufferOffset;

	uint8 asOutInterface, asInInterface;
	CalculateStreamingInterfaceIndices(GetConfigs(), Config, asOutInterface, asInInterface);

	uint8 asInterfaceNumbers[2];
	uint8 asInterfaceCount = 0;
	if (hasOutput)
		asInterfaceNumbers[asInterfaceCount++] = asOutInterface;
	if (hasInput)
		asInterfaceNumbers[asInterfaceCount++] = asInInterface;

	uint8 headerLength = 8 + asInterfaceCount;
	{
		uint8* h = buffer + BufferOffset;
		h[0] = headerLength;
		h[1] = (uint8)USBDescTypes::CS_INTERFACE;
		h[2] = AC_DESC_HEADER;
		h[3] = 0x00;
		h[4] = 0x01;       // bcdADC = 0x0100
		// h[5],h[6] = wTotalLength, filled in below once known
		h[7] = asInterfaceCount;         // bInCollection
		for (uint8 i = 0; i < asInterfaceCount; ++i)
			h[8 + i] = asInterfaceNumbers[i];

		BufferOffset += headerLength;
	}
	uint8* headerBytes = buffer + acDescStart; // kept so we can fill wTotalLength at the end

	// ---------- Playback chain (host -> hardware) ----------
	if (hasOutput)
	{
		UAC1InputTerminalDescriptor* it = reinterpret_cast<UAC1InputTerminalDescriptor*>(buffer + BufferOffset);
		it->bLength = sizeof(UAC1InputTerminalDescriptor);
		it->bDescriptorType = USBDescTypes::CS_INTERFACE;
		it->bDescriptorSubtype = AC_DESC_INPUT_TERMINAL;
		it->bTerminalID = IT_USB_STREAMING_ID;
		it->wTerminalType = (uint16)TerminalTypes::USBStreaming;
		it->bAssocTerminal = 0;
		it->bNrChannels = Config.OutputChannelCount;
		it->wChannelConfig = (uint16)(Config.OutputChannelCount == 2 ? ChannelOutputPositions::Stereo : ChannelOutputPositions::Unknown);
		it->iChannelNames = 0;
		it->iTerminal = 0;
		BufferOffset += sizeof(UAC1InputTerminalDescriptor);

		// Feature Unit (variable-length, bControlSize=1)
		{
			uint8 channelCount = Config.OutputChannelCount;
			uint8 fuLength = 7 + (channelCount + 1); // bControlSize=1 byte per channel + master

			uint8* fuStart = buffer + BufferOffset;
			fuStart[0] = fuLength;
			fuStart[1] = (uint8)USBDescTypes::CS_INTERFACE;
			fuStart[2] = AC_DESC_FEATURE_UNIT;
			fuStart[3] = FU_OUTPUT_ID;
			fuStart[4] = IT_USB_STREAMING_ID;
			fuStart[5] = 1; // bControlSize

			uint8 masterControls = 0;
			if (Config.EnableHardwareMute)   masterControls |= 0x01;
			if (Config.EnableHardwareVolumeControl) masterControls |= 0x02;

			fuStart[6] = masterControls; // master channel
			for (uint8 ch = 1; ch <= channelCount; ++ch)
				fuStart[6 + ch] = 0; // no independent per-channel control

			fuStart[6 + channelCount + 1] = 0; // iFeature

			BufferOffset += fuLength;
		}

		UAC1OutputTerminalDescriptor* ot = reinterpret_cast<UAC1OutputTerminalDescriptor*>(buffer + BufferOffset);
		ot->bLength = sizeof(UAC1OutputTerminalDescriptor);
		ot->bDescriptorType = USBDescTypes::CS_INTERFACE;
		ot->bDescriptorSubtype = AC_DESC_OUTPUT_TERMINAL;
		ot->bTerminalID = OT_OUTPUT_ID;
		ot->wTerminalType = (uint16)TerminalTypes::LineOut;
		ot->bAssocTerminal = 0;
		ot->bSourceID = FU_OUTPUT_ID;
		ot->iTerminal = 0;
		BufferOffset += sizeof(UAC1OutputTerminalDescriptor);
	}

	// ---------- Capture chain (hardware -> host) ----------
	if (hasInput)
	{
		UAC1InputTerminalDescriptor* it = reinterpret_cast<UAC1InputTerminalDescriptor*>(buffer + BufferOffset);
		it->bLength = sizeof(UAC1InputTerminalDescriptor);
		it->bDescriptorType = USBDescTypes::CS_INTERFACE;
		it->bDescriptorSubtype = AC_DESC_INPUT_TERMINAL;
		it->bTerminalID = IT_INPUT_ID;
		it->wTerminalType = (uint16)TerminalTypes::LineIn;
		it->bAssocTerminal = 0;
		it->bNrChannels = Config.InputChannelCount;
		it->wChannelConfig = (uint16)(Config.InputChannelCount == 2 ? ChannelOutputPositions::Stereo : ChannelOutputPositions::Unknown);
		it->iChannelNames = 0;
		it->iTerminal = 0;
		BufferOffset += sizeof(UAC1InputTerminalDescriptor);

		{
			uint8 channelCount = Config.InputChannelCount;
			uint8 fuLength = 7 + (channelCount + 1);

			uint8* fuStart = buffer + BufferOffset;
			fuStart[0] = fuLength;
			fuStart[1] = (uint8)USBDescTypes::CS_INTERFACE;
			fuStart[2] = AC_DESC_FEATURE_UNIT;
			fuStart[3] = FU_INPUT_ID;
			fuStart[4] = IT_INPUT_ID;
			fuStart[5] = 1; // bControlSize

			uint8 masterControls = 0;
			if (Config.EnableHardwareMute)   masterControls |= 0x01;
			if (Config.EnableHardwareVolumeControl) masterControls |= 0x02;

			fuStart[6] = masterControls;
			for (uint8 ch = 1; ch <= channelCount; ++ch)
				fuStart[6 + ch] = 0;

			fuStart[6 + channelCount + 1] = 0; // iFeature

			BufferOffset += fuLength;
		}

		UAC1OutputTerminalDescriptor* ot = reinterpret_cast<UAC1OutputTerminalDescriptor*>(buffer + BufferOffset);
		ot->bLength = sizeof(UAC1OutputTerminalDescriptor);
		ot->bDescriptorType = USBDescTypes::CS_INTERFACE;
		ot->bDescriptorSubtype = AC_DESC_OUTPUT_TERMINAL;
		ot->bTerminalID = OT_USB_STREAMING_ID;
		ot->wTerminalType = (uint16)TerminalTypes::USBStreaming;
		ot->bAssocTerminal = 0;
		ot->bSourceID = FU_INPUT_ID;
		ot->iTerminal = 0;
		BufferOffset += sizeof(UAC1OutputTerminalDescriptor);
	}

	// Now that the whole AC block has been built, fill in wTotalLength
	uint16 totalLen = BufferOffset - acDescStart;
	headerBytes[5] = (uint8)(totalLen & 0xFF);
	headerBytes[6] = (uint8)((totalLen >> 8) & 0xFF);

	// ---------- Output AS Interface ----------
	if (hasOutput)
		BuildStreamingInterface(EP0Buffer, BufferOffset, asOutInterface, Config.OutputChannelCount, GetConfigs().EndpointOut, IT_USB_STREAMING_ID, Config, true);

	// ---------- Input AS Interface ----------
	if (hasInput)
		BuildStreamingInterface(EP0Buffer, BufferOffset, asInInterface, Config.InputChannelCount, GetConfigs().EndpointIn, OT_USB_STREAMING_ID, Config, false);
}

void DaisyUSBAMCInterface::TransmitBuffer(void)
{
	const Configs& configs = GetConfigs();

	uint16 bytesRead = m_TransmitFIFO.Pop(m_TransmitBuffer, configs.TransmitPacketSize);

	if (bytesRead < configs.TransmitPacketSize)
		Memory::Set(m_TransmitBuffer + bytesRead, 0, configs.TransmitPacketSize - bytesRead);

	EndpointTransmit(m_TransmitBuffer, configs.TransmitPacketSize);
}

bool DaisyUSBAMCInterface::IsSampleRateSupported(uint32 Rate) const
{
	for (uint8 i = 0; i < m_Class.SupportedSampleRateCount; ++i)
		if (m_Class.SupportedSampleRates[i] == Rate)
			return true;

	return false;
}

void DaisyUSBAMCInterface::BuildStreamingInterface(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, uint8 ChannelCount, uint8 Endpoint, uint8 TerminalLinkID, const AMCClassConfig& Config, bool IsOutput)
{
	uint8* buffer = EP0Buffer.configDescs;

	const uint32 defaultSampleRate = Config.SupportedSampleRates[Config.DefaultSampleRateIndex];
	const uint8 defaultBitDepths = (uint8)Config.SupportedBitDepths[Config.DefaultBitDepthIndex];
	uint8 subslotSize = (uint8)(defaultBitDepths / 8);

	// --- Alt Setting 0: zero-bandwidth ---
	USBInterfaceDescriptor* alt0 = reinterpret_cast<USBInterfaceDescriptor*>(buffer + BufferOffset);
	alt0->bLength = sizeof(USBInterfaceDescriptor);
	alt0->bDescriptorType = USBDescTypes::Interface;
	alt0->bInterfaceNumber = InterfaceIndex;
	alt0->bAlternateSetting = 0;
	alt0->bNumEndpoints = 0;
	alt0->bInterfaceClass = USBSDeviceClasses::Audio;
	alt0->bInterfaceSubClass = (uint8)AMCSubClasses::STRM;
	alt0->bInterfaceProtocol = 0x00;
	alt0->iInterface = 0;
	BufferOffset += sizeof(USBInterfaceDescriptor);

	// --- Alt Setting 1: the actual stream ---
	USBInterfaceDescriptor* alt1 = reinterpret_cast<USBInterfaceDescriptor*>(buffer + BufferOffset);
	alt1->bLength = sizeof(USBInterfaceDescriptor);
	alt1->bDescriptorType = USBDescTypes::Interface;
	alt1->bInterfaceNumber = InterfaceIndex;
	alt1->bAlternateSetting = 1;
	alt1->bNumEndpoints = 1;
	alt1->bInterfaceClass = USBSDeviceClasses::Audio;
	alt1->bInterfaceSubClass = (uint8)AMCSubClasses::STRM;
	alt1->bInterfaceProtocol = 0x00;
	alt1->iInterface = 0;
	BufferOffset += sizeof(USBInterfaceDescriptor);

	UAC1StreamingInterfaceDescriptor* asGeneral = reinterpret_cast<UAC1StreamingInterfaceDescriptor*>(buffer + BufferOffset);
	asGeneral->bLength = sizeof(UAC1StreamingInterfaceDescriptor);
	asGeneral->bDescriptorType = USBDescTypes::CS_INTERFACE;
	asGeneral->bDescriptorSubtype = AS_DESC_GENERAL;
	asGeneral->bTerminalLink = TerminalLinkID;
	asGeneral->bDelay = 0;
	asGeneral->wFormatTag = AUDIO_FORMAT_TAG_PCM;
	BufferOffset += sizeof(UAC1StreamingInterfaceDescriptor);

	// Format Type I (variable-length: sample rate list goes directly here)
	{
		uint8 rateCount = Config.SupportedSampleRateCount;
		uint8 formatLength = 8 + (rateCount * 3);

		uint8* f = buffer + BufferOffset;
		f[0] = formatLength;
		f[1] = (uint8)USBDescTypes::CS_INTERFACE;
		f[2] = AS_DESC_FORMAT_TYPE;
		f[3] = AUDIO_FORMAT_TYPE_I;
		f[4] = ChannelCount;
		f[5] = subslotSize;
		f[6] = defaultBitDepths;
		f[7] = rateCount; // bSamFreqType: discrete, not continuous

		for (uint8 i = 0; i < rateCount; ++i)
		{
			uint32 rate = Config.SupportedSampleRates[i];
			f[8 + i * 3 + 0] = (uint8)(rate & 0xFF);
			f[8 + i * 3 + 1] = (uint8)((rate >> 8) & 0xFF);
			f[8 + i * 3 + 2] = (uint8)((rate >> 16) & 0xFF);
		}

		BufferOffset += formatLength;
	}

	uint16 packetSize = CalculateIsoPacketSize(ChannelCount, Config);

	USBEndpointDescriptor* ep = reinterpret_cast<USBEndpointDescriptor*>(buffer + BufferOffset);
	ep->bLength = sizeof(USBEndpointDescriptor);
	ep->bDescriptorType = USBDescTypes::Endpoint;
	ep->bEndpointAddress = Endpoint;
	ep->bmAttributes = (uint8)USBEndpointAttributes::Isochronous | (uint8)EndpointSyncTypes::Async;
	ep->wMaxPacketSize = packetSize;
	ep->bInterval = 1;
	BufferOffset += sizeof(USBEndpointDescriptor);

	UAC1IsoEndpointDescriptor* isoDesc = reinterpret_cast<UAC1IsoEndpointDescriptor*>(buffer + BufferOffset);
	isoDesc->bLength = sizeof(UAC1IsoEndpointDescriptor);
	isoDesc->bDescriptorType = USBDescTypes::CS_ENDPOINT;
	isoDesc->bDescriptorSubtype = AS_DESC_EP_GENERAL;
	isoDesc->bmAttributes = (Config.SupportedSampleRateCount != 1 || Config.SupportedBitDepthCount != 1 ? 0x01 : 0x00);
	isoDesc->bLockDelayUnits = 0;
	isoDesc->wLockDelay = 0;
	BufferOffset += sizeof(UAC1IsoEndpointDescriptor);
}

void DaisyUSBAMCInterface::CalculateStreamingInterfaceIndices(const Configs& Configs, const AMCClassConfig& Class, uint8& OutInterfaceIndex, uint8& InInterfaceIndex)
{
	uint8 nextIf = Configs.InterfaceIndexStart + 1;

	OutInterfaceIndex = (Class.OutputChannelCount > 0) ? nextIf++ : 0;
	InInterfaceIndex = (Class.InputChannelCount > 0) ? nextIf++ : 0;
}

uint8 DaisyUSBAMCInterface::CalculateRequiredInterfaceCount(const AMCClassConfig& Class)
{
	return 1 + (Class.OutputChannelCount > 0 ? 1 : 0) + (Class.InputChannelCount > 0 ? 1 : 0);
}

uint16 DaisyUSBAMCInterface::CalculateIsoPacketSize(uint8 ChannelCount, const AMCClassConfig& Class)
{
	if (ChannelCount == 0)
		return 0;

	const uint8 defaultBitDepth = (uint8)Class.SupportedBitDepths[Class.DefaultBitDepthIndex];

	// Ensure the packet size can accommodate the highest supported sample rate.
	uint32 maxSampleRate = 0;
	for (uint8 i = 0; i < Class.SupportedSampleRateCount; ++i)
	{
		if (Class.SupportedSampleRates[i] > maxSampleRate)
			maxSampleRate = Class.SupportedSampleRates[i];
	}

	uint32 bytesPerFrame = ChannelCount * (defaultBitDepth / 8);

	// Formula uses (Rate + 999) / 1000 to safely accommodate fractional frequencies like 44.1kHz.
	uint32 bytesPerPacket = bytesPerFrame * ((maxSampleRate + 999) / 1000);

	ASSERT(bytesPerPacket <= 1023, "Packet size exceeds Full-Speed isochronous limit");

	return (uint16)bytesPerPacket;
}

#endif