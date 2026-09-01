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
	uint8 recipient = Setup->bmRequestType & USB_REQ_RECIPIENT_MASK;
	uint8 controlSelector = (uint8)(Setup->wValue >> 8);

	// --- کنترل سمپل‌ریت: روی خود Endpoint میاد، نه یه Entity (UAC1) ---
	if (recipient == USB_REQ_RECIPIENT_ENDPOINT && controlSelector == CS_SAM_FREQ_CONTROL)
	{
		if (Setup->bRequest == UAC1_GET_CUR)
		{
			uint8 rateBytes[3];
			rateBytes[0] = (uint8)(m_CurrentSampleRate & 0xFF);
			rateBytes[1] = (uint8)((m_CurrentSampleRate >> 8) & 0xFF);
			rateBytes[2] = (uint8)((m_CurrentSampleRate >> 16) & 0xFF);

			GetUSB()->DeviceTransmit(rateBytes, sizeof(rateBytes));
			GetUSB()->DeviceReceiveAck();
			return true;
		}

		if (Setup->bRequest == UAC1_SET_CUR)
		{
			uint8 rateBytes[3];
			GetUSB()->DeviceReceive(rateBytes, sizeof(rateBytes));

			uint32 requestedRate = (uint32)rateBytes[0] | ((uint32)rateBytes[1] << 8) | ((uint32)rateBytes[2] << 16);

			if (!IsSampleRateSupported(requestedRate))
				return false;

			m_CurrentSampleRate = requestedRate;
			GetUSB()->DeviceTransmitAck();
			return true;
		}

		// SET_MIN/GET_MIN/SET_MAX/GET_MAX/SET_RES/GET_RES پیاده نشدن —
		// چون bSamFreqType گسسته (نه continuous) استفاده می‌کنیم، درایورهای
		// استاندارد نباید این‌ها رو بفرستن.
		return false;
	}

	// --- Mute/Volume: روی Feature Unit، recipient = Interface (AC) ---
	if (recipient == USB_REQ_RECIPIENT_INTERFACE)
	{
		uint8 entityId = (uint8)(Setup->wIndex >> 8);

		bool isFeatureUnit = (entityId == FU_SPEAKER_ID || entityId == FU_MIC_ID);
		bool isMute = (controlSelector == CS_MUTE_CONTROL && m_Class.EnableHardwareMute);
		bool isVolume = (controlSelector == CS_VOLUME_CONTROL && m_Class.EnableHardwareVolumeControl);

		if (isFeatureUnit && (isMute || isVolume))
		{
			bool isDeviceToHost = ((Setup->bmRequestType & USB_REQ_TYPE_MASK) == USB_REQ_DIR_DEVICE_TO_HOST);

			if (isDeviceToHost)
			{
				uint8 value = 0; // TODO: مقدار واقعی mute/volume رو وصل کن
				GetUSB()->DeviceTransmit(&value, sizeof(value));
				GetUSB()->DeviceReceiveAck();
			}
			else
			{
				uint8 requestedValue;
				GetUSB()->DeviceReceive(&requestedValue, sizeof(requestedValue));
				GetUSB()->DeviceTransmitAck();
			}
			return true;
		}
	}

	return false;
}

void DaisyUSBAMCInterface::OnSetupCompleted(void)
{
	DaisyUSB* usb = GetUSB();
	const Configs& configs = GetConfigs();

	if (TO_ENDPOINT_NUMBER(configs.EndpointIn) != 0)
	{
		uint16 sizeInWords = (configs.TransmitPacketSize + 3) / 4;
		usb->AllocateTransmitBuffer(configs.EndpointIn, sizeInWords);

		USBEpAttr mode = (USBEpAttr)((uint8)USBEpAttr::Isochronous | USB_EP_SYNC_TYPE_ASYNC);
		usb->OpenEndpoint(configs.EndpointIn, configs.TransmitPacketSize, mode);

		m_InAltSetting = 1;
		m_StreamPrimed = false;   // ← دیگه اینجا EndpointTransmit صدا نمی‌زنیم
	}
}

void DaisyUSBAMCInterface::OnDataInStage(void)
{
	memset(m_TransmitBuffer, 0, GetConfigs().TransmitPacketSize);
	EndpointTransmit(m_TransmitBuffer, GetConfigs().TransmitPacketSize);
}

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
	currentAlt = AlternateSetting;

	// دیگه اینجا هیچ OpenEndpoint/CloseEndpoint‌ای نمی‌زنیم —
	// endpoint همون یه‌بار توی OnSetupCompleted باز شده و همونجا می‌مونه.
	return true;
}

void DaisyUSBAMCInterface::OnSOF(void)
{
	if (m_InAltSetting == 1 && !m_StreamPrimed)
	{
		m_StreamPrimed = true;

		memset(m_TransmitBuffer, 0, GetConfigs().TransmitPacketSize);
		EndpointTransmit(m_TransmitBuffer, GetConfigs().TransmitPacketSize);
	}
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
	//USBInterfaceAssociationDescriptor* iad = reinterpret_cast<USBInterfaceAssociationDescriptor*>(buffer + BufferOffset);
	//iad->bLength = sizeof(USBInterfaceAssociationDescriptor);
	//iad->bDescriptorType = USBDescType::InterfaceAssociation;
	//iad->bFirstInterface = InterfaceIndex;
	//iad->bInterfaceCount = CalculateRequiredInterfaceCount(Config);
	//iad->bFunctionClass = USBSDeviceClass::Audio;
	//iad->bFunctionSubClass = AUDIO_SUBCLASS_CONTROL;
	//iad->bFunctionProtocol = 0x00; // UAC1: بدون کد پروتکل
	//iad->iFunction = 0;
	//BufferOffset += sizeof(USBInterfaceAssociationDescriptor);

	// ---------- AC Interface (standard) ----------
	USBInterfaceDescriptor* acIf = reinterpret_cast<USBInterfaceDescriptor*>(buffer + BufferOffset);
	acIf->bLength = sizeof(USBInterfaceDescriptor);
	acIf->bDescriptorType = USBDescType::Interface;
	acIf->bInterfaceNumber = InterfaceIndex;
	acIf->bAlternateSetting = 0;
	acIf->bNumEndpoints = 0;
	acIf->bInterfaceClass = USBSDeviceClass::Audio;
	acIf->bInterfaceSubClass = AUDIO_SUBCLASS_CONTROL;
	acIf->bInterfaceProtocol = 0x00;
	acIf->iInterface = 0;
	BufferOffset += sizeof(USBInterfaceDescriptor);

	// ---------- AC Header (متغیر-طول، بسته به تعداد AS interfaceها) ----------
	uint16 acDescStart = BufferOffset;

	uint8 asInterfaceNumbers[2];
	uint8 asInterfaceCount = 0;
	if (hasOutput) asInterfaceNumbers[asInterfaceCount++] = asOutInterface;
	if (hasInput)  asInterfaceNumbers[asInterfaceCount++] = asInInterface;

	uint8 headerLength = 8 + asInterfaceCount;
	{
		uint8* h = buffer + BufferOffset;
		h[0] = headerLength;
		h[1] = (uint8)USBDescType::CDCFunc;
		h[2] = AC_DESC_HEADER;
		h[3] = 0x00; h[4] = 0x01;       // bcdADC = 0x0100
		// h[5],h[6] = wTotalLength، پایین‌تر پر میشه
		h[7] = asInterfaceCount;         // bInCollection
		for (uint8 i = 0; i < asInterfaceCount; ++i)
			h[8 + i] = asInterfaceNumbers[i];

		BufferOffset += headerLength;
	}
	uint8* headerBytes = buffer + acDescStart; // برای پر کردن wTotalLength در پایان نگه می‌داریم

	// ---------- زنجیره‌ی خروجی (Playback: هاست -> پدال) ----------
	if (hasOutput)
	{
		UAC1InputTerminalDescriptor* it = reinterpret_cast<UAC1InputTerminalDescriptor*>(buffer + BufferOffset);
		it->bLength = sizeof(UAC1InputTerminalDescriptor);
		it->bDescriptorType = USBDescType::CDCFunc;
		it->bDescriptorSubtype = AC_DESC_INPUT_TERMINAL;
		it->bTerminalID = IT_USB_STREAMING_ID;
		it->wTerminalType = AUDIO_TERMINAL_USB_STREAMING;
		it->bAssocTerminal = 0;
		it->bNrChannels = Config.OutputChannelCount;
		it->wChannelConfig = (uint16)ChannelOutputPosition::Stereo;
		it->iChannelNames = 0;
		it->iTerminal = 0;
		BufferOffset += sizeof(UAC1InputTerminalDescriptor);

		// Feature Unit (متغیر-طول، bControlSize=1)
		{
			uint8 channelCount = Config.OutputChannelCount;
			uint8 fuLength = 7 + (channelCount + 1); // bControlSize=1 بایت به‌ازای هر کانال+master

			uint8* fuStart = buffer + BufferOffset;
			fuStart[0] = fuLength;
			fuStart[1] = (uint8)USBDescType::CDCFunc;
			fuStart[2] = AC_DESC_FEATURE_UNIT;
			fuStart[3] = FU_SPEAKER_ID;
			fuStart[4] = IT_USB_STREAMING_ID;
			fuStart[5] = 1; // bControlSize

			uint8 masterControls = 0;
			if (Config.EnableHardwareMute)   masterControls |= 0x01;
			if (Config.EnableHardwareVolumeControl) masterControls |= 0x02;

			fuStart[6] = masterControls; // کانال master
			for (uint8 ch = 1; ch <= channelCount; ++ch)
				fuStart[6 + ch] = 0; // بدون کنترل مستقل per-channel

			fuStart[6 + channelCount + 1] = 0; // iFeature

			BufferOffset += fuLength;
		}

		UAC1OutputTerminalDescriptor* ot = reinterpret_cast<UAC1OutputTerminalDescriptor*>(buffer + BufferOffset);
		ot->bLength = sizeof(UAC1OutputTerminalDescriptor);
		ot->bDescriptorType = USBDescType::CDCFunc;
		ot->bDescriptorSubtype = AC_DESC_OUTPUT_TERMINAL;
		ot->bTerminalID = OT_SPEAKER_ID;
		ot->wTerminalType = AUDIO_TERMINAL_SPEAKER;
		ot->bAssocTerminal = 0;
		ot->bSourceID = FU_SPEAKER_ID;
		ot->iTerminal = 0;
		BufferOffset += sizeof(UAC1OutputTerminalDescriptor);
	}

	// ---------- زنجیره‌ی ورودی (Capture: پدال -> هاست) ----------
	if (hasInput)
	{
		UAC1InputTerminalDescriptor* it = reinterpret_cast<UAC1InputTerminalDescriptor*>(buffer + BufferOffset);
		it->bLength = sizeof(UAC1InputTerminalDescriptor);
		it->bDescriptorType = USBDescType::CDCFunc;
		it->bDescriptorSubtype = AC_DESC_INPUT_TERMINAL;
		it->bTerminalID = IT_MIC_ID;
		it->wTerminalType = AUDIO_TERMINAL_MIC;
		it->bAssocTerminal = 0;
		it->bNrChannels = Config.InputChannelCount;
		it->wChannelConfig = (uint16)ChannelOutputPosition::Stereo;
		it->iChannelNames = 0;
		it->iTerminal = 0;
		BufferOffset += sizeof(UAC1InputTerminalDescriptor);

		{
			uint8 channelCount = Config.InputChannelCount;
			uint8 fuLength = 7 + (channelCount + 1);

			uint8* fuStart = buffer + BufferOffset;
			fuStart[0] = fuLength;
			fuStart[1] = (uint8)USBDescType::CDCFunc;
			fuStart[2] = AC_DESC_FEATURE_UNIT;
			fuStart[3] = FU_MIC_ID;
			fuStart[4] = IT_MIC_ID;
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
		ot->bDescriptorType = USBDescType::CDCFunc;
		ot->bDescriptorSubtype = AC_DESC_OUTPUT_TERMINAL;
		ot->bTerminalID = OT_USB_STREAMING_ID;
		ot->wTerminalType = AUDIO_TERMINAL_USB_STREAMING;
		ot->bAssocTerminal = 0;
		ot->bSourceID = FU_MIC_ID;
		ot->iTerminal = 0;
		BufferOffset += sizeof(UAC1OutputTerminalDescriptor);
	}

	// حالا که کل بلاک AC ساخته شد، wTotalLength رو پر می‌کنیم
	uint16 totalLen = BufferOffset - acDescStart;
	headerBytes[5] = (uint8)(totalLen & 0xFF);
	headerBytes[6] = (uint8)((totalLen >> 8) & 0xFF);

	// ---------- AS Interface خروجی ----------
	if (hasOutput)
		BuildStreamingInterface(EP0Buffer, BufferOffset, asOutInterface, Config.OutputChannelCount, GetConfigs().EndpointOut, IT_USB_STREAMING_ID, Config, true);

	// ---------- AS Interface ورودی ----------
	if (hasInput)
		BuildStreamingInterface(EP0Buffer, BufferOffset, asInInterface, Config.InputChannelCount, GetConfigs().EndpointIn, OT_USB_STREAMING_ID, Config, false);
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
	const uint8 defaultBitDepths = Config.SupportedBitDepths[Config.DefaultBitDepthIndex];
	uint8 subslotSize = (uint8)(defaultBitDepths / 8);

	// --- Alt Setting 0: zero-bandwidth ---
	USBInterfaceDescriptor* alt0 = reinterpret_cast<USBInterfaceDescriptor*>(buffer + BufferOffset);
	alt0->bLength = sizeof(USBInterfaceDescriptor);
	alt0->bDescriptorType = USBDescType::Interface;
	alt0->bInterfaceNumber = InterfaceIndex;
	alt0->bAlternateSetting = 0;
	alt0->bNumEndpoints = 0;
	alt0->bInterfaceClass = USBSDeviceClass::Audio;
	alt0->bInterfaceSubClass = AUDIO_SUBCLASS_STREAMING;
	alt0->bInterfaceProtocol = 0x00;
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
	alt1->bInterfaceProtocol = 0x00;
	alt1->iInterface = 0;
	BufferOffset += sizeof(USBInterfaceDescriptor);

	UAC1StreamingInterfaceDescriptor* asGeneral = reinterpret_cast<UAC1StreamingInterfaceDescriptor*>(buffer + BufferOffset);
	asGeneral->bLength = sizeof(UAC1StreamingInterfaceDescriptor);
	asGeneral->bDescriptorType = USBDescType::CDCFunc;
	asGeneral->bDescriptorSubtype = AS_DESC_GENERAL;
	asGeneral->bTerminalLink = TerminalLinkID;
	asGeneral->bDelay = 0;
	asGeneral->wFormatTag = AUDIO_FORMAT_TAG_PCM;
	BufferOffset += sizeof(UAC1StreamingInterfaceDescriptor);

	// Format Type I (متغیر-طول: لیست سمپل‌ریت‌ها مستقیم اینجا)
	{
		uint8 rateCount = Config.SupportedSampleRateCount;
		uint8 formatLength = 8 + (rateCount * 3);

		uint8* f = buffer + BufferOffset;
		f[0] = formatLength;
		f[1] = (uint8)USBDescType::CDCFunc;
		f[2] = AS_DESC_FORMAT_TYPE;
		f[3] = AUDIO_FORMAT_TYPE_I;
		f[4] = ChannelCount;
		f[5] = subslotSize;
		f[6] = defaultBitDepths;
		f[7] = rateCount; // bSamFreqType: گسسته، نه continuous

		for (uint8 i = 0; i < rateCount; ++i)
		{
			uint32 rate = Config.SupportedSampleRates[i];
			f[8 + i * 3 + 0] = (uint8)(rate & 0xFF);
			f[8 + i * 3 + 1] = (uint8)((rate >> 8) & 0xFF);
			f[8 + i * 3 + 2] = (uint8)((rate >> 16) & 0xFF);
		}

		BufferOffset += formatLength;
	}

	uint16 packetSize = ChannelCount * subslotSize * (uint16)(defaultSampleRate / 1000);

	USBEndpointDescriptor* ep = reinterpret_cast<USBEndpointDescriptor*>(buffer + BufferOffset);
	ep->bLength = sizeof(USBEndpointDescriptor);
	ep->bDescriptorType = USBDescType::Endpoint;
	ep->bEndpointAddress = Endpoint;
	ep->bmAttributes = (USBEpAttr)((uint8)USBEpAttr::Isochronous | USB_EP_SYNC_TYPE_ASYNC);
	ep->wMaxPacketSize = packetSize;
	ep->bInterval = 1;
	BufferOffset += sizeof(USBEndpointDescriptor);

	UAC1IsoEndpointDescriptor* isoDesc = reinterpret_cast<UAC1IsoEndpointDescriptor*>(buffer + BufferOffset);
	isoDesc->bLength = sizeof(UAC1IsoEndpointDescriptor);
	isoDesc->bDescriptorType = USBDescType::CDCEndpointFunc;
	isoDesc->bDescriptorSubtype = AS_DESC_EP_GENERAL;
	isoDesc->bmAttributes = 0; // این باید مطابق درخواست‌های واقعی هاست باشه
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

	const uint8 defaultBitDepths = Class.SupportedBitDepths[Class.DefaultBitDepthIndex];
	const uint32 defaultSampleRate = Class.SupportedSampleRates[Class.DefaultSampleRateIndex];

	uint32 bytesPerFrame = ChannelCount * (defaultBitDepths / 8);
	uint32 bytesPerPacket = bytesPerFrame * (defaultSampleRate / 1000);

	ASSERT(bytesPerPacket <= 1023, "Packet size exceeds Full-Speed isochronous limit");

	return (uint16)bytesPerPacket;
}

#endif