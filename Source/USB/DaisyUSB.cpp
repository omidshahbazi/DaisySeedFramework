#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSB.h"
#include <DigitalSignalProcessing/Debug.h>
#include <DigitalSignalProcessing/StringUtils.h>
#include <DigitalSignalProcessing/Math.h>
#include <DigitalSignalProcessing/Memory.h>

static DaisyUSB* s_Instance[(uint8)DaisyUSB::Peripherals::COUNT] = {};

extern "C"
{
	void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle)
	{
		GPIO_InitTypeDef GPIO_InitStruct = { 0 };
		if (pcdHandle->Instance == USB_OTG_FS)
		{
			/* USER CODE BEGIN USB_OTG_FS_MspInit 0 */

			/* USER CODE END USB_OTG_FS_MspInit 0 */

			__HAL_RCC_GPIOA_CLK_ENABLE();
			/**USB_OTG_FS GPIO Configuration
			PA12     ------> USB_OTG_FS_DP
			PA11     ------> USB_OTG_FS_DM
			PA9     ------> USB_OTG_FS_VBUS
			*/
			GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

			GPIO_InitStruct.Pin = GPIO_PIN_9;
			GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

			/* Peripheral clock enable */
			__HAL_RCC_USB_OTG_FS_CLK_ENABLE();

			/* Peripheral interrupt init */
			HAL_NVIC_SetPriority(OTG_FS_EP1_OUT_IRQn, 0, 0);
			HAL_NVIC_EnableIRQ(OTG_FS_EP1_OUT_IRQn);
			HAL_NVIC_SetPriority(OTG_FS_EP1_IN_IRQn, 0, 0);
			HAL_NVIC_EnableIRQ(OTG_FS_EP1_IN_IRQn);
			HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 0);
			HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
			/* USER CODE BEGIN USB_OTG_FS_MspInit 1 */

			/* USER CODE END USB_OTG_FS_MspInit 1 */
		}
		else if (pcdHandle->Instance == USB_OTG_HS)
		{
			/* USER CODE BEGIN USB_OTG_HS_MspInit 0 */

			/* USER CODE END USB_OTG_HS_MspInit 0 */

			__HAL_RCC_GPIOB_CLK_ENABLE();
			/**USB_OTG_HS GPIO Configuration
			PB14     ------> USB_OTG_HS_DM
			PB15     ------> USB_OTG_HS_DP
			*/
			GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF12_OTG2_FS;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

			/* Peripheral clock enable */
			__HAL_RCC_USB_OTG_HS_CLK_ENABLE();

			/* Peripheral interrupt init */
			HAL_NVIC_SetPriority(OTG_HS_EP1_OUT_IRQn, 0, 0);
			HAL_NVIC_EnableIRQ(OTG_HS_EP1_OUT_IRQn);
			HAL_NVIC_SetPriority(OTG_HS_EP1_IN_IRQn, 0, 0);
			HAL_NVIC_EnableIRQ(OTG_HS_EP1_IN_IRQn);
			HAL_NVIC_SetPriority(OTG_HS_IRQn, 0, 0);
			HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
			/* USER CODE BEGIN USB_OTG_HS_MspInit 1 */

			/* USER CODE END USB_OTG_HS_MspInit 1 */
		}
	}

	void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle)
	{
		if (pcdHandle->Instance == USB_OTG_FS)
		{
			/* USER CODE BEGIN USB_OTG_FS_MspDeInit 0 */

			/* USER CODE END USB_OTG_FS_MspDeInit 0 */
			/* Peripheral clock disable */
			__HAL_RCC_USB_OTG_FS_CLK_DISABLE();

			/**USB_OTG_FS GPIO Configuration
			PA12     ------> USB_OTG_FS_DP
			PA11     ------> USB_OTG_FS_DM
			PA9     ------> USB_OTG_FS_VBUS
			*/
			HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12 | GPIO_PIN_11 | GPIO_PIN_9);

			/* Peripheral interrupt Deinit*/
			HAL_NVIC_DisableIRQ(OTG_FS_EP1_OUT_IRQn);

			HAL_NVIC_DisableIRQ(OTG_FS_EP1_IN_IRQn);

			HAL_NVIC_DisableIRQ(OTG_FS_IRQn);

			/* USER CODE BEGIN USB_OTG_FS_MspDeInit 1 */

			/* USER CODE END USB_OTG_FS_MspDeInit 1 */
		}
		else if (pcdHandle->Instance == USB_OTG_HS)
		{
			/* USER CODE BEGIN USB_OTG_HS_MspDeInit 0 */

			/* USER CODE END USB_OTG_HS_MspDeInit 0 */
			/* Peripheral clock disable */
			__HAL_RCC_USB_OTG_HS_CLK_DISABLE();

			/**USB_OTG_HS GPIO Configuration
			PB14     ------> USB_OTG_HS_DM
			PB15     ------> USB_OTG_HS_DP
			*/
			HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14 | GPIO_PIN_15);

			/* Peripheral interrupt Deinit*/
			HAL_NVIC_DisableIRQ(OTG_HS_EP1_OUT_IRQn);

			HAL_NVIC_DisableIRQ(OTG_HS_EP1_IN_IRQn);

			HAL_NVIC_DisableIRQ(OTG_HS_IRQn);

			/* USER CODE BEGIN USB_OTG_HS_MspDeInit 1 */

			/* USER CODE END USB_OTG_HS_MspDeInit 1 */
		}
	}

	void OTG_FS_IRQHandler(void)
	{
		s_Instance[(uint8)DaisyUSB::Peripherals::FullSpeed]->OnHALHandleRequest();
	}

	void OTG_HS_IRQHandler(void)
	{
		s_Instance[(uint8)DaisyUSB::Peripherals::HighSpeed]->OnHALHandleRequest();
	}

	void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef* hpcd)
	{
		for (uint8_t i = 0; i < (uint8_t)DaisyUSB::Peripherals::COUNT; ++i)
		{
			if (s_Instance[i] == nullptr)
				continue;

			if (&s_Instance[i]->m_DeviceHandle != hpcd)
				continue;

			s_Instance[i]->OnSetupStage();

			break;
		}
	}

	void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
	{
		for (uint8_t i = 0; i < (uint8_t)DaisyUSB::Peripherals::COUNT; ++i)
		{
			if (s_Instance[i] == nullptr)
				continue;

			if (&s_Instance[i]->m_DeviceHandle != hpcd)
				continue;

			s_Instance[i]->OnDataInStage(epnum);

			break;
		}
	}

	void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
	{
		for (uint8_t i = 0; i < (uint8_t)DaisyUSB::Peripherals::COUNT; ++i)
		{
			if (s_Instance[i] == nullptr)
				continue;

			if (&s_Instance[i]->m_DeviceHandle != hpcd)
				continue;

			s_Instance[i]->OnDataOutStage(epnum);

			break;
		}
	}
}

