#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBDevice.h"
#include "DaisySeedFramework/StringUtils.h"
#include <DigitalSignalProcessing/Debug.h>
#include <DigitalSignalProcessing/Math.h>
#include <DigitalSignalProcessing/Memory.h>

static DaisyUSBDevice* s_Instance[(uint8_t)Peripherals::COUNT] = {};

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

	void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef* hpcd)
	{
		for (uint8_t i = 0; i < (uint8_t)Peripherals::COUNT; ++i)
		{
			if (s_Instance[i] == nullptr)
				continue;

			if (&s_Instance[i]->m_DeviceHandle != hpcd)
				continue;

			s_Instance[i]->OnSetupStage();

			break;
		}
	}

	void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
	{
		for (uint8_t i = 0; i < (uint8_t)Peripherals::COUNT; ++i)
		{
			if (s_Instance[i] == nullptr)
				continue;

			if (&s_Instance[i]->m_DeviceHandle != hpcd)
				continue;

			s_Instance[i]->OnDataOutStage(epnum);

			break;
		}
	}

	void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
	{
		for (uint8_t i = 0; i < (uint8_t)Peripherals::COUNT; ++i)
		{
			if (s_Instance[i] == nullptr)
				continue;

			if (&s_Instance[i]->m_DeviceHandle != hpcd)
				continue;

			s_Instance[i]->OnDataInStage(epnum);

			break;
		}
	}

	void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
	{
		for (uint8_t i = 0; i < (uint8_t)Peripherals::COUNT; ++i)
		{
			if (s_Instance[i] == nullptr)
				continue;

			if (&s_Instance[i]->m_DeviceHandle != hpcd)
				continue;

			s_Instance[i]->OnIsoOutIncomplete(epnum);
			break;
		}
	}

	void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
	{
		for (uint8_t i = 0; i < (uint8_t)Peripherals::COUNT; ++i)
		{
			if (s_Instance[i] == nullptr)
				continue;

			if (&s_Instance[i]->m_DeviceHandle != hpcd)
				continue;

			s_Instance[i]->OnIsoInIncomplete(epnum);
			break;
		}
	}

	void HAL_PCD_SOFCallback(PCD_HandleTypeDef* hpcd)
	{
		for (uint8_t i = 0; i < (uint8_t)Peripherals::COUNT; ++i)
		{
			if (s_Instance[i] == nullptr)
				continue;

			if (&s_Instance[i]->m_DeviceHandle != hpcd)
				continue;

			s_Instance[i]->OnStartOfFrame();
			break;
		}
	}
}

#define CHECK_CALL(Expr) ASSERT((Expr) == HAL_OK, #Expr);

DaisyUSBDevice::DaisyUSBDevice(Peripherals Peripheral)
	: m_Peripheral(Peripheral),
	m_IsRunning(false),
	m_EP0TransmitHandler((uint16_t)PacketSizes::Max),
	m_DeviceCount(0)
{
	s_Instance[(uint8_t)Peripheral] = this;
}

void DaisyUSBDevice::Update(void)
{
	for (uint8_t i = 0; i < m_DeviceCount; ++i)
		m_Devices[i].Interface->Update();
}

void DaisyUSBDevice::Start(const USBDeviceProfile& Profile)
{
	ASSERT(!m_IsRunning, "Interface has already started.");
	ASSERT(0 < Profile.ClassNodeCount && Profile.ClassNodeCount <= MaxClassCount, "Invalid ClassNodeCount");

	m_Profile = Profile;

	uint8_t nextEndpoint = 1;
	uint8_t interfaceIndex = 0;

	for (uint8_t i = 0; i < Profile.ClassNodeCount; ++i)
	{
		const USBClassNode& node = Profile.ClassNodes[i];

		DeviceInstanceInfo& dii = m_Devices[m_DeviceCount++];
		dii.Class = node.Class;

		DaisyUSBInterfaceCommon::Configs configs = {};
		configs.InterfaceIndexStart = interfaceIndex;

		switch (dii.Class)
		{
		case USBDeviceClasses::CDC:
		{
			configs.InterfaceIndexCount = DaisyUSBCDCInterface::CalculateRequiredInterfaceCount(node.CDC);

			configs.EndpointCommand = TO_IN_ENDPOINT(nextEndpoint);
			nextEndpoint++;
			configs.EndpointOut = TO_OUT_ENDPOINT(nextEndpoint);
			configs.EndpointIn = TO_IN_ENDPOINT(nextEndpoint);
			nextEndpoint++;

			configs.MaxReceivePacketSize = (uint16_t)node.CDC.ReceiveBufferSize;
			configs.MaxTransmitPacketSize = (uint16_t)node.CDC.SendBufferSize;

			DaisyUSBCDCInterface* cdc = Memory::Allocate<DaisyUSBCDCInterface>(1, true);
			new (cdc) DaisyUSBCDCInterface(this, configs, node.CDC);
			dii.Interface = cdc;

			break;
		}

		case USBDeviceClasses::AMC:
		{
			ASSERT(node.AMC.OutputChannelCount != 0 || node.AMC.InputChannelCount != 0, "AMC needs at least one channel");
			ASSERT(node.AMC.EnableHardwareMute || node.AMC.EnableHardwareVolumeControl, "Either of EnableHardwareMute or EnableHardwareVolumeControl must be true");
			ASSERT(node.AMC.SupportedSampleRateCount != 0, "AMC needs at least one supported sample rate");
			ASSERT(node.AMC.SupportedBitDepthCount != 0, "AMC needs at least one supported sample rate");

			// LINE1_TODO: Add support for multiple bit depths. For now, we only support one bit depth. (Requires alt setting change)
			ASSERT(node.AMC.SupportedBitDepthCount == 1, "AMC only supports one bit depth for now");

			configs.InterfaceIndexCount = DaisyUSBAMCInterface::CalculateRequiredInterfaceCount(node.AMC);

			configs.EndpointCommand = 0;
			configs.EndpointOut = (node.AMC.OutputChannelCount == 0 ? 0 : TO_OUT_ENDPOINT(nextEndpoint));
			configs.EndpointIn = (node.AMC.InputChannelCount == 0 ? 0 : TO_IN_ENDPOINT(nextEndpoint));
			nextEndpoint++;

			configs.MaxReceivePacketSize = DaisyUSBAMCInterface::CalculateMaxPacketSize(node.AMC.OutputChannelCount, node.AMC, false);
			configs.MaxTransmitPacketSize = DaisyUSBAMCInterface::CalculateMaxPacketSize(node.AMC.InputChannelCount, node.AMC, true);

			DaisyUSBAMCInterface* amc = Memory::Allocate<DaisyUSBAMCInterface>(1, true);
			new (amc) DaisyUSBAMCInterface(this, configs, node.AMC);
			dii.Interface = amc;

			break;
		}
		}

		interfaceIndex += configs.InterfaceIndexCount;
	}

	if (m_Peripheral == Peripherals::HighSpeed)
		m_DeviceHandle.Instance = USB_OTG_HS;
	else
		m_DeviceHandle.Instance = USB_OTG_FS;

	m_DeviceHandle.Init.dev_endpoints = USB_EP_COUNT_DEFAULT;
	m_DeviceHandle.Init.speed = PCD_SPEED_FULL;
	m_DeviceHandle.Init.phy_itface = PCD_PHY_EMBEDDED;

	m_DeviceHandle.Init.Sof_enable = ENABLE;
	m_DeviceHandle.Init.low_power_enable = DISABLE;
	m_DeviceHandle.Init.lpm_enable = DISABLE;
	m_DeviceHandle.Init.vbus_sensing_enable = DISABLE;
	m_DeviceHandle.Init.dma_enable = DISABLE;
	m_DeviceHandle.Init.battery_charging_enable = DISABLE;
	m_DeviceHandle.Init.use_dedicated_ep1 = DISABLE;

	CHECK_CALL(HAL_PCD_Init(&m_DeviceHandle));

	AllocateReceiveBuffer(512);
	AllocateTransmitBuffer(USB_EP0_IN, (uint16_t)PacketSizes::Max);

	OpenEndpoint(USB_EP0_OUT, (uint16_t)PacketSizes::Max, USBEndpointAttributes::Control);
	OpenEndpoint(USB_EP0_IN, (uint16_t)PacketSizes::Max, USBEndpointAttributes::Control);

	CHECK_CALL(HAL_PCD_Start(&m_DeviceHandle));

	if (m_Peripheral == Peripherals::HighSpeed)
	{
		HAL_NVIC_SetPriority(OTG_HS_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
	}
	else
	{
		HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
	}

	m_IsRunning = true;
}

void DaisyUSBDevice::Stop(void)
{
	ASSERT(m_IsRunning, "Interface is not started.");

	// LINE1_TODO: Calling this would cause a memory leak, so we don't call it. The memory will be freed when the device is reset.

	CHECK_CALL(HAL_PCD_Stop(&m_DeviceHandle));
	CHECK_CALL(HAL_PCD_DeInit(&m_DeviceHandle));

	m_EP0TransmitHandler.Reset();

	m_DeviceCount = 0;
	m_IsRunning = false;
}

void DaisyUSBDevice::OnHALHandleRequest(void)
{
	HAL_PCD_IRQHandler(&m_DeviceHandle);
}

void DaisyUSBDevice::OnSetupStage(void)
{
	const USBDeviceSetupPacket* setup = reinterpret_cast<const USBDeviceSetupPacket*>(m_DeviceHandle.Setup);

	uint8_t interfaceIndex = (uint8_t)(setup->wIndex & 0xFF);
	uint8_t reqType = setup->bmRequestType & USB_REQ_TYPE_MASK;

	if (reqType == USB_REQ_TYPE_STANDARD)
	{
		switch (setup->bRequest)
		{
		case USB_REQ_GET_DESCRIPTOR:
			HandleGetDescriptor();
			break;

		case USB_REQ_SET_ADDRESS:
		{
			uint8_t devAddr = (uint8_t)(setup->wValue & 0x7F);

			CHECK_CALL(HAL_PCD_SetAddress(&m_DeviceHandle, devAddr));

			DeviceTransmitAck();

			break;
		}

		case USB_REQ_SET_CONFIGURATION:
		{
			for (uint8_t i = 0; i < m_DeviceCount; ++i)
				m_Devices[i].Interface->OnSetupCompleted();

			DeviceTransmitAck();
			break;
		}

		case USB_REQ_SET_INTERFACE:
		{
			uint8_t altSetting = (uint8_t)(setup->wValue & 0xFF);

			DeviceInstanceInfo& dii = GetDeviceInstanceByInterfaceIndex(interfaceIndex);

			if (!dii.Interface->OnSetInterface(interfaceIndex, altSetting))
				SetStall();
			else
				DeviceTransmitAck();

			break;
		}

		case USB_REQ_GET_INTERFACE:
		{
			DeviceInstanceInfo& dii = GetDeviceInstanceByInterfaceIndex(interfaceIndex);

			uint8_t altSetting = dii.Interface->GetCurrentAltSetting(interfaceIndex);
			DeviceTransmit(&altSetting);

			break;
		}

		default:
			SetStall();
			break;
		}
	}
	else if (reqType == USB_REQ_TYPE_CLASS)
	{
		uint8_t recipient = setup->bmRequestType & USB_REQ_RECIPIENT_MASK;

		if (recipient == USB_REQ_RECIPIENT_ENDPOINT)
		{
			DeviceInstanceInfo& dii = GetDeviceInstanceByEndpoint(TO_ENDPOINT_NUMBER(interfaceIndex));
			if (!dii.Interface->OnSetupStage(setup))
				SetStall();
		}
		else
		{
			DeviceInstanceInfo& dii = GetDeviceInstanceByInterfaceIndex(interfaceIndex);
			if (!dii.Interface->OnSetupStage(setup))
				SetStall();
		}
	}
}

void DaisyUSBDevice::OnDataOutStage(uint8_t EPNum)
{
	if (EPNum == TO_ENDPOINT_NUMBER(USB_EP0_OUT))
	{
		//Here the EPNum would be 0 for sure and all of the AMC Command Endpoints are 0 as well
		for (uint8_t i = 0; i < m_DeviceCount; ++i)
		{
			if (m_Devices[i].Class != USBDeviceClasses::AMC)
				continue;

			m_Devices[i].Interface->OnDeviceDataOutStage();
		}

		return;
	}

	DeviceInstanceInfo& dii = GetDeviceInstanceByEndpoint(TO_ENDPOINT_NUMBER(EPNum));
	dii.Interface->OnDataOutStage();
}

void DaisyUSBDevice::OnDataInStage(uint8_t EPNum)
{
	if (EPNum == TO_ENDPOINT_NUMBER(USB_EP0_IN))
	{
		m_EP0TransmitHandler.MoveForward();

		if (m_EP0TransmitHandler.HasMore())
			DeviceTransmit(m_EP0TransmitHandler.GetBuffer(), m_EP0TransmitHandler.GetLength());
		else
			DeviceReceiveAck();

		//Here the EPNum would be 0 for sure and all of the AMC Command Endpoints are 0 as well
		for (uint8_t i = 0; i < m_DeviceCount; ++i)
		{
			if (m_Devices[i].Class != USBDeviceClasses::AMC)
				continue;

			m_Devices[i].Interface->OnDeviceDataInStage();
		}

		return;
	}

	DeviceInstanceInfo& dii = GetDeviceInstanceByEndpoint(EPNum);
	dii.Interface->OnDataInStage();
}

void DaisyUSBDevice::OnIsoOutIncomplete(uint8_t EPNum)
{
	if (EPNum == TO_ENDPOINT_NUMBER(USB_EP0_OUT))
		return;

	DeviceInstanceInfo& dii = GetDeviceInstanceByEndpoint(EPNum);
	dii.Interface->OnIsoOutIncomplete();
}

void DaisyUSBDevice::OnIsoInIncomplete(uint8_t EPNum)
{
	if (EPNum == TO_ENDPOINT_NUMBER(USB_EP0_OUT))
		return;

	DeviceInstanceInfo& dii = GetDeviceInstanceByEndpoint(EPNum);
	dii.Interface->OnIsoInIncomplete();
}

void DaisyUSBDevice::OnStartOfFrame(void)
{
	for (uint8_t i = 0; i < m_DeviceCount; ++i)
		m_Devices[i].Interface->OnStartOfFrame();
}

void DaisyUSBDevice::HandleGetDescriptor(void)
{
	static EP0Buffer ep0Buffers[(uint8_t)Peripherals::COUNT] = {};

	const USBDeviceSetupPacket* setup = reinterpret_cast<const USBDeviceSetupPacket*>(m_DeviceHandle.Setup);

	EP0Buffer& ep0Buffer = ep0Buffers[(uint8_t)m_Peripheral];

	USBDescTypes descType = (USBDescTypes)(setup->wValue >> 8);
	uint8_t descIndex = (uint8_t)(setup->wValue & 0xFF);
	uint16_t sendLen = 0;

	switch (descType)
	{
	case USBDescTypes::Device:
		sendLen = BuildDeviceDescriptor(ep0Buffer, m_Profile);
		break;

	case USBDescTypes::Configuration:
		sendLen = BuildConfigurationDescriptor(ep0Buffer, m_Profile);
		break;

	case USBDescTypes::String:
	{
		if (descIndex == USB_STRING_INDEX_LANGID)
		{
			ep0Buffer.stringDesc.bLength = 4;
			ep0Buffer.stringDesc.bDescriptorType = USBDescTypes::String;
			ep0Buffer.stringDesc.wData[0] = USB_LANGID_ENGLISH_US;

			sendLen = 4;
		}
		else if (descIndex == USB_STRING_INDEX_MANUFACTURER)
			sendLen = BuildStringDescriptor(ep0Buffer, m_Profile.Manufacturer);
		else if (descIndex == USB_STRING_INDEX_PRODUCT)
			sendLen = BuildStringDescriptor(ep0Buffer, m_Profile.Product);
		else if (descIndex == USB_STRING_INDEX_SERIAL)
			sendLen = BuildStringDescriptor(ep0Buffer, m_Profile.SerialNumber);
		else
			for (uint8_t i = 0; i < m_DeviceCount; ++i)
			{
				cstr string = m_Devices[i].Interface->GetDescriptorString(descIndex);
				if (string == nullptr)
					continue;

				sendLen = BuildStringDescriptor(ep0Buffer, string);
				break;
			}
		break;
	}

	default:
		SetStall();
		return;
	}

	ASSERT(sendLen <= sizeof(EP0Buffer), "Descriptor is too large for EP0Buffer");

	if (sendLen > 0)
	{
		uint16_t actualLen = (sendLen < setup->wLength) ? sendLen : setup->wLength;

		m_EP0TransmitHandler.Set(&ep0Buffer, actualLen);

		DeviceTransmit(m_EP0TransmitHandler.GetBuffer(), m_EP0TransmitHandler.GetLength());
	}
	else
		SetStall();
}

void DaisyUSBDevice::AllocateReceiveBuffer(uint16_t Size)
{
	CHECK_CALL(HAL_PCDEx_SetRxFiFo(&m_DeviceHandle, BYTES_TO_DWORDS(Size)));
}

void DaisyUSBDevice::AllocateTransmitBuffer(uint8_t Endpoint, uint16_t Size)
{
	CHECK_CALL(HAL_PCDEx_SetTxFiFo(&m_DeviceHandle, TO_ENDPOINT_NUMBER(Endpoint), BYTES_TO_DWORDS(Size)));
}

void DaisyUSBDevice::OpenEndpoint(uint8_t Endpoint, uint16_t Length, USBEndpointAttributes Type)
{
	CHECK_CALL(HAL_PCD_EP_Open(&m_DeviceHandle, Endpoint, Length, (uint8_t)Type));
}

void DaisyUSBDevice::CloseEndpoint(uint8_t Endpoint)
{
	CHECK_CALL(HAL_PCD_EP_Close(&m_DeviceHandle, Endpoint));
}

uint16_t DaisyUSBDevice::DeviceReceiveCount(uint8_t Endpoint)
{
	return HAL_PCD_EP_GetRxCount(&m_DeviceHandle, Endpoint);
}

void DaisyUSBDevice::DeviceReceive(uint8_t* Buffer, uint16_t Length, uint8_t Endpoint)
{
	CHECK_CALL(HAL_PCD_EP_Receive(&m_DeviceHandle, Endpoint, Buffer, Length));
}

void DaisyUSBDevice::DeviceTransmit(const uint8_t* Buffer, uint16_t Length, uint8_t Endpoint, bool ClearDCache)
{
	if (ClearDCache)
		SCB_CleanDCache_by_Addr(const_cast<uint8_t*>(Buffer), Length);

	CHECK_CALL(HAL_PCD_EP_Transmit(&m_DeviceHandle, Endpoint, const_cast<uint8_t*>(Buffer), Length));
}

void DaisyUSBDevice::FlushEndpoint(uint8_t Endpoint)
{
	CHECK_CALL(HAL_PCD_EP_Flush(&m_DeviceHandle, Endpoint));
}

void DaisyUSBDevice::SetStall(void)
{
	CHECK_CALL(HAL_PCD_EP_SetStall(&m_DeviceHandle, USB_EP0_OUT));
}

DaisyUSBDevice::DeviceInstanceInfo& DaisyUSBDevice::GetDeviceInstanceByInterfaceIndex(uint8_t InterfaceIndex)
{
	for (uint8_t i = 0; i < m_DeviceCount; ++i)
	{
		if (!m_Devices[i].Interface->MatchByInterfaceIndex(InterfaceIndex))
			continue;

		return m_Devices[i];
	}

	BREAK("Couldn't find the proper interface");
}

DaisyUSBDevice::DeviceInstanceInfo& DaisyUSBDevice::GetDeviceInstanceByEndpoint(uint8_t Endpoint)
{
	for (uint8_t i = 0; i < m_DeviceCount; ++i)
	{
		if (!m_Devices[i].Interface->MatchByEndpoint(Endpoint))
			continue;

		return m_Devices[i];
	}

	BREAK("Couldn't find the proper interface");
}

uint16_t DaisyUSBDevice::BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, const USBDeviceProfile& Profile)
{
	uint8_t* buffer = EP0Buffer.configDescs;
	uint16_t offset = 0;

	USBConfigurationDescriptor* config = reinterpret_cast<USBConfigurationDescriptor*>(buffer + offset);
	offset += sizeof(USBConfigurationDescriptor);

	uint8_t interfaceIndex = 0;
	for (uint8_t i = 0; i < Profile.ClassNodeCount; ++i)
	{
		DeviceInstanceInfo& dii = m_Devices[i];

		dii.Interface->BuildConfigurationDescriptor(EP0Buffer, offset, interfaceIndex);

		interfaceIndex += dii.Interface->GetConfigs().InterfaceIndexCount;
	}

	config->bLength = sizeof(USBConfigurationDescriptor);
	config->bDescriptorType = USBDescTypes::Configuration;
	config->wTotalLength = offset;
	config->bNumInterfaces = interfaceIndex;
	config->bConfigurationValue = USB_CONFIG_VALUE_DEFAULT;
	config->iConfiguration = 0;
	config->bmAttributes = Profile.IsSelfPowered ? USBConfigAttributes::SelfPoweredMask : USBConfigAttributes::BusPoweredMask;
	config->bMaxPower = Profile.MaxPowerCurrent / 2;

	return offset;
}

uint16_t DaisyUSBDevice::BuildDeviceDescriptor(EP0Buffer& EP0Buffer, const USBDeviceProfile& Profile)
{
	EP0Buffer.deviceDesc.bLength = sizeof(USBDeviceDescriptor);
	EP0Buffer.deviceDesc.bDescriptorType = USBDescTypes::Device;
	EP0Buffer.deviceDesc.bcdUSB = USB_VERSION_2_0;
	EP0Buffer.deviceDesc.bDeviceClass = USBSDeviceClasses::Misc;
	EP0Buffer.deviceDesc.bDeviceSubClass = USBDeviceSubClasses::Common;
	EP0Buffer.deviceDesc.bDeviceProtocol = USBDeviceProtocols::IAD;
	EP0Buffer.deviceDesc.bMaxPacketSize0 = (uint16_t)PacketSizes::Max;
	EP0Buffer.deviceDesc.idVendor = Profile.VendorID;
	EP0Buffer.deviceDesc.idProduct = Profile.ProductID;
	EP0Buffer.deviceDesc.bcdDevice = Profile.Version;

	if (GetStringLength(Profile.Manufacturer) != 0)
		EP0Buffer.deviceDesc.iManufacturer = USB_STRING_INDEX_MANUFACTURER;
	else
		EP0Buffer.deviceDesc.iManufacturer = 0;

	if (GetStringLength(Profile.Product) != 0)
		EP0Buffer.deviceDesc.iProduct = USB_STRING_INDEX_PRODUCT;
	else
		EP0Buffer.deviceDesc.iProduct = 0;

	if (GetStringLength(Profile.SerialNumber) != 0)
		EP0Buffer.deviceDesc.iSerialNumber = USB_STRING_INDEX_SERIAL;
	else
		EP0Buffer.deviceDesc.iSerialNumber = 0;

	EP0Buffer.deviceDesc.bNumConfigurations = USB_CONFIG_VALUE_DEFAULT;

	return sizeof(USBDeviceDescriptor);
}

uint16_t DaisyUSBDevice::BuildStringDescriptor(EP0Buffer& EP0Buffer, cstr Value)
{
	if (!Value)
		return 0;

	uint8_t length = Math::Min(USBMaxStringLength, GetStringLength(Value));
	for (uint8_t i = 0; i < length; ++i)
		EP0Buffer.stringDesc.wData[i] = Value[i];

	uint8_t totalLength = __offsetof(USBStringDescriptor, wData) + (length * sizeof(uint16_t));
	EP0Buffer.stringDesc.bLength = totalLength;
	EP0Buffer.stringDesc.bDescriptorType = USBDescTypes::String;

	return totalLength;
}

#endif