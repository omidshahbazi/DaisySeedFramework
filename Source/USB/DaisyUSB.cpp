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

	void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum)
	{
		for (uint8_t i = 0; i < (uint8_t)DaisyUSB::Peripherals::COUNT; ++i)
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
		for (uint8_t i = 0; i < (uint8_t)DaisyUSB::Peripherals::COUNT; ++i)
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
		for (uint8_t i = 0; i < (uint8_t)DaisyUSB::Peripherals::COUNT; ++i)
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

DaisyUSB::DaisyUSB(Peripherals Peripheral)
	: m_Peripheral(Peripheral),
	m_IsRunning(false),
	m_EP0TransmitHandler((uint16)PacketSizes::Max),
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
		uint8 nextEndpoint = 1;
		uint8 interfaceIndex = 0;

		for (uint8 i = 0; i < Profile.Device.ClassNodeCount; ++i)
		{
			const USBClassNode& node = Profile.Device.ClassNodes[i];

			DeviceInstanceInfo& dii = m_Devices[m_DeviceCount++];
			dii.Class = node.Class;

			DaisyUSBInterfaceCommon::Configs configs;
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

				configs.MaxReceivePacketSize = (uint16)node.CDC.ReceiveBufferSize;
				configs.MaxTransmitPacketSize = (uint16)node.CDC.SendBufferSize;

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

				configs.MaxReceivePacketSize = DaisyUSBAMCInterface::CalculateMaxPacketSize(node.AMC.OutputChannelCount, node.AMC);
				configs.MaxTransmitPacketSize = DaisyUSBAMCInterface::CalculateMaxPacketSize(node.AMC.InputChannelCount, node.AMC);

				DaisyUSBAMCInterface* amc = Memory::Allocate<DaisyUSBAMCInterface>(1, true);
				new (amc) DaisyUSBAMCInterface(this, configs, node.AMC);
				dii.Interface = amc;

				break;
			}
			}

			interfaceIndex += configs.InterfaceIndexCount;
		}
	}
	else
		ASSERT(false, "Not implemented");

	if (m_Profile.Mode == USBModes::Device)
	{
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
		AllocateTransmitBuffer(USB_EP0_IN, (uint16)PacketSizes::Max);

		OpenEndpoint(USB_EP0_OUT, (uint16)PacketSizes::Max, USBEndpointAttributes::Control);
		OpenEndpoint(USB_EP0_IN, (uint16)PacketSizes::Max, USBEndpointAttributes::Control);

		CHECK_CALL(HAL_PCD_Start(&m_DeviceHandle));

		HAL_NVIC_SetPriority(OTG_HS_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(OTG_HS_IRQn);

		HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
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

	// Calling this would cause a memory leak, so we don't call it. The memory will be freed when the device is reset.

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

	HAL_Delay(100);

	m_EP0TransmitHandler.Reset();

	m_DeviceCount = 0;
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
			uint8 devAddr = (uint8)(setup->wValue & 0x7F);

			CHECK_CALL(HAL_PCD_SetAddress(&m_DeviceHandle, devAddr));

			DeviceTransmitAck();

			break;
		}

		case USB_REQ_SET_CONFIGURATION:
		{
			for (uint8 i = 0; i < m_DeviceCount; ++i)
				m_Devices[i].Interface->OnSetupCompleted();

			DeviceTransmitAck();
			break;
		}

		case USB_REQ_SET_INTERFACE:
		{
			uint8 interfaceIndex = (uint8)(setup->wIndex & 0xFF);
			uint8 altSetting = (uint8)(setup->wValue & 0xFF);

			DeviceInstanceInfo& dii = GetDeviceInstanceByInterfaceIndex(interfaceIndex);

			if (!dii.Interface->OnSetInterface(interfaceIndex, altSetting))
				SetStall();
			else
				DeviceTransmitAck();

			break;
		}

		case USB_REQ_GET_INTERFACE:
		{
			uint8 interfaceIndex = (uint8)(setup->wIndex & 0xFF);

			DeviceInstanceInfo& dii = GetDeviceInstanceByInterfaceIndex(interfaceIndex);

			uint8 altSetting = dii.Interface->GetCurrentAltSetting(interfaceIndex);
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
		uint8 recipient = setup->bmRequestType & USB_REQ_RECIPIENT_MASK;

		if (recipient == USB_REQ_RECIPIENT_ENDPOINT)
		{
			DeviceInstanceInfo& dii = GetDeviceInstanceByEndpoint(TO_ENDPOINT_NUMBER((uint8)(setup->wIndex & 0xFF)));
			if (!dii.Interface->OnSetupStage(setup))
				SetStall();
		}
		else
		{
			DeviceInstanceInfo& dii = GetDeviceInstanceByInterfaceIndex((uint8)(setup->wIndex & 0xFF));
			if (!dii.Interface->OnSetupStage(setup))
				SetStall();
		}
	}
}

void DaisyUSB::OnDataOutStage(uint8 EPNum)
{
	DeviceInstanceInfo& dii = GetDeviceInstanceByEndpoint(EPNum);

	if (EPNum == TO_ENDPOINT_NUMBER(USB_EP0_OUT))
	{
		dii.Interface->OnDeviceDataOutStage();

		return;
	}

	dii.Interface->OnDataOutStage();
}

void DaisyUSB::OnDataInStage(uint8 EPNum)
{
	DeviceInstanceInfo& dii = GetDeviceInstanceByEndpoint(EPNum);

	if (EPNum == TO_ENDPOINT_NUMBER(USB_EP0_IN))
	{
		if (m_EP0TransmitHandler.HasMore())
		{
			DeviceTransmit(m_EP0TransmitHandler.GetBuffer(), m_EP0TransmitHandler.GetLength());

			m_EP0TransmitHandler.MoveForward();
		}
		else
			DeviceReceiveAck();

		dii.Interface->OnDeviceDataInStage();

		return;
	}

	dii.Interface->OnDataInStage();
}

void DaisyUSB::OnIsoOutIncomplete(uint8 EPNum)
{
	if (EPNum == TO_ENDPOINT_NUMBER(USB_EP0_OUT))
		return;

	DeviceInstanceInfo& dii = GetDeviceInstanceByEndpoint(EPNum);
	dii.Interface->OnIsoOutIncomplete();
}

void DaisyUSB::OnIsoInIncomplete(uint8 EPNum)
{
	if (EPNum == TO_ENDPOINT_NUMBER(USB_EP0_OUT))
		return;

	DeviceInstanceInfo& dii = GetDeviceInstanceByEndpoint(EPNum);
	dii.Interface->OnIsoInIncomplete();
}

void DaisyUSB::OnStartOfFrame(void)
{
	for (uint8 i = 0; i < m_DeviceCount; ++i)
		m_Devices[i].Interface->OnStartOfFrame();
}

void DaisyUSB::HandleGetDescriptor(void)
{
	static EP0Buffer ep0Buffers[(uint8)DaisyUSB::Peripherals::COUNT] = {};

	const USBDeviceSetupPacket* setup = reinterpret_cast<const USBDeviceSetupPacket*>(m_DeviceHandle.Setup);

	EP0Buffer& ep0Buffer = ep0Buffers[(uint8)m_Peripheral];

	USBDescTypes descType = (USBDescTypes)(setup->wValue >> 8);
	uint8 descIndex = (uint8)(setup->wValue & 0xFF);
	uint16 sendLen = 0;

	switch (descType)
	{
	case USBDescTypes::Device:
		sendLen = BuildDeviceDescriptor(ep0Buffer, m_Profile);
		break;

	case USBDescTypes::Configuration:
		sendLen = BuildConfigurationDescriptor(ep0Buffer, m_Profile.Device);
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
			sendLen = BuildStringDescriptor(ep0Buffer, m_Profile.Device.Manufacturer);
		else if (descIndex == USB_STRING_INDEX_PRODUCT)
			sendLen = BuildStringDescriptor(ep0Buffer, m_Profile.Device.Product);
		else if (descIndex == USB_STRING_INDEX_SERIAL)
			sendLen = BuildStringDescriptor(ep0Buffer, m_Profile.Device.SerialNumber);
		else
			for (uint8 i = 0; i < m_DeviceCount; ++i)
				if ((sendLen = m_Devices[i].Interface->HandleGetDescriptor(ep0Buffer, descIndex)) != 0)
					break;
		break;
	}

	default:
		SetStall();
		return;
	}

	ASSERT(sendLen <= sizeof(EP0Buffer), "Descriptor is too large for EP0Buffer");

	if (sendLen > 0)
	{
		uint16 actualLen = (sendLen < setup->wLength) ? sendLen : setup->wLength;

		m_EP0TransmitHandler.Set(&ep0Buffer, actualLen);

		DeviceTransmit(m_EP0TransmitHandler.GetBuffer(), m_EP0TransmitHandler.GetLength());

		m_EP0TransmitHandler.MoveForward();
	}
	else
		SetStall();
}

void DaisyUSB::AllocateReceiveBuffer(uint16 Size)
{
	CHECK_CALL(HAL_PCDEx_SetRxFiFo(&m_DeviceHandle, BYTES_TO_DWORDS(Size)));
}

void DaisyUSB::AllocateTransmitBuffer(uint8 Endpoint, uint16 Size)
{
	CHECK_CALL(HAL_PCDEx_SetTxFiFo(&m_DeviceHandle, TO_ENDPOINT_NUMBER(Endpoint), BYTES_TO_DWORDS(Size)));
}

void DaisyUSB::OpenEndpoint(uint8 Endpoint, uint16 Length, USBEndpointAttributes Type)
{
	CHECK_CALL(HAL_PCD_EP_Open(&m_DeviceHandle, Endpoint, Length, (uint8)Type));
}

void DaisyUSB::CloseEndpoint(uint8 Endpoint)
{
	CHECK_CALL(HAL_PCD_EP_Close(&m_DeviceHandle, Endpoint));
}

uint16 DaisyUSB::DeviceReceiveCount(uint8 Endpoint)
{
	return HAL_PCD_EP_GetRxCount(&m_DeviceHandle, Endpoint);
}

void DaisyUSB::DeviceReceive(uint8* Buffer, uint16 Length, uint8 Endpoint)
{
	CHECK_CALL(HAL_PCD_EP_Receive(&m_DeviceHandle, Endpoint, Buffer, Length));
}

void DaisyUSB::DeviceTransmit(const uint8* Buffer, uint16 Length, uint8 Endpoint, bool ClearDCache)
{
	if (ClearDCache)
		SCB_CleanDCache_by_Addr(const_cast<uint8*>(Buffer), Length);

	CHECK_CALL(HAL_PCD_EP_Transmit(&m_DeviceHandle, Endpoint, const_cast<uint8*>(Buffer), Length));
}

void DaisyUSB::FlushEndpoint(uint8 Endpoint)
{
	CHECK_CALL(HAL_PCD_EP_Flush(&m_DeviceHandle, Endpoint));
}

void DaisyUSB::SetStall(void)
{
	CHECK_CALL(HAL_PCD_EP_SetStall(&m_DeviceHandle, USB_EP0_OUT));
}

DaisyUSB::DeviceInstanceInfo& DaisyUSB::GetDeviceInstanceByInterfaceIndex(uint8 InterfaceIndex)
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

uint16 DaisyUSB::BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, const USBDeviceProfile& Profile)
{
	uint8* buffer = EP0Buffer.configDescs;
	uint16 offset = 0;

	USBConfigurationDescriptor* config = reinterpret_cast<USBConfigurationDescriptor*>(buffer + offset);
	offset += sizeof(USBConfigurationDescriptor);

	uint8 interfaceIndex = 0;
	for (uint8 i = 0; i < Profile.ClassNodeCount; ++i)
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

uint16 DaisyUSB::BuildDeviceDescriptor(EP0Buffer& EP0Buffer, const USBProfile& Profile)
{
	EP0Buffer.deviceDesc.bLength = sizeof(USBDeviceDescriptor);
	EP0Buffer.deviceDesc.bDescriptorType = USBDescTypes::Device;
	EP0Buffer.deviceDesc.bcdUSB = USB_VERSION_2_0;
	EP0Buffer.deviceDesc.bDeviceClass = USBSDeviceClasses::Misc;
	EP0Buffer.deviceDesc.bDeviceSubClass = USBDeviceSubClasses::Common;
	EP0Buffer.deviceDesc.bDeviceProtocol = USBDeviceProtocols::IAD;
	EP0Buffer.deviceDesc.bMaxPacketSize0 = (uint16)PacketSizes::Max;
	EP0Buffer.deviceDesc.idVendor = Profile.Device.VendorID;
	EP0Buffer.deviceDesc.idProduct = Profile.Device.ProductID;
	EP0Buffer.deviceDesc.bcdDevice = Profile.Device.Version;

	if (GetStringLength(Profile.Device.Manufacturer) != 0)
		EP0Buffer.deviceDesc.iManufacturer = USB_STRING_INDEX_MANUFACTURER;
	else
		EP0Buffer.deviceDesc.iManufacturer = 0;

	if (GetStringLength(Profile.Device.Product) != 0)
		EP0Buffer.deviceDesc.iProduct = USB_STRING_INDEX_PRODUCT;
	else
		EP0Buffer.deviceDesc.iProduct = 0;

	if (GetStringLength(Profile.Device.SerialNumber) != 0)
		EP0Buffer.deviceDesc.iSerialNumber = USB_STRING_INDEX_SERIAL;
	else
		EP0Buffer.deviceDesc.iSerialNumber = 0;

	EP0Buffer.deviceDesc.bNumConfigurations = USB_CONFIG_VALUE_DEFAULT;

	return sizeof(USBDeviceDescriptor);
}

uint16 DaisyUSB::BuildStringDescriptor(EP0Buffer& EP0Buffer, cstr Value)
{
	if (!Value)
		return 0;

	uint8 length = Math::Min(USBMaxStringLength, GetStringLength(Value));
	for (uint8 i = 0; i < length; ++i)
		EP0Buffer.stringDesc.wData[i] = Value[i];

	uint8 totalLength = __offsetof(USBStringDescriptor, wData) + (length * sizeof(uint16));
	EP0Buffer.stringDesc.bLength = totalLength;
	EP0Buffer.stringDesc.bDescriptorType = USBDescTypes::String;

	return totalLength;
}

#endif