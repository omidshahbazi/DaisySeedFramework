#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBAMCInterface.h"
#include "DaisySeedFramework/USB/DaisyUSB.h"
#include <DigitalSignalProcessing/Memory.h>

DaisyUSBAMCInterface::DaisyUSBAMCInterface(DaisyUSB* USB, const Configs& Configs, const AMCClassConfig& Class)
	: DaisyUSBInterfaceCommon(USB, Configs),
	m_Class(Class),
	m_CurrentSampleRate(Class.SupportedSampleRates[Class.DefaultSampleRateIndex]),
	m_OutAltSetting(0),
	m_InAltSetting(0),
	m_OutInterfaceIndex(0),
	m_InInterfaceIndex(0)
{
	CalculateStreamingInterfaceIndices(GetConfigs(), Class, m_OutInterfaceIndex, m_InInterfaceIndex);
}

bool DaisyUSBAMCInterface::OnSetupStage(const USBDeviceSetupPacket* Setup)
{
	uint8 entityId = (uint8)(Setup->wIndex >> 8);
	uint8 controlSelector = (uint8)(Setup->wValue >> 8);

	if (entityId == CLOCK_SOURCE_ID && controlSelector == CS_SAM_FREQ_CONTROL)
	{
		bool isDeviceToHost = ((Setup->bmRequestType & USB_REQ_TYPE_MASK) == USB_REQ_DIR_DEVICE_TO_HOST);

		if (Setup->bRequest == UAC2_REQ_CUR)
		{
			if (isDeviceToHost)
			{
				GetUSB()->DeviceTransmit(&m_CurrentSampleRate);
				GetUSB()->DeviceReceiveAck();
			}
			else
			{
				uint32 requestedRate;
				GetUSB()->DeviceReceive(&requestedRate);

				if (!IsSampleRateSupported(requestedRate))
					return false;

				m_CurrentSampleRate = requestedRate;
				GetUSB()->DeviceTransmitAck();
			}
			return true;
		}

		if (Setup->bRequest == UAC2_REQ_RANGE && isDeviceToHost)
		{
			uint16 len = BuildSampleRateRangeResponse();
			GetUSB()->DeviceTransmit(m_RangeResponseBuffer, len);
			GetUSB()->DeviceReceiveAck();
			return true;
		}
	}

	// Feature Unit (Mute/Volume) — فعلاً فقط ACK می‌کنیم، مقدار واقعی رو ذخیره نمی‌کنیم
	if ((entityId == FU_SPEAKER_ID || entityId == FU_MIC_ID) && controlSelector == CS_MUTE_CONTROL && m_Class.EnableHardwareMute)
	{
		bool isDeviceToHost = ((Setup->bmRequestType & USB_REQ_TYPE_MASK) == USB_REQ_DIR_DEVICE_TO_HOST);

		if (isDeviceToHost)
		{
			uint8 muted = 0;
			GetUSB()->DeviceTransmit(&muted);
			GetUSB()->DeviceReceiveAck();
		}
		else
		{
			uint8 requestedMute;
			GetUSB()->DeviceReceive(&requestedMute);
			GetUSB()->DeviceTransmitAck();
		}
		return true;
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
{}

void DaisyUSBAMCInterface::OnDataOutStage(void)
{}

bool DaisyUSBAMCInterface::OnSetInterface(uint8 InterfaceIndex, uint8 AlternateSetting)
{
	if (AlternateSetting > 1)
		return false;

	bool isOutInterface = (InterfaceIndex == m_OutInterfaceIndex);
	bool isInInterface = (InterfaceIndex == m_InInterfaceIndex);

	if (!isOutInterface && !isInInterface)
		return false;

	uint8& currentAlt = isOutInterface ? m_OutAltSetting : m_InAltSetting;

	if (currentAlt == AlternateSetting)
		return true;

	uint8 endpoint = isOutInterface ? GetConfigs().EndpointOut : GetConfigs().EndpointIn;

	if (AlternateSetting == 1)
	{
		// فقط باز کردن endpoint — بدون AllocateTransmitBuffer، چون قبلاً توی OnSetupCompleted انجام شده
		USBEpAttr mode = USBEpAttr::Isochronous;
		uint16 size = isOutInterface ? GetConfigs().ReceivePacketSize : GetConfigs().TransmitPacketSize;

		GetUSB()->OpenEndpoint(endpoint, size, mode);
	}
	else
	{
		GetUSB()->CloseEndpoint(endpoint);
	}

	currentAlt = AlternateSetting;
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

	uint8 asOutInterface, asInInterface;
	CalculateStreamingInterfaceIndices(GetConfigs(), Config, asOutInterface, asInInterface);

	// ---------- IAD ----------
	USBInterfaceAssociationDescriptor* iad = reinterpret_cast<USBInterfaceAssociationDescriptor*>(buffer + BufferOffset);
	iad->bLength = sizeof(USBInterfaceAssociationDescriptor);
	iad->bDescriptorType = USBDescType::InterfaceAssociation;
	iad->bFirstInterface = InterfaceIndex;
	iad->bInterfaceCount = CalculateRequiredInterfaceCount(Config);
	iad->bFunctionClass = USBSDeviceClass::Audio;
	iad->bFunctionSubClass = AUDIO_SUBCLASS_CONTROL;
	iad->bFunctionProtocol = AUDIO_FUNCTION_PROTOCOL_AF_2_0;
	iad->iFunction = 0;
	BufferOffset += sizeof(USBInterfaceAssociationDescriptor);

	// ---------- AC Interface (standard) ----------
	USBInterfaceDescriptor* acIf = reinterpret_cast<USBInterfaceDescriptor*>(buffer + BufferOffset);
	acIf->bLength = sizeof(USBInterfaceDescriptor);
	acIf->bDescriptorType = USBDescType::Interface;
	acIf->bInterfaceNumber = InterfaceIndex;
	acIf->bAlternateSetting = 0;
	acIf->bNumEndpoints = 0;
	acIf->bInterfaceClass = USBSDeviceClass::Audio;
	acIf->bInterfaceSubClass = AUDIO_SUBCLASS_CONTROL;
	acIf->bInterfaceProtocol = AUDIO_PROTOCOL_UAC2;
	acIf->iInterface = 0;
	BufferOffset += sizeof(USBInterfaceDescriptor);

	// ---------- AC Header ----------
	// wTotalLength باید طول کل بلاک class-specific AC (Header + Clock + Terminals + FeatureUnits) باشه،
	// نه شامل خود Interface/IAD استاندارد. اینجا آفستش رو نگه می‌داریم تا در آخر پرش کنیم.
	uint16 acDescStart = BufferOffset;

	UACHeaderDescriptor* header = reinterpret_cast<UACHeaderDescriptor*>(buffer + BufferOffset);
	header->bLength = sizeof(UACHeaderDescriptor);
	header->bDescriptorType = USBDescType::CDCFunc; // 0x24، همون class-specific interface subtype که CDC هم استفاده می‌کرد
	header->bDescriptorSubtype = AC_DESC_HEADER;
	header->bcdADC = 0x0200;
	header->bCategory = AUDIO_CATEGORY_IO_BOX;
	header->bmControls = 0;
	// wTotalLength بعداً پر میشه
	BufferOffset += sizeof(UACHeaderDescriptor);

	// ---------- Clock Source ----------
	UACClockSourceDescriptor* clock = reinterpret_cast<UACClockSourceDescriptor*>(buffer + BufferOffset);
	clock->bLength = sizeof(UACClockSourceDescriptor);
	clock->bDescriptorType = USBDescType::CDCFunc;
	clock->bDescriptorSubtype = AC_DESC_CLOCK_SOURCE;
	clock->bClockID = CLOCK_SOURCE_ID;
	clock->bmAttributes = 0x03; // Internal, variable-rate (چون از پروفایل چند سمپل‌ریت پشتیبانی می‌کنیم)
	clock->bmControls = 0x03;   // Sampling Frequency Control: Host Read + Host Write (حیاتی، وگرنه ویندوز SET_CUR نمی‌فرسته)
	clock->bAssocTerminal = 0;
	clock->iClockSource = 0;
	BufferOffset += sizeof(UACClockSourceDescriptor);

	// ---------- زنجیره‌ی خروجی (Playback: هاست -> پدال) ----------
	if (hasOutput)
	{
		UACInputTerminalDescriptor* it = reinterpret_cast<UACInputTerminalDescriptor*>(buffer + BufferOffset);
		it->bLength = sizeof(UACInputTerminalDescriptor);
		it->bDescriptorType = USBDescType::CDCFunc;
		it->bDescriptorSubtype = AC_DESC_INPUT_TERMINAL;
		it->bTerminalID = IT_USB_STREAMING_ID;
		it->wTerminalType = AUDIO_TERMINAL_USB_STREAMING;
		it->bAssocTerminal = 0;
		it->bCSourceID = CLOCK_SOURCE_ID;
		it->bNrChannels = Config.OutputChannelCount;
		it->bmChannelConfig = (uint32)ChannelOutputPosition::Unknown;
		it->iChannelNames = 0;
		it->bmControls = 0;
		it->iTerminal = 0;
		BufferOffset += sizeof(UACInputTerminalDescriptor);

		//UACFeatureUnitDescriptor
		{
			uint8 channelCount = Config.OutputChannelCount; // یا InputChannelCount
			uint8 fuLength = 6 + (channelCount + 1) * 4;

			uint8* fuStart = buffer + BufferOffset;
			fuStart[0] = fuLength;
			fuStart[1] = (uint8)USBDescType::CDCFunc;
			fuStart[2] = AC_DESC_FEATURE_UNIT;
			fuStart[3] = FU_SPEAKER_ID;
			fuStart[4] = IT_USB_STREAMING_ID;

			uint32 masterControls = 0;
			if (Config.EnableHardwareMute)
				masterControls |= 0x03;
			if (Config.EnableHardwareVolumeControl)
				masterControls |= 0x0C;

			memcpy(fuStart + 5, &masterControls, sizeof(uint32));

			uint32 zero = 0;
			for (uint8 ch = 1; ch <= channelCount; ++ch)
				memcpy(fuStart + 5 + ch * sizeof(uint32), &zero, sizeof(uint32));

			fuStart[5 + (channelCount + 1) * 4] = 0; // iFeature

			BufferOffset += fuLength;
		}

		UACOutputTerminalDescriptor* ot = reinterpret_cast<UACOutputTerminalDescriptor*>(buffer + BufferOffset);
		ot->bLength = sizeof(UACOutputTerminalDescriptor);
		ot->bDescriptorType = USBDescType::CDCFunc;
		ot->bDescriptorSubtype = AC_DESC_OUTPUT_TERMINAL;
		ot->bTerminalID = OT_SPEAKER_ID;
		ot->wTerminalType = AUDIO_TERMINAL_SPEAKER;
		ot->bAssocTerminal = 0;
		ot->bSourceID = FU_SPEAKER_ID;
		ot->bCSourceID = CLOCK_SOURCE_ID;
		ot->bmControls = 0;
		ot->iTerminal = 0;
		BufferOffset += sizeof(UACOutputTerminalDescriptor);
	}

	// ---------- زنجیره‌ی ورودی (Capture: پدال -> هاست) ----------
	if (hasInput)
	{
		UACInputTerminalDescriptor* it = reinterpret_cast<UACInputTerminalDescriptor*>(buffer + BufferOffset);
		it->bLength = sizeof(UACInputTerminalDescriptor);
		it->bDescriptorType = USBDescType::CDCFunc;
		it->bDescriptorSubtype = AC_DESC_INPUT_TERMINAL;
		it->bTerminalID = IT_MIC_ID;
		it->wTerminalType = AUDIO_TERMINAL_MIC;
		it->bAssocTerminal = 0;
		it->bCSourceID = CLOCK_SOURCE_ID;
		it->bNrChannels = Config.InputChannelCount;
		it->bmChannelConfig = (uint32)ChannelOutputPosition::Unknown;
		it->iChannelNames = 0;
		it->bmControls = 0;
		it->iTerminal = 0;
		BufferOffset += sizeof(UACInputTerminalDescriptor);

		//UACFeatureUnitDescriptor
		{
			uint8 channelCount = Config.InputChannelCount;
			uint8 fuLength = 6 + (channelCount + 1) * 4;

			uint8* fuStart = buffer + BufferOffset;
			fuStart[0] = fuLength;
			fuStart[1] = (uint8)USBDescType::CDCFunc;
			fuStart[2] = AC_DESC_FEATURE_UNIT;
			fuStart[3] = FU_MIC_ID;
			fuStart[4] = IT_MIC_ID;

			uint32 masterControls = 0;
			if (Config.EnableHardwareMute)
				masterControls |= 0x03;
			if (Config.EnableHardwareVolumeControl)
				masterControls |= 0x0C;

			memcpy(fuStart + 5, &masterControls, sizeof(uint32));

			uint32 zero = 0;
			for (uint8 ch = 1; ch <= channelCount; ++ch)
				memcpy(fuStart + 5 + ch * sizeof(uint32), &zero, sizeof(uint32));

			fuStart[5 + (channelCount + 1) * 4] = 0; // iFeature

			BufferOffset += fuLength;
		}

		UACOutputTerminalDescriptor* ot = reinterpret_cast<UACOutputTerminalDescriptor*>(buffer + BufferOffset);
		ot->bLength = sizeof(UACOutputTerminalDescriptor);
		ot->bDescriptorType = USBDescType::CDCFunc;
		ot->bDescriptorSubtype = AC_DESC_OUTPUT_TERMINAL;
		ot->bTerminalID = OT_USB_STREAMING_ID;
		ot->wTerminalType = AUDIO_TERMINAL_USB_STREAMING;
		ot->bAssocTerminal = 0;
		ot->bSourceID = FU_MIC_ID;
		ot->bCSourceID = CLOCK_SOURCE_ID;
		ot->bmControls = 0;
		ot->iTerminal = 0;
		BufferOffset += sizeof(UACOutputTerminalDescriptor);
	}

	// حالا که کل بلاک AC ساخته شد، wTotalLength رو پر می‌کنیم
	header->wTotalLength = (BufferOffset - acDescStart);

	// ---------- AS Interface خروجی (Alt0 zero-bw + Alt1 با endpoint) ----------
	if (hasOutput)
		BuildStreamingInterface(EP0Buffer, BufferOffset, asOutInterface, Config.OutputChannelCount, GetConfigs().EndpointOut, IT_USB_STREAMING_ID, Config, /*isOutput=*/true);

	// ---------- AS Interface ورودی ----------
	if (hasInput)
		BuildStreamingInterface(EP0Buffer, BufferOffset, asInInterface, Config.InputChannelCount, GetConfigs().EndpointIn, OT_USB_STREAMING_ID, Config, /*isOutput=*/false);
}

bool DaisyUSBAMCInterface::IsSampleRateSupported(uint32 Rate) const
{
	for (uint8 i = 0; i < m_Class.SupportedSampleRateCount; ++i)
		if (m_Class.SupportedSampleRates[i] == Rate)
			return true;

	return false;
}

uint16 DaisyUSBAMCInterface::BuildSampleRateRangeResponse(void)
{
	uint16 numSubRanges = m_Class.SupportedSampleRateCount;
	memcpy(m_RangeResponseBuffer, &numSubRanges, sizeof(uint16));

	for (uint8 i = 0; i < m_Class.SupportedSampleRateCount; ++i)
	{
		UACFreqSubRange range;
		range.dMIN = m_Class.SupportedSampleRates[i];
		range.dMAX = m_Class.SupportedSampleRates[i];
		range.dRES = 0;

		memcpy(m_RangeResponseBuffer + sizeof(uint16) + i * sizeof(UACFreqSubRange), &range, sizeof(UACFreqSubRange));
	}

	return sizeof(uint16) + (m_Class.SupportedSampleRateCount * sizeof(UACFreqSubRange));
}

void DaisyUSBAMCInterface::BuildStreamingInterface(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, uint8 ChannelCount, uint8 Endpoint, uint8 TerminalLinkID, const AMCClassConfig& Config, bool IsOutput)
{
	uint8* buffer = EP0Buffer.configDescs;

	const uint32 defaultSampleRate = Config.SupportedSampleRates[Config.DefaultSampleRateIndex];
	const uint8 defaultBitDepths = Config.SupportedBitDepths[Config.DefaultBitDepthIndex];

	uint8 subslotSize = (uint8)(defaultBitDepths / 8);

	// --- Alt Setting 0: zero-bandwidth (باید طبق اسپک وجود داشته باشه) ---
	USBInterfaceDescriptor* alt0 = reinterpret_cast<USBInterfaceDescriptor*>(buffer + BufferOffset);
	alt0->bLength = sizeof(USBInterfaceDescriptor);
	alt0->bDescriptorType = USBDescType::Interface;
	alt0->bInterfaceNumber = InterfaceIndex;
	alt0->bAlternateSetting = 0;
	alt0->bNumEndpoints = 0;
	alt0->bInterfaceClass = USBSDeviceClass::Audio;
	alt0->bInterfaceSubClass = AUDIO_SUBCLASS_STREAMING;
	alt0->bInterfaceProtocol = AUDIO_PROTOCOL_UAC2;
	alt0->iInterface = 0;
	BufferOffset += sizeof(USBInterfaceDescriptor);

	// --- Alt Setting 1: استریم واقعی ---
	USBInterfaceDescriptor* alt1 = reinterpret_cast<USBInterfaceDescriptor*>(buffer + BufferOffset);
	alt1->bLength = sizeof(USBInterfaceDescriptor);
	alt1->bDescriptorType = USBDescType::Interface;
	alt1->bInterfaceNumber = InterfaceIndex;
	alt1->bAlternateSetting = 1;
	alt1->bNumEndpoints = 1;
	alt1->bInterfaceClass = USBSDeviceClass::Audio;
	alt1->bInterfaceSubClass = AUDIO_SUBCLASS_STREAMING;
	alt1->bInterfaceProtocol = AUDIO_PROTOCOL_UAC2;
	alt1->iInterface = 0;
	BufferOffset += sizeof(USBInterfaceDescriptor);

	UACStreamingInterfaceDescriptor* asGeneral = reinterpret_cast<UACStreamingInterfaceDescriptor*>(buffer + BufferOffset);
	asGeneral->bLength = sizeof(UACStreamingInterfaceDescriptor);
	asGeneral->bDescriptorType = USBDescType::CDCFunc;
	asGeneral->bDescriptorSubtype = AS_DESC_GENERAL;
	asGeneral->bTerminalLink = TerminalLinkID;
	asGeneral->bmControls = 0;
	asGeneral->bFormatType = AUDIO_FORMAT_TYPE_I;
	asGeneral->bmFormats = AUDIO_DATA_FORMAT_PCM;
	asGeneral->bNrChannels = ChannelCount;
	asGeneral->bmChannelConfig = (uint32)ChannelOutputPosition::Unknown;
	asGeneral->iChannelNames = 0;
	BufferOffset += sizeof(UACStreamingInterfaceDescriptor);

	UACFormatTypeIDescriptor* format = reinterpret_cast<UACFormatTypeIDescriptor*>(buffer + BufferOffset);
	format->bLength = sizeof(UACFormatTypeIDescriptor);
	format->bDescriptorType = USBDescType::CDCFunc;
	format->bDescriptorSubtype = AS_DESC_FORMAT_TYPE;
	format->bFormatType = AUDIO_FORMAT_TYPE_I;
	format->bSubslotSize = subslotSize;
	format->bBitResolution = (uint8)defaultBitDepths;
	BufferOffset += sizeof(UACFormatTypeIDescriptor);

	uint16 packetSize = ChannelCount * subslotSize * (uint16)(defaultSampleRate / 1000);

	USBEndpointDescriptor* ep = reinterpret_cast<USBEndpointDescriptor*>(buffer + BufferOffset);
	ep->bLength = sizeof(USBEndpointDescriptor);
	ep->bDescriptorType = USBDescType::Endpoint;
	ep->bEndpointAddress = Endpoint;
	ep->bmAttributes = (USBEpAttr)((uint8)USBEpAttr::Isochronous | USB_EP_SYNC_TYPE_SYNC);
	ep->wMaxPacketSize = packetSize;
	ep->bInterval = 1; // هر 1ms، چون Full-Speed
	BufferOffset += sizeof(USBEndpointDescriptor);

	UACIsoEndpointDescriptor* isoDesc = reinterpret_cast<UACIsoEndpointDescriptor*>(buffer + BufferOffset);
	isoDesc->bLength = sizeof(UACIsoEndpointDescriptor);
	isoDesc->bDescriptorType = USBDescType::CDCEndpointFunc;
	isoDesc->bDescriptorSubtype = 0x01; // AS General class-specific ISO endpoint
	isoDesc->bmAttributes = 0;
	isoDesc->bmControls = 0;
	isoDesc->bLockDelayUnits = 0;
	isoDesc->wLockDelay = 0;
	BufferOffset += sizeof(UACIsoEndpointDescriptor);
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

	const uint8 defaultBitDepths = Class.SupportedBitDepths[Class.DefaultBitDepthIndex];
	const uint32 defaultSampleRate = Class.SupportedSampleRates[Class.DefaultSampleRateIndex];

	uint32 bytesPerFrame = ChannelCount * (defaultBitDepths / 8);
	uint32 bytesPerPacket = bytesPerFrame * (defaultSampleRate / 1000);

	ASSERT(bytesPerPacket <= 1023, "Packet size exceeds Full-Speed isochronous limit");

	return (uint16)bytesPerPacket;
}

#endif