#define CHECK_CALL(Expr) ASSERT((Expr) == HAL_OK, #Expr);

DaisyUSB::DaisyUSB(Peripherals Peripheral)
	: m_Peripheral(Peripheral),
	m_IsRunning(false),
	m_EO0TransmitRemainingLength(0),
	m_EP0TransmitBufferStart(nullptr),
	m_DeviceCount(0)
{
	s_Instance[(uint8)Peripheral] = this;
}

void DaisyUSB::Start(const USBProfile& Profile)
{
	ASSERT(!m_IsRunning, "Interface has already started.");
	ASSERT(Profile.Mode != USBModes::Device || Profile.Device.ClassNodeCount <= MaxClassCount, "Invalid ClassNodeCount");
	ASSERT(Profile.Mode != USBModes::Host || Profile.Host.SupportedClassCount <= MaxClassCount, "Invalid SupportedClassCount");

	m_Profile = Profile;

	if (Profile.Mode == USBModes::Device)
	{
		uint8 interfaceCounter = 0;
		for (uint8 i = 0; i < Profile.Device.ClassNodeCount; ++i)
		{
			const USBClassNode& node = Profile.Device.ClassNodes[i];

			DeviceInstanceInfo& dii = m_Devices[m_DeviceCount++];
			dii.Class = node.Class;

			switch (dii.Class)
			{
			case USBDeviceClassses::CDC:
			{
				uint16 interfaceIndexMask = (1 << interfaceCounter) | (1 << (interfaceCounter + 1));

				dii.Interface = Memory::Allocate<DaisyUSBCDCInterface>(1, true);
				new (dii.Interface) DaisyUSBCDCInterface(this, interfaceIndexMask, node.CDC.CustomEndpointCommand, node.CDC.CustomEndpointIn, node.CDC.CustomEndpointOut);

				interfaceCounter += DaisyUSBCDCInterface::RequiredInterfaceCount;

				break;
			}

			case USBDeviceClassses::AMC:
			{
				dii.Interface = Memory::Allocate<DaisyUSBAMCInterface>(1, true);
				//new (dii.Interface) DaisyUSBAMCInterface(this, interfaceIndexMask, node.CDC.CustomEndpointIn, node.CDC.CustomEndpointOut);

				interfaceCounter += DaisyUSBAMCInterface::RequiredInterfaceCount;

				break;
			}
			}
		}
	}
	else
	{
	}

	if (m_Profile.Mode == USBModes::Device)
	{
		if (m_Peripheral == Peripherals::HighSpeed)
		{
			m_DeviceHandle.Instance = USB_OTG_HS;
			m_DeviceHandle.Init.dev_endpoints = USB_EP_COUNT_DEFAULT;
			m_DeviceHandle.Init.speed = PCD_SPEED_FULL;
			m_DeviceHandle.Init.phy_itface = PCD_PHY_EMBEDDED;
		}
		else
		{
			m_DeviceHandle.Instance = USB_OTG_FS;
			m_DeviceHandle.Init.dev_endpoints = USB_EP_COUNT_DEFAULT;
			m_DeviceHandle.Init.speed = PCD_SPEED_FULL;
			m_DeviceHandle.Init.phy_itface = PCD_PHY_EMBEDDED;
		}

		m_DeviceHandle.Init.Sof_enable = DISABLE;
		m_DeviceHandle.Init.low_power_enable = DISABLE;
		m_DeviceHandle.Init.lpm_enable = DISABLE;
		m_DeviceHandle.Init.vbus_sensing_enable = DISABLE;

		CHECK_CALL(HAL_PCD_Init(&m_DeviceHandle));

		OpenEndpoint(USB_EP0_OUT, MaxPacketSize, USBEpAttr::Control);
		OpenEndpoint(USB_EP0_IN, MaxPacketSize, USBEpAttr::Control);

		CHECK_CALL(HAL_PCD_Start(&m_DeviceHandle));
	}
	else
	{
		if (m_Peripheral == Peripherals::HighSpeed)
		{
			m_HostHandle.Instance = USB_OTG_HS;
			m_HostHandle.Init.speed = HCD_SPEED_HIGH;
			m_HostHandle.Init.phy_itface = PCD_PHY_ULPI;
		}
		else
		{
			m_HostHandle.Instance = USB_OTG_FS;
			m_HostHandle.Init.speed = HCD_SPEED_FULL;
			m_HostHandle.Init.phy_itface = PCD_PHY_EMBEDDED;
		}

		m_HostHandle.Init.Host_channels = USB_HCD_CHANNELS_DEFAULT;
		m_HostHandle.Init.Sof_enable = DISABLE;
		m_HostHandle.Init.vbus_sensing_enable = DISABLE;

		CHECK_CALL(HAL_HCD_Init(&m_HostHandle));
		CHECK_CALL(HAL_HCD_Start(&m_HostHandle));
	}

	m_IsRunning = true;
}

void DaisyUSB::Stop(void)
{
	ASSERT(m_IsRunning, "Interface is not started.");

	if (m_Profile.Mode == USBModes::Device)
	{
		CHECK_CALL(HAL_PCD_Stop(&m_DeviceHandle));
		CHECK_CALL(HAL_PCD_DeInit(&m_DeviceHandle));
	}
	else
	{
		CHECK_CALL(HAL_HCD_Stop(&m_HostHandle));
		CHECK_CALL(HAL_HCD_DeInit(&m_HostHandle));
	}

	m_IsRunning = false;
}

void DaisyUSB::OnHALHandleRequest(void)
{
	if (m_Profile.Mode == USBModes::Device)
		HAL_PCD_IRQHandler(&m_DeviceHandle);
	else
		HAL_HCD_IRQHandler(&m_HostHandle);
}

void DaisyUSB::OnSetupStage(void)
{
	const USBDeviceSetupPacket* setup = reinterpret_cast<const USBDeviceSetupPacket*>(m_DeviceHandle.Setup);

	uint8 reqType = setup->bmRequestType & USB_REQ_TYPE_MASK;

	if (reqType == USB_REQ_TYPE_STANDARD)
	{
		switch (setup->bRequest)
		{
		case USB_REQ_GET_DESCRIPTOR:
			HandleGetDescriptor();
			break;

		case USB_REQ_SET_ADDRESS:
		{
			uint8 devAddr = static_cast<uint8>(setup->wValue & 0x7F);

			CHECK_CALL(HAL_PCD_SetAddress(&m_DeviceHandle, devAddr));

			DeviceTransmitAck();

			break;
		}

		case USB_REQ_SET_CONFIGURATION:
		{
			for (uint8 i = 0; i < m_DeviceCount; ++i)
				m_Devices[i].Interface->OnReady();

			DeviceTransmitAck();
			break;
		}

		default:
			SetStall();
			break;
		}
	}
	else if (reqType == USB_REQ_TYPE_CLASS)
	{
		DeviceInstanceInfo& dii = GetDeviceInstanceByIndex(setup->wIndex);

		bool handled = false;
		switch (dii.Class)
		{
		case USBDeviceClassses::CDC:
			handled = reinterpret_cast<DaisyUSBCDCInterface*>(dii.Interface)->OnSetupStage(setup);
			break;

		case USBDeviceClassses::AMC:
			//handled = reinterpret_cast<DaisyUSBMCInterface*>(dii.Interface)->OnSetupStage(setup);
			break;
		}

		if (!handled)
			SetStall();
	}
}

void DaisyUSB::OnDataInStage(uint8 EPNum)
{
	if (EPNum == 0 || EPNum == (USB_EP0_IN & 0x7F))
	{
		if (m_EO0TransmitRemainingLength > 0)
		{
			uint16 chunkLen = (m_EO0TransmitRemainingLength < MaxPacketSize) ? m_EO0TransmitRemainingLength : MaxPacketSize;

			const uint8* ptr = m_EP0TransmitBufferStart;
			m_EP0TransmitBufferStart += chunkLen;
			m_EO0TransmitRemainingLength -= chunkLen;

			DeviceTransmit(const_cast<uint8*>(ptr), chunkLen);
		}
		else
			DeviceReceiveAck();

		return;
	}
}

void DaisyUSB::OnDataOutStage(uint8 EPNum)
{
	if (EPNum == 0 || EPNum == (USB_EP0_IN & 0x7F))
		return;

	DeviceInstanceInfo& dii = GetDeviceInstanceByEndpoint(EPNum);
	switch (dii.Class)
	{
	case USBDeviceClassses::CDC:
		reinterpret_cast<DaisyUSBCDCInterface*>(dii.Interface)->OnDataOutStage();
		break;

	case USBDeviceClassses::AMC:
		reinterpret_cast<DaisyUSBCDCInterface*>(dii.Interface)->OnDataOutStage();
		break;
	}
}

void DaisyUSB::HandleGetDescriptor(void)
{
	static EP0Buffer ep0Buffers[(uint8)DaisyUSB::Peripherals::COUNT] = {};

	const USBDeviceSetupPacket* setup = reinterpret_cast<const USBDeviceSetupPacket*>(m_DeviceHandle.Setup);

	EP0Buffer& ep0Buffer = ep0Buffers[(uint8)m_Peripheral];

	uint8 descType = static_cast<uint8>(setup->wValue >> 8);
	uint8 descIndex = static_cast<uint8>(setup->wValue & 0xFF);
	uint16 sendLen = 0;

	switch (static_cast<USBDescType>(descType))
	{
	case USBDescType::Device:
		sendLen = BuildDeviceDescriptor(ep0Buffer, m_Profile);
		break;

	case USBDescType::Configuration:
		sendLen = BuildConfigurationDescriptor(ep0Buffer, (m_Peripheral == Peripherals::FullSpeed ? USB_EP_INTERVAL_FS : USB_EP_INTERVAL_HS), m_Profile.Device);
		break;

	case USBDescType::String:
	{
		if (descIndex == USB_STRING_INDEX_LANGID)
		{
			ep0Buffer.stringDesc.bLength = 4;
			ep0Buffer.stringDesc.bDescriptorType = USBDescType::String;
			ep0Buffer.stringDesc.wData[0] = USB_LANGID_ENGLISH_US;

			sendLen = 4;
		}
		else if (descIndex == USB_STRING_INDEX_MANUFACTURER)
			sendLen = BuildStringDescriptor(ep0Buffer, m_Profile.Device.Manufacturer);
		else if (descIndex == USB_STRING_INDEX_PRODUCT)
			sendLen = BuildStringDescriptor(ep0Buffer, m_Profile.Device.Product);
		else if (descIndex == USB_STRING_INDEX_SERIAL)
			sendLen = BuildStringDescriptor(ep0Buffer, m_Profile.Device.SerialNumber);
		break;
	}

	default:
		SetStall();
		return;
	}

	if (sendLen > 0)
	{
		uint16 actualLen = (sendLen < setup->wLength) ? sendLen : setup->wLength;

		if (actualLen > MaxPacketSize)
		{
			m_EP0TransmitBufferStart = reinterpret_cast<const uint8*>(&ep0Buffer) + MaxPacketSize;
			m_EO0TransmitRemainingLength = actualLen - MaxPacketSize;
			actualLen = MaxPacketSize;
		}
		else
		{
			m_EP0TransmitBufferStart = nullptr;
			m_EO0TransmitRemainingLength = 0;
		}

		DeviceTransmit(reinterpret_cast<uint8*>(&ep0Buffer), actualLen);
	}
}

void DaisyUSB::OpenEndpoint(uint8 Endpoint, uint16 Length, USBEpAttr Type)
{
	CHECK_CALL(HAL_PCD_EP_Open(&m_DeviceHandle, Endpoint, Length, (uint8)Type));
}

uint16 DaisyUSB::DeviceReceiveCount(uint8 Endpoint)
{
	return HAL_PCD_EP_GetRxCount(&m_DeviceHandle, Endpoint);
}

void DaisyUSB::DeviceReceive(uint8* Buffer, uint16 Length, uint8 Endpoint)
{
	CHECK_CALL(HAL_PCD_EP_Receive(&m_DeviceHandle, Endpoint, Buffer, Length));
}

void DaisyUSB::DeviceTransmit(uint8* Buffer, uint16 Length, uint8 Endpoint)
{
	CHECK_CALL(HAL_PCD_EP_Transmit(&m_DeviceHandle, Endpoint, Buffer, Length));
}

void DaisyUSB::SetStall(void)
{
	CHECK_CALL(HAL_PCD_EP_SetStall(&m_DeviceHandle, USB_EP0_OUT));
}

DaisyUSB::DeviceInstanceInfo& DaisyUSB::GetDeviceInstanceByIndex(uint8 InterfaceIndex)
{
	for (uint8 i = 0; i < m_DeviceCount; ++i)
	{
		if (!m_Devices[i].Interface->MatchByInterfaceIndex(InterfaceIndex))
			continue;

		return m_Devices[i];
	}

	ASSERT(false, "Couldn't find the proper interface");
}

DaisyUSB::DeviceInstanceInfo& DaisyUSB::GetDeviceInstanceByEndpoint(uint8 Endpoint)
{
	for (uint8 i = 0; i < m_DeviceCount; ++i)
	{
		if (!m_Devices[i].Interface->MatchByEndpoint(Endpoint))
			continue;

		return m_Devices[i];
	}

	ASSERT(false, "Couldn't find the proper interface");
}

uint16 DaisyUSB::BuildDeviceDescriptor(EP0Buffer& EP0Buffer, const USBProfile& Profile)
{
	EP0Buffer.deviceDesc.bLength = sizeof(USBDeviceDescriptor);
	EP0Buffer.deviceDesc.bDescriptorType = USBDescType::Device;
	EP0Buffer.deviceDesc.bcdUSB = USB_VERSION_2_0;
	EP0Buffer.deviceDesc.bDeviceClass = USBSDeviceClass::Misc;
	EP0Buffer.deviceDesc.bDeviceSubClass = USBDeviceSubClass::Common;
	EP0Buffer.deviceDesc.bDeviceProtocol = USBDeviceProtocol::IAD;
	EP0Buffer.deviceDesc.bMaxPacketSize0 = MaxPacketSize;
	EP0Buffer.deviceDesc.idVendor = Profile.Device.VendorID;
	EP0Buffer.deviceDesc.idProduct = Profile.Device.ProductID;
	EP0Buffer.deviceDesc.bcdDevice = Profile.Device.Version;
	EP0Buffer.deviceDesc.iManufacturer = USB_STRING_INDEX_MANUFACTURER;
	EP0Buffer.deviceDesc.iProduct = USB_STRING_INDEX_PRODUCT;
	EP0Buffer.deviceDesc.iSerialNumber = USB_STRING_INDEX_SERIAL;
	EP0Buffer.deviceDesc.bNumConfigurations = USB_CONFIG_VALUE_DEFAULT;

	return sizeof(USBDeviceDescriptor);
}

uint16 DaisyUSB::BuildStringDescriptor(EP0Buffer& EP0Buffer, cstr Value)
{
	if (!Value)
		return 0;

	uint8 length = Math::Max(USBMaxStringLength, GetStringLength(Value));
	for (uint8 i = 0; i < length; ++i)
		EP0Buffer.stringDesc.wData[i] = Value[i];

	uint8 totalLength = __offsetof(USBStringDescriptor, wData) + (length * sizeof(uint16));
	EP0Buffer.stringDesc.bLength = totalLength;
	EP0Buffer.stringDesc.bDescriptorType = USBDescType::String;

	return totalLength;
}

uint16 DaisyUSB::BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint8 Interval, const USBDeviceProfile& Profile)
{
	uint8* buffer = EP0Buffer.configDescs;
	uint16 offset = 0;

	USBConfigurationDescriptor* config = reinterpret_cast<USBConfigurationDescriptor*>(buffer + offset);
	offset += sizeof(USBConfigurationDescriptor);

	uint8 interfaceCounter = 0;
	for (uint8 i = 0; i < Profile.ClassNodeCount; ++i)
	{
		const USBClassNode& node = Profile.ClassNodes[i];

		if (node.Class == USBDeviceClassses::CDC)
		{
			DaisyUSBCDCInterface::BuildConfigurationDescriptor(EP0Buffer, offset, interfaceCounter, Interval, node.CDC);

			interfaceCounter += DaisyUSBCDCInterface::RequiredInterfaceCount;
		}
		else if (node.Class == USBDeviceClassses::AMC)
		{
		}
	}

	config->bLength = sizeof(USBConfigurationDescriptor);
	config->bDescriptorType = USBDescType::Configuration;
	config->wTotalLength = offset;
	config->bNumInterfaces = interfaceCounter;
	config->bConfigurationValue = USB_CONFIG_VALUE_DEFAULT;
	config->iConfiguration = 0;
	config->bmAttributes = Profile.IsSelfPowered ? USBConfigAttr::SelfPoweredMask : USBConfigAttr::BusPoweredMask;
	config->bMaxPower = Profile.MaxPowerCurrent / 2;

	return offset;
}

#endif