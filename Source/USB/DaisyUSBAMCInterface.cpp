#ifdef ON_HARDWARE

#define ENABLE_USB_AMC_DEBUG

#ifndef DEBUG
#undef ENABLE_USB_AMC_DEBUG
#endif

#include "DaisySeedFramework/USB/DaisyUSBAMCInterface.h"
#include "DaisySeedFramework/DaisyInclude.h"
#include <DigitalSignalProcessing/Memory.h>

const uint16_t VOLUME_STEPS = 256; // 1/256 dB steps

DaisyUSBAMCInterface::DaisyUSBAMCInterface(DaisyUSBDevice* Device, const Configs& Configs, const AMCClassConfig& Class)
	: DaisyUSBInterfaceCommon(Device, Configs),
	m_Class(Class),
	m_OutAltSetting(0),
	m_InAltSetting(0),
	m_OutInterfaceIndex(0),
	m_InInterfaceIndex(0),
	m_ReceiveBuffer(nullptr),
	m_TransmitBuffer(nullptr),

	m_CurrentOutSampleRate(Class.SupportedSampleRates[Class.DefaultSampleRateIndex]),
	m_CurrentInSampleRate(Class.SupportedSampleRates[Class.DefaultSampleRateIndex]),
	m_CurrentOutBitDepth(Class.SupportedBitDepths[Class.DefaultBitDepthIndex]),
	m_CurrentInBitDepth(Class.SupportedBitDepths[Class.DefaultBitDepthIndex]),
	m_CurrentIsOutMuted(false),
	m_CurrentIsInMuted(false)

#ifdef ENABLE_USB_AMC_DEBUG
	, m_LastLogTime(0)
#endif
{
	m_ReceiveBuffer = Memory::Allocate<uint8_t>(Configs.MaxReceivePacketSize, true);
	m_TransmitBuffer = Memory::Allocate<uint8_t>(Configs.MaxTransmitPacketSize, true);

	UpdatePacketSize();

	CalculateStreamingInterfaceIndices(GetConfigs(), Class, m_OutInterfaceIndex, m_InInterfaceIndex);
}

#ifdef ENABLE_USB_AMC_DEBUG
void DaisyUSBAMCInterface::Update(void)
{
	DaisyUSBInterfaceCommon::Update();

	uint32_t now = HAL_GetTick();

	if (now - m_LastLogTime >= 1000)
	{
		m_LastLogTime = now;

		if (m_Class.InputChannelCount != 0)
			Log::WriteInfo(nullptr, "[Input-%s] URun: %u ORun: %u Iso-Incomp: %u FIFO: %u", m_Class.InputTitle, m_DebugStatsIn.Underrun, m_DebugStatsIn.Overrun, m_DebugStatsIn.IsoIncomplete, m_TransmitFIFO.GetSize());

		if (m_Class.OutputChannelCount != 0)
			Log::WriteInfo(nullptr, "[Output-%s] URun: %u ORun: %u Iso-Incomp: %u FIFO: %u", m_Class.OutputTitle, m_DebugStatsOut.Underrun, m_DebugStatsOut.Overrun, m_DebugStatsOut.IsoIncomplete, m_ReceiveFIFO.GetSize());
	}
}
#endif

bool DaisyUSBAMCInterface::OnSetupStage(const USBDeviceSetupPacket* Setup)
{
	uint8_t recipient = Setup->bmRequestType & USB_REQ_RECIPIENT_MASK;
	uint8_t controlSelector = (uint8_t)(Setup->wValue >> 8);

	// ========================================================================
	// 1. Sampling Frequency Control (Target: ENDPOINT)
	// ========================================================================
	if (recipient == USB_REQ_RECIPIENT_ENDPOINT)
	{
		uint8_t epAddress = (uint8_t)(Setup->wIndex & 0xFF);
		const Configs& configs = GetConfigs();

		if (controlSelector == CS_SAM_FREQ_CONTROL)
		{
			if (Setup->bRequest == UAC1_GET_CUR)
			{
				uint24_t val;
				if (epAddress == configs.EndpointOut)
					val = m_CurrentOutSampleRate;
				else if (epAddress == configs.EndpointIn)
					val = m_CurrentInSampleRate;

				DeviceTransmit(&val);

				DeviceReceiveAck();

				return true;
			}
			else if (Setup->bRequest == UAC1_SET_CUR)
			{
				SetPendingReceive<uint24_t>(epAddress == configs.EndpointOut ? ControlTypes::OutSampleRate : ControlTypes::InSampleRate);

				DeviceTransmitAck();

				return true;
			}
		}
	}
	// ========================================================================
	// 2. Feature Unit Controls: Mute & Volume (Target: INTERFACE)
	// ========================================================================
	else if (recipient == USB_REQ_RECIPIENT_INTERFACE)
	{
		uint8_t entityID = (uint8_t)(Setup->wIndex >> 8);
		if (entityID != FU_OUTPUT_ID && entityID != FU_INPUT_ID)
			return false;

		bool isOutput = (entityID == FU_OUTPUT_ID);

		if (controlSelector == CS_MUTE_CONTROL && m_Class.EnableHardwareMute)
		{
			if (Setup->bRequest == UAC1_SET_CUR)
			{
				SetPendingReceive<uint8_t>(isOutput ? ControlTypes::OutMute : ControlTypes::InMute);

				DeviceTransmitAck();
				return true;
			}
			else if (Setup->bRequest == UAC1_GET_CUR)
			{
				uint8_t muteVal;
				if (isOutput)
					muteVal = m_CurrentIsOutMuted;
				else
					muteVal = m_CurrentIsInMuted;

				DeviceTransmit(&muteVal);

				DeviceReceiveAck();

				return true;
			}
		}
		else if (controlSelector == CS_VOLUME_CONTROL && m_Class.EnableHardwareVolumeControl)
		{
			switch (Setup->bRequest)
			{
			case UAC1_SET_CUR:
			{
				SetPendingReceive<int16_t>(isOutput ? ControlTypes::OutVolume : ControlTypes::InVolume);

				DeviceTransmitAck();

				return true;
			}
			case UAC1_GET_CUR:
			{
				int16_t volume;
				if (isOutput)
					volume = (dBGain)m_CurrentOutVolume * VOLUME_STEPS;
				else
					volume = (dBGain)m_CurrentInVolume * VOLUME_STEPS;

				DeviceTransmit(&volume);

				DeviceReceiveAck();

				return true;
			}
			case UAC1_GET_MIN:
			{
				int16_t minVol = m_Class.MinimumVolume * VOLUME_STEPS;
				DeviceTransmit(&minVol);

				DeviceReceiveAck();

				return true;
			}
			case UAC1_GET_MAX:
			{
				int16_t maxVol = m_Class.MaximumVolume * VOLUME_STEPS;
				DeviceTransmit(&maxVol);

				DeviceReceiveAck();

				return true;
			}
			case UAC1_GET_RES:
			{
				int16_t resVol = m_Class.VolumeResolution * VOLUME_STEPS;
				DeviceTransmit(&resVol);

				DeviceReceiveAck();

				return true;
			}
			}
		}
	}

	return false;
}

void DaisyUSBAMCInterface::OnSetupCompleted(void)
{
	const Configs& configs = GetConfigs();

	if (TO_ENDPOINT_NUMBER(configs.EndpointIn) != 0)
		AllocateTransmitBuffer(configs.EndpointIn, configs.MaxTransmitPacketSize);
}

void DaisyUSBAMCInterface::OnDeviceDataOutStage(void)
{
	ControlTypes controlType = GetPendingType<ControlTypes>();

	switch (controlType)
	{
	case ControlTypes::OutSampleRate:
	{
		uint24_t sampleRate = ReadPendingReceive<uint24_t>();

		if (!IsSampleRateSupported(sampleRate))
			return;

		m_CurrentOutSampleRate = sampleRate;

		UpdatePacketSize();

		break;
	}
	case ControlTypes::InSampleRate:
	{
		uint24_t sampleRate = ReadPendingReceive<uint24_t>();

		if (!IsSampleRateSupported(sampleRate))
			return;

		m_CurrentInSampleRate = sampleRate;

		UpdatePacketSize();

		break;
	}

	case ControlTypes::OutMute:
	{
		m_CurrentIsOutMuted = (ReadPendingReceive<uint8_t>() != 0);

		break;
	}
	case ControlTypes::InMute:
	{
		m_CurrentIsInMuted = (ReadPendingReceive<uint8_t>() != 0);

		break;
	}

	case ControlTypes::OutVolume:
	{
		m_CurrentOutVolume = dBGain((float)ReadPendingReceive<int16_t>() / VOLUME_STEPS);

		break;
	}
	case ControlTypes::InVolume:
	{
		m_CurrentInVolume = dBGain((float)ReadPendingReceive<int16_t>() / VOLUME_STEPS);

		break;
	}
	default:
		return;
	}

	m_ControlChangedCallback(controlType);
}

void DaisyUSBAMCInterface::OnDataOutStage(void)
{
	const Configs& configs = GetConfigs();

	uint16_t len = EndpointReceiveCount();

	if (len > 0)
	{
		EndpointReceive(m_ReceiveBuffer, len);

		uint16_t bytesPushed = m_ReceiveFIFO.Push(m_ReceiveBuffer, len);

#ifdef ENABLE_USB_AMC_DEBUG
		if (bytesPushed < len)
			m_DebugStatsOut.Overrun++;
#endif
	}

	EndpointPrepareReceive(m_ReceiveBuffer, configs.MaxReceivePacketSize);
}

void DaisyUSBAMCInterface::OnDataInStage(void)
{
	TransmitBuffer();
}

void DaisyUSBAMCInterface::OnIsoInIncomplete(void)
{
#ifdef ENABLE_USB_AMC_DEBUG
	m_DebugStatsIn.IsoIncomplete++;
#endif

	EndpointTransmitFlush();

	TransmitBuffer();
}

void DaisyUSBAMCInterface::OnIsoOutIncomplete(void)
{
#ifdef ENABLE_USB_AMC_DEBUG
	m_DebugStatsOut.IsoIncomplete++;
#endif

	const Configs& configs = GetConfigs();

	EndpointReceiveFlush();

	EndpointPrepareReceive(m_ReceiveBuffer, configs.MaxReceivePacketSize);
}

bool DaisyUSBAMCInterface::OnSetInterface(uint8_t InterfaceIndex, uint8_t AlternateSetting)
{
	if (AlternateSetting > 1)
		return false;

	const Configs& configs = GetConfigs();

	if (InterfaceIndex == m_OutInterfaceIndex)
	{
		if (m_OutAltSetting == AlternateSetting)
			return true;

		if (AlternateSetting == 1)
		{
			OpenEndpoint(configs.EndpointOut, configs.MaxReceivePacketSize, USBEndpointAttributes::Isochronous);

			EndpointPrepareReceive(m_ReceiveBuffer, configs.MaxReceivePacketSize);
		}
		else
			CloseEndpoint(configs.EndpointOut);

		m_ReceiveFIFO.Clear();

		m_OutAltSetting = AlternateSetting;
	}
	else if (InterfaceIndex == m_InInterfaceIndex)
	{
		if (m_InAltSetting == AlternateSetting)
			return true;

		m_TransmitFIFO.Clear();

		if (AlternateSetting == 0)
			CloseEndpoint(configs.EndpointIn);
		else
		{
			OpenEndpoint(configs.EndpointIn, configs.MaxTransmitPacketSize, USBEndpointAttributes::Isochronous);

			TransmitBuffer();
		}

		m_InAltSetting = AlternateSetting;
	}

	return true;
}

uint8_t DaisyUSBAMCInterface::GetCurrentAltSetting(uint8_t InterfaceIndex) const
{
	if (InterfaceIndex == m_OutInterfaceIndex)
		return m_OutAltSetting;

	if (InterfaceIndex == m_InInterfaceIndex)
		return m_InAltSetting;

	return 0;
}

void DaisyUSBAMCInterface::BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16_t& BufferOffset, uint8_t InterfaceIndex) const
{
	const Configs& configs = GetConfigs();

	uint8_t* buffer = EP0Buffer.configDescs;

	bool hasOutput = (m_Class.OutputChannelCount > 0);
	bool hasInput = (m_Class.InputChannelCount > 0);

	USBInterfaceAssociationDescriptor* iad = reinterpret_cast<USBInterfaceAssociationDescriptor*>(buffer + BufferOffset);
	iad->bLength = sizeof(USBInterfaceAssociationDescriptor);
	iad->bDescriptorType = USBDescTypes::InterfaceAssociation;
	iad->bFirstInterface = InterfaceIndex;
	iad->bInterfaceCount = CalculateRequiredInterfaceCount(m_Class);
	iad->bFunctionClass = USBSDeviceClasses::Audio;
	iad->bFunctionSubClass = (uint8_t)AMCSubClasses::CTRL;
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
	acIf->bInterfaceSubClass = (uint8_t)AMCSubClasses::CTRL;
	acIf->bInterfaceProtocol = 0x00;
	acIf->iInterface = 0;
	BufferOffset += sizeof(USBInterfaceDescriptor);

	uint16_t acDescStart = BufferOffset;

	uint8_t asInterfaceNumbers[2];
	uint8_t asInterfaceCount = 0;
	if (hasOutput)
		asInterfaceNumbers[asInterfaceCount++] = m_OutInterfaceIndex;
	if (hasInput)
		asInterfaceNumbers[asInterfaceCount++] = m_InInterfaceIndex;

	uint8_t headerLength = 8 + asInterfaceCount;
	{
		uint8_t* h = buffer + BufferOffset;
		h[0] = headerLength;
		h[1] = (uint8_t)USBDescTypes::CS_INTERFACE;
		h[2] = AC_DESC_HEADER;
		h[3] = 0x00;
		h[4] = 0x01;       // bcdADC = 0x0100
		// h[5],h[6] = wTotalLength, filled in below once known
		h[7] = asInterfaceCount;         // bInCollection
		for (uint8_t i = 0; i < asInterfaceCount; ++i)
			h[8 + i] = asInterfaceNumbers[i];

		BufferOffset += headerLength;
	}
	uint8_t* headerBytes = buffer + acDescStart; // kept so we can fill wTotalLength at the end

	// ---------- Playback chain (host -> hardware) ----------
	if (hasOutput)
	{
		UAC1InputTerminalDescriptor* it = reinterpret_cast<UAC1InputTerminalDescriptor*>(buffer + BufferOffset);
		it->bLength = sizeof(UAC1InputTerminalDescriptor);
		it->bDescriptorType = USBDescTypes::CS_INTERFACE;
		it->bDescriptorSubtype = AC_DESC_INPUT_TERMINAL;
		it->bTerminalID = IT_USB_STREAMING_ID;
		it->wTerminalType = (uint16_t)TerminalTypes::USBStreaming;
		it->bAssocTerminal = 0;
		it->bNrChannels = m_Class.OutputChannelCount;
		it->wChannelConfig = (uint16_t)(m_Class.OutputChannelCount == 2 ? ChannelOutputPositions::Stereo : ChannelOutputPositions::Unknown);
		it->iChannelNames = 0;
		it->iTerminal = 0;
		BufferOffset += sizeof(UAC1InputTerminalDescriptor);

		// Feature Unit (variable-length, bControlSize=1)
		{
			uint8_t channelCount = m_Class.OutputChannelCount;
			uint8_t fuLength = 7 + (channelCount + 1); // bControlSize=1 byte per channel + master

			uint8_t* fuStart = buffer + BufferOffset;
			fuStart[0] = fuLength;
			fuStart[1] = (uint8_t)USBDescTypes::CS_INTERFACE;
			fuStart[2] = AC_DESC_FEATURE_UNIT;
			fuStart[3] = FU_OUTPUT_ID;
			fuStart[4] = IT_USB_STREAMING_ID;
			fuStart[5] = 1; // bControlSize

			uint8_t masterControls = 0;
			if (m_Class.EnableHardwareMute)
				masterControls |= 0x01;
			if (m_Class.EnableHardwareVolumeControl)
				masterControls |= 0x02;

			fuStart[6] = masterControls; // master channel
			for (uint8_t ch = 1; ch <= channelCount; ++ch)
				fuStart[6 + ch] = 0; // no independent per-channel control

			fuStart[6 + channelCount + 1] = 0; // iFeature

			BufferOffset += fuLength;
		}

		UAC1OutputTerminalDescriptor* ot = reinterpret_cast<UAC1OutputTerminalDescriptor*>(buffer + BufferOffset);
		ot->bLength = sizeof(UAC1OutputTerminalDescriptor);
		ot->bDescriptorType = USBDescTypes::CS_INTERFACE;
		ot->bDescriptorSubtype = AC_DESC_OUTPUT_TERMINAL;
		ot->bTerminalID = OT_OUTPUT_ID;
		ot->wTerminalType = (uint16_t)TerminalTypes::LineOut;
		ot->bAssocTerminal = 0;
		ot->bSourceID = FU_OUTPUT_ID;
		ot->iTerminal = (m_Class.OutputTitle == nullptr ? 0 : USB_STRING_INDEX_OUT_TERMINAL_BASE + m_OutInterfaceIndex);
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
		it->wTerminalType = (uint16_t)TerminalTypes::LineIn;
		it->bAssocTerminal = 0;
		it->bNrChannels = m_Class.InputChannelCount;
		it->wChannelConfig = (uint16_t)(m_Class.InputChannelCount == 2 ? ChannelOutputPositions::Stereo : ChannelOutputPositions::Unknown);
		it->iChannelNames = 0;
		it->iTerminal = (m_Class.InputTitle == nullptr ? 0 : USB_STRING_INDEX_IN_TERMINAL_BASE + m_InInterfaceIndex);
		BufferOffset += sizeof(UAC1InputTerminalDescriptor);

		{
			uint8_t channelCount = m_Class.InputChannelCount;
			uint8_t fuLength = 7 + (channelCount + 1);

			uint8_t* fuStart = buffer + BufferOffset;
			fuStart[0] = fuLength;
			fuStart[1] = (uint8_t)USBDescTypes::CS_INTERFACE;
			fuStart[2] = AC_DESC_FEATURE_UNIT;
			fuStart[3] = FU_INPUT_ID;
			fuStart[4] = IT_INPUT_ID;
			fuStart[5] = 1; // bControlSize

			uint8_t masterControls = 0;
			if (m_Class.EnableHardwareMute)
				masterControls |= 0x01;
			if (m_Class.EnableHardwareVolumeControl)
				masterControls |= 0x02;

			fuStart[6] = masterControls;
			for (uint8_t ch = 1; ch <= channelCount; ++ch)
				fuStart[6 + ch] = 0;

			fuStart[6 + channelCount + 1] = 0; // iFeature

			BufferOffset += fuLength;
		}

		UAC1OutputTerminalDescriptor* ot = reinterpret_cast<UAC1OutputTerminalDescriptor*>(buffer + BufferOffset);
		ot->bLength = sizeof(UAC1OutputTerminalDescriptor);
		ot->bDescriptorType = USBDescTypes::CS_INTERFACE;
		ot->bDescriptorSubtype = AC_DESC_OUTPUT_TERMINAL;
		ot->bTerminalID = OT_USB_STREAMING_ID;
		ot->wTerminalType = (uint16_t)TerminalTypes::USBStreaming;
		ot->bAssocTerminal = 0;
		ot->bSourceID = FU_INPUT_ID;
		ot->iTerminal = 0;
		BufferOffset += sizeof(UAC1OutputTerminalDescriptor);
	}

	// Now that the whole AC block has been built, fill in wTotalLength
	uint16_t totalLen = BufferOffset - acDescStart;
	headerBytes[5] = (uint8_t)(totalLen & 0xFF);
	headerBytes[6] = (uint8_t)((totalLen >> 8) & 0xFF);

	// ---------- Output AS Interface ----------
	if (hasOutput)
		BuildStreamingInterface(EP0Buffer, BufferOffset, m_OutInterfaceIndex, m_Class.OutputChannelCount, configs.EndpointOut, IT_USB_STREAMING_ID, m_Class);

	// ---------- Input AS Interface ----------
	if (hasInput)
		BuildStreamingInterface(EP0Buffer, BufferOffset, m_InInterfaceIndex, m_Class.InputChannelCount, configs.EndpointIn, OT_USB_STREAMING_ID, m_Class);
}

cstr DaisyUSBAMCInterface::GetDescriptorString(uint8_t StringIndex) const
{
	if (StringIndex == USB_STRING_INDEX_OUT_TERMINAL_BASE + m_OutInterfaceIndex)
		return m_Class.OutputTitle;
	else if (StringIndex == USB_STRING_INDEX_IN_TERMINAL_BASE + m_InInterfaceIndex)
		return m_Class.InputTitle;

	return nullptr;
}

void DaisyUSBAMCInterface::TransmitBuffer(void)
{
	const Configs& configs = GetConfigs();

	uint16_t bytesRead = m_TransmitFIFO.Pop(m_TransmitBuffer, m_CurrentTransmitPacketSize);

	if (bytesRead < m_CurrentTransmitPacketSize)
	{
#ifdef ENABLE_USB_AMC_DEBUG
		m_DebugStatsIn.Underrun++;
#endif

		Memory::Set(m_TransmitBuffer + bytesRead, 0, m_CurrentTransmitPacketSize - bytesRead);
	}

	EndpointTransmit(m_TransmitBuffer, m_CurrentTransmitPacketSize);
}

bool DaisyUSBAMCInterface::IsSampleRateSupported(uint32_t Rate) const
{
	for (uint8_t i = 0; i < m_Class.SupportedSampleRateCount; ++i)
		if (m_Class.SupportedSampleRates[i] == Rate)
			return true;

	return false;
}

void DaisyUSBAMCInterface::UpdatePacketSize(void)
{
	m_CurrentReceivePacketSize = CalculatePacketSize(m_Class.OutputChannelCount, m_CurrentOutSampleRate, m_CurrentOutBitDepth);
	m_CurrentTransmitPacketSize = CalculatePacketSize(m_Class.InputChannelCount, m_CurrentInSampleRate, m_CurrentInBitDepth);
}

void DaisyUSBAMCInterface::BuildStreamingInterface(EP0Buffer& EP0Buffer, uint16_t& BufferOffset, uint8_t InterfaceIndex, uint8_t ChannelCount, uint8_t Endpoint, uint8_t TerminalLinkID, const AMCClassConfig& Config)
{
	uint8_t* buffer = EP0Buffer.configDescs;

	const uint8_t defaultBitDepths = (uint8_t)Config.SupportedBitDepths[Config.DefaultBitDepthIndex];
	uint8_t subslotSize = (uint8_t)(defaultBitDepths / 8);

	// --- Alt Setting 0: zero-bandwidth ---
	USBInterfaceDescriptor* alt0 = reinterpret_cast<USBInterfaceDescriptor*>(buffer + BufferOffset);
	alt0->bLength = sizeof(USBInterfaceDescriptor);
	alt0->bDescriptorType = USBDescTypes::Interface;
	alt0->bInterfaceNumber = InterfaceIndex;
	alt0->bAlternateSetting = 0;
	alt0->bNumEndpoints = 0;
	alt0->bInterfaceClass = USBSDeviceClasses::Audio;
	alt0->bInterfaceSubClass = (uint8_t)AMCSubClasses::STRM;
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
	alt1->bInterfaceSubClass = (uint8_t)AMCSubClasses::STRM;
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
		uint8_t rateCount = Config.SupportedSampleRateCount;
		uint8_t formatLength = 8 + (rateCount * 3);

		uint8_t* f = buffer + BufferOffset;
		f[0] = formatLength;
		f[1] = (uint8_t)USBDescTypes::CS_INTERFACE;
		f[2] = AS_DESC_FORMAT_TYPE;
		f[3] = AUDIO_FORMAT_TYPE_I;
		f[4] = ChannelCount;
		f[5] = subslotSize;
		f[6] = defaultBitDepths;
		f[7] = rateCount; // bSamFreqType: discrete, not continuous

		for (uint8_t i = 0; i < rateCount; ++i)
		{
			uint32_t rate = Config.SupportedSampleRates[i];
			f[8 + i * 3 + 0] = (uint8_t)(rate & 0xFF);
			f[8 + i * 3 + 1] = (uint8_t)((rate >> 8) & 0xFF);
			f[8 + i * 3 + 2] = (uint8_t)((rate >> 16) & 0xFF);
		}

		BufferOffset += formatLength;
	}

	USBEndpointDescriptor* ep = reinterpret_cast<USBEndpointDescriptor*>(buffer + BufferOffset);
	ep->bLength = sizeof(USBEndpointDescriptor);
	ep->bDescriptorType = USBDescTypes::Endpoint;
	ep->bEndpointAddress = Endpoint;
	ep->bmAttributes = (uint8_t)(USBEndpointAttributes::Isochronous | EndpointSyncTypes::Async);
	ep->wMaxPacketSize = CalculateMaxPacketSize(ChannelCount, Config);
	ep->bInterval = 1;
	BufferOffset += sizeof(USBEndpointDescriptor);

	UAC1IsoEndpointDescriptor* isoDesc = reinterpret_cast<UAC1IsoEndpointDescriptor*>(buffer + BufferOffset);
	isoDesc->bLength = sizeof(UAC1IsoEndpointDescriptor);
	isoDesc->bDescriptorType = USBDescTypes::CS_ENDPOINT;
	isoDesc->bDescriptorSubtype = AS_DESC_EP_GENERAL;
	isoDesc->bmAttributes = (uint8_t)(Config.SupportedSampleRateCount != 1 || Config.SupportedBitDepthCount != 1 ? 0x01 : 0x00);
	isoDesc->bLockDelayUnits = 0;
	isoDesc->wLockDelay = 0;
	BufferOffset += sizeof(UAC1IsoEndpointDescriptor);
}

void DaisyUSBAMCInterface::CalculateStreamingInterfaceIndices(const Configs& Configs, const AMCClassConfig& Class, uint8_t& OutInterfaceIndex, uint8_t& InInterfaceIndex)
{
	uint8_t nextIf = Configs.InterfaceIndexStart + 1;

	OutInterfaceIndex = (Class.OutputChannelCount > 0) ? nextIf++ : 0;
	InInterfaceIndex = (Class.InputChannelCount > 0) ? nextIf++ : 0;
}

uint8_t DaisyUSBAMCInterface::CalculateRequiredInterfaceCount(const AMCClassConfig& Class)
{
	return 1 + (Class.OutputChannelCount > 0 ? 1 : 0) + (Class.InputChannelCount > 0 ? 1 : 0);
}

uint16_t DaisyUSBAMCInterface::CalculateMaxPacketSize(uint8_t ChannelCount, const AMCClassConfig& Class)
{
	if (ChannelCount == 0)
		return 0;

	BitDepths maxBitDepth = BitDepths::BitDepths8;
	for (uint8_t i = 0; i < Class.SupportedBitDepthCount; ++i)
	{
		if (Class.SupportedBitDepths[i] > maxBitDepth)
			maxBitDepth = Class.SupportedBitDepths[i];
	}

	uint32_t maxSampleRate = 0;
	for (uint8_t i = 0; i < Class.SupportedSampleRateCount; ++i)
	{
		if (Class.SupportedSampleRates[i] > maxSampleRate)
			maxSampleRate = Class.SupportedSampleRates[i];
	}

	return CalculatePacketSize(ChannelCount, maxSampleRate, (BitDepths)maxBitDepth);
}

uint16_t DaisyUSBAMCInterface::CalculatePacketSize(uint8_t ChannelCount, uint32_t SampleRate, BitDepths BitDepth)
{
	uint32_t bytesPerFrame = ChannelCount * ((uint8_t)BitDepth / 8);

	// Formula uses (Rate + 999) / 1000 to safely accommodate fractional frequencies like 44.1kHz.
	uint32_t bytesPerPacket = bytesPerFrame * ((SampleRate + 999) / 1000);

	ASSERT(bytesPerPacket <= 1023, "Packet size exceeds Full-Speed isochronous limit");

	return (uint16_t)bytesPerPacket;
}

#endif