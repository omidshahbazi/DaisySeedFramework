#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBInterface.h"
#include "DaisySeedFramework/USB/USBDefinitions.h"
#include <DigitalSignalProcessing/Debug.h>

// LINE1_TODO: Handle return values
// LINE1_TODO: Assert on number of classes

static DaisyUSBInterface* s_Instance[(uint8)DaisyUSBInterface::Peripherals::COUNT] = {};

extern "C"
{
	void OTG_FS_IRQHandler(void)
	{
		s_Instance[(uint8)DaisyUSBInterface::Peripherals::FullSpeed]->OnHALHandleRequest();
	}

	void OTG_HS_IRQHandler(void)
	{
		s_Instance[(uint8)DaisyUSBInterface::Peripherals::HighSpeed]->OnHALHandleRequest();
	}

	void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef* hpcd)
	{
		for (uint8_t i = 0; i < (uint8_t)DaisyUSBInterface::Peripherals::COUNT; ++i)
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
		for (uint8_t i = 0; i < (uint8_t)DaisyUSBInterface::Peripherals::COUNT; ++i)
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
		for (uint8_t i = 0; i < (uint8_t)DaisyUSBInterface::Peripherals::COUNT; ++i)
		{
			if (s_Instance[i] == nullptr)
				continue;

			if (&s_Instance[i]->m_DeviceHandle != hpcd)
				continue;

			s_Instance[i]->OnDataOutStage(epnum);

			break;
		}
	}

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
}

DaisyUSBInterface::DaisyUSBInterface(Peripherals Peripheral)
	: m_Peripheral(Peripheral),
	m_IsRunning(false),
	m_EO0TransmitRemainingLength(0),
	m_EP0TransmitBufferStart(nullptr)
{
	s_Instance[(uint8)Peripheral] = this;
}

void DaisyUSBInterface::Start(const USBProfile& Profile)
{
	ASSERT(!m_IsRunning, "Interface has already started.");

	m_Profile = Profile;

	if (m_Profile.Mode == USBModes::Device)
	{
		if (m_Peripheral == Peripherals::HighSpeed)
		{
			m_DeviceHandle.Instance = USB_OTG_HS;
			m_DeviceHandle.Init.dev_endpoints = 9;
			m_DeviceHandle.Init.speed = PCD_SPEED_FULL;
			m_DeviceHandle.Init.phy_itface = PCD_PHY_EMBEDDED;
		}
		else
		{
			m_DeviceHandle.Instance = USB_OTG_FS;
			m_DeviceHandle.Init.dev_endpoints = 9;
			m_DeviceHandle.Init.speed = PCD_SPEED_FULL;
			m_DeviceHandle.Init.phy_itface = PCD_PHY_EMBEDDED;
		}

		m_DeviceHandle.Init.Sof_enable = DISABLE;
		m_DeviceHandle.Init.low_power_enable = DISABLE;
		m_DeviceHandle.Init.lpm_enable = DISABLE;
		m_DeviceHandle.Init.vbus_sensing_enable = DISABLE;

		HAL_PCD_Init(&m_DeviceHandle);

		HAL_PCD_EP_Open(&m_DeviceHandle, USB_EP0_OUT, 64, EP_TYPE_CTRL);
		HAL_PCD_EP_Open(&m_DeviceHandle, USB_EP0_IN, 64, EP_TYPE_CTRL);

		HAL_PCD_Start(&m_DeviceHandle);
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

		m_HostHandle.Init.Host_channels = 16;
		m_HostHandle.Init.Sof_enable = DISABLE;
		m_HostHandle.Init.vbus_sensing_enable = DISABLE;

		HAL_HCD_Init(&m_HostHandle);
		HAL_HCD_Start(&m_HostHandle);
	}

	m_IsRunning = true;
}

void DaisyUSBInterface::Stop(void)
{
	ASSERT(m_IsRunning, "Interface is not started.");

	if (m_Profile.Mode == USBModes::Device)
	{
		HAL_PCD_Stop(&m_DeviceHandle);
		HAL_PCD_DeInit(&m_DeviceHandle);
	}
	else
	{
		HAL_HCD_Stop(&m_HostHandle);
		HAL_HCD_DeInit(&m_HostHandle);
	}

	m_IsRunning = false;
}

void DaisyUSBInterface::OnHALHandleRequest(void)
{
	if (m_Profile.Mode == USBModes::Device)
		HAL_PCD_IRQHandler(&m_DeviceHandle);
	else
		HAL_HCD_IRQHandler(&m_HostHandle);
}

void DaisyUSBInterface::OnSetupStage(void)
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
			HAL_PCD_SetAddress(&m_DeviceHandle, devAddr);

			DeviceTransmitAck();

			break;
		}

		case USB_REQ_SET_CONFIGURATION:
		{
			for (uint8 i = 0; i < m_Profile.Device.ClassNodeCount; ++i)
			{
				const USBClassNode& node = m_Profile.Device.ClassNodes[i];

				if (node.Class == USBDeviceClassses::CDC)
				{
					HAL_PCD_EP_Open(&m_DeviceHandle, node.CDC.CustomEndpointCommand, 8, EP_TYPE_INTR);
					HAL_PCD_EP_Open(&m_DeviceHandle, node.CDC.CustomEndpointOut, 64, EP_TYPE_BULK);
					HAL_PCD_EP_Open(&m_DeviceHandle, node.CDC.CustomEndpointIn, 64, EP_TYPE_BULK);
				}
			}

			DeviceTransmitAck();
			break;
		}

		default:
			HAL_PCD_EP_SetStall(&m_DeviceHandle, USB_EP0_OUT);
			break;
		}
	}
	else if (reqType == USB_REQ_TYPE_CLASS)
	{
		switch (setup->bRequest)
		{
		case USB_CDC_REQ_SET_LINE_CODING:
		{
			if (setup->wLength > 0)
				DeviceReceive(&m_DeviceLineCoding);

			DeviceTransmitAck();

			break;
		}

		case USB_CDC_REQ_GET_LINE_CODING:
		{
			DeviceTransmit(&m_DeviceLineCoding);
			DeviceWaitForReceive();

			break;
		}

		case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		{
			//m_ControlLineState = static_cast<uint8>(setup->wValue & 0xFF);
			DeviceTransmitAck();

			break;
		}

		default:
			HAL_PCD_EP_SetStall(&m_DeviceHandle, USB_EP0_OUT);
			break;
		}
	}
}

void DaisyUSBInterface::OnDataInStage(uint8 EPNum)
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
		{
			// اتمام انتقال داده، اکنون منتظر پکت تاییدیه (Status) از هاست روی OUT می‌شویم
			DeviceWaitForReceive();
		}
	}
}

void DaisyUSBInterface::OnDataOutStage(uint8 EPNum)
{}

void DaisyUSBInterface::HandleGetDescriptor(void)
{
	static EP0Buffer ep0Buffers[(uint8)DaisyUSBInterface::Peripherals::COUNT] = {};

	const USBDeviceSetupPacket* setup = reinterpret_cast<const USBDeviceSetupPacket*>(m_DeviceHandle.Setup);

	EP0Buffer& ep0Buffer = ep0Buffers[(uint8)m_Peripheral];

	uint8 descType = static_cast<uint8>(setup->wValue >> 8);
	uint8 descIndex = static_cast<uint8>(setup->wValue & 0xFF);
	uint16 sendLen = 0;

	switch (descType)
	{
	case USB_DESC_TYPE_DEVICE:
		sendLen = BuildDeviceDescriptor(ep0Buffer);
		break;

	case USB_DESC_TYPE_CONFIGURATION:
		sendLen = BuildConfigurationDescriptor(ep0Buffer, m_Profile.Device);
		break;

	case USB_DESC_TYPE_STRING:
	{
		if (descIndex == USB_STRING_INDEX_LANGID)
		{
			ep0Buffer.stringDesc.bLength = 4;
			ep0Buffer.stringDesc.bDescriptorType = USB_DESC_TYPE_STRING;
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
		HAL_PCD_EP_SetStall(&m_DeviceHandle, USB_EP0_OUT);
		return;
	}

	if (sendLen > 0)
	{
		uint16 actualLen = (sendLen < setup->wLength) ? sendLen : setup->wLength;

		// تنظیم وضعیت برای ارسال‌های چندبخشی (Chunking)
		if (actualLen > MaxPacketSize)
		{
			m_EP0TransmitBufferStart = reinterpret_cast<const uint8*>(&ep0Buffer) + MaxPacketSize;
			m_EO0TransmitRemainingLength = actualLen - MaxPacketSize;
			actualLen = MaxPacketSize; // ارسال بسته اول با حداکثر سایز ۶۴ بایت
		}
		else
		{
			m_EP0TransmitBufferStart = nullptr;
			m_EO0TransmitRemainingLength = 0;
		}

		DeviceTransmit(reinterpret_cast<uint8*>(&ep0Buffer), actualLen);
	}
}

uint16 DaisyUSBInterface::BuildDeviceDescriptor(EP0Buffer& EP0Buffer)
{
	EP0Buffer.deviceDesc.bLength = 18;
	EP0Buffer.deviceDesc.bDescriptorType = USB_DESC_TYPE_DEVICE;
	EP0Buffer.deviceDesc.bcdUSB = USB_VERSION_2_0;
	EP0Buffer.deviceDesc.bDeviceClass = USB_CLASS_MISC;
	EP0Buffer.deviceDesc.bDeviceSubClass = USB_SUBCLASS_COMMON;
	EP0Buffer.deviceDesc.bDeviceProtocol = USB_PROTOCOL_IAD;
	EP0Buffer.deviceDesc.bMaxPacketSize0 = MaxPacketSize;
	EP0Buffer.deviceDesc.idVendor = m_Profile.Device.VendorID;
	EP0Buffer.deviceDesc.idProduct = m_Profile.Device.ProductID;
	EP0Buffer.deviceDesc.bcdDevice = m_Profile.Device.Version;
	EP0Buffer.deviceDesc.iManufacturer = USB_STRING_INDEX_MANUFACTURER;
	EP0Buffer.deviceDesc.iProduct = USB_STRING_INDEX_PRODUCT;
	EP0Buffer.deviceDesc.iSerialNumber = USB_STRING_INDEX_SERIAL;
	EP0Buffer.deviceDesc.bNumConfigurations = 1;

	return sizeof(USBDeviceDescriptor);
}

uint16 DaisyUSBInterface::BuildStringDescriptor(EP0Buffer& EP0Buffer, cstr Value)
{
	if (!Value)
		return 0;

	uint8 length = 0;
	while (Value[length] != '\0' && length < 31)
	{
		EP0Buffer.stringDesc.wData[length] = static_cast<uint16>(Value[length]);
		length++;
	}

	uint8 totalLength = 2 + (length * 2);
	EP0Buffer.stringDesc.bLength = totalLength;
	EP0Buffer.stringDesc.bDescriptorType = USB_DESC_TYPE_STRING;

	return totalLength;
}

uint16 DaisyUSBInterface::BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, const USBDeviceProfile& Profile)
{
	uint8* buffer = EP0Buffer.configDescs;
	uint16 offset = 0;

	// ۱. رزرو کردن جا برای Configuration Descriptor اصلی در ابتدای بافر
	USBConfigurationDescriptor* config = reinterpret_cast<USBConfigurationDescriptor*>(buffer + offset);
	offset += sizeof(USBConfigurationDescriptor);

	uint8 interfaceCounter = 0;

	// ۲. پیمایش روی نودهای کلاس تعریف‌شده در پروفایل کاربر
	for (uint8 i = 0; i < Profile.ClassNodeCount; ++i)
	{
		const USBClassNode& node = Profile.ClassNodes[i];

		if (node.Class == USBDeviceClassses::CDC)
		{
			// --- الف) اضافه کردن IAD برای این پورت CDC ---
			USBInterfaceAssociationDescriptor* iad = reinterpret_cast<USBInterfaceAssociationDescriptor*>(buffer + offset);
			iad->bLength = sizeof(USBInterfaceAssociationDescriptor);
			iad->bDescriptorType = 0x0B; // IAD Type
			iad->bFirstInterface = interfaceCounter; // شماره اولین اینترفیس این CDC
			iad->bInterfaceCount = 2;                // Control + Data Interface
			iad->bFunctionClass = USB_CLASS_CDC;
			iad->bFunctionSubClass = 0x02;             // ACM
			iad->bFunctionProtocol = 0x01;
			iad->iFunction = 0;
			offset += sizeof(USBInterfaceAssociationDescriptor);

			// --- ب) Control Interface ---
			USBInterfaceDescriptor* ctrlIf = reinterpret_cast<USBInterfaceDescriptor*>(buffer + offset);
			ctrlIf->bLength = sizeof(USBInterfaceDescriptor);
			ctrlIf->bDescriptorType = 0x04;
			ctrlIf->bInterfaceNumber = interfaceCounter;
			ctrlIf->bAlternateSetting = 0;
			ctrlIf->bNumEndpoints = 1;
			ctrlIf->bInterfaceClass = USB_CLASS_CDC;
			ctrlIf->bInterfaceSubClass = 0x02;
			ctrlIf->bInterfaceProtocol = 0x01;
			ctrlIf->iInterface = 0;
			offset += sizeof(USBInterfaceDescriptor);

			// --- ج) CDC Functional Descriptors ---
			USBCDCHeaderFunctionalDescriptor* header = reinterpret_cast<USBCDCHeaderFunctionalDescriptor*>(buffer + offset);
			header->bFunctionLength = sizeof(USBCDCHeaderFunctionalDescriptor);
			header->bDescriptorType = 0x24; header->bDescriptorSubtype = 0x00;
			header->bcdCDC = 0x0110;
			offset += sizeof(USBCDCHeaderFunctionalDescriptor);

			USBCDCACMFunctionalDescriptor* acm = reinterpret_cast<USBCDCACMFunctionalDescriptor*>(buffer + offset);
			acm->bFunctionLength = sizeof(USBCDCACMFunctionalDescriptor);
			acm->bDescriptorType = 0x24; acm->bDescriptorSubtype = 0x02; acm->bmCapabilities = 0x02;
			offset += sizeof(USBCDCACMFunctionalDescriptor);

			USBCDCUnionFunctionalDescriptor* cdcUnion = reinterpret_cast<USBCDCUnionFunctionalDescriptor*>(buffer + offset);
			cdcUnion->bFunctionLength = sizeof(USBCDCUnionFunctionalDescriptor);
			cdcUnion->bDescriptorType = 0x24; cdcUnion->bDescriptorSubtype = 0x06;
			cdcUnion->bMasterInterface = interfaceCounter;
			cdcUnion->bSlaveInterface0 = interfaceCounter + 1;
			offset += sizeof(USBCDCUnionFunctionalDescriptor);

			// --- د) Control Endpoint ---
			USBEndpointDescriptor* ctrlEp = reinterpret_cast<USBEndpointDescriptor*>(buffer + offset);
			ctrlEp->bLength = sizeof(USBEndpointDescriptor);
			ctrlEp->bDescriptorType = 0x05;
			ctrlEp->bEndpointAddress = node.CDC.CustomEndpointCommand;
			ctrlEp->bmAttributes = 0x03; // Interrupt
			ctrlEp->wMaxPacketSize = 8;
			ctrlEp->bInterval = (m_Peripheral == Peripherals::FullSpeed ? 10 : 6);
			offset += sizeof(USBEndpointDescriptor);

			// --- هـ) Data Interface ---
			USBInterfaceDescriptor* dataIf = reinterpret_cast<USBInterfaceDescriptor*>(buffer + offset);
			dataIf->bLength = sizeof(USBInterfaceDescriptor);
			dataIf->bDescriptorType = 0x04;
			dataIf->bInterfaceNumber = interfaceCounter + 1;
			dataIf->bAlternateSetting = 0;
			dataIf->bNumEndpoints = 2;
			dataIf->bInterfaceClass = 0x0A; // Data Class
			dataIf->bInterfaceSubClass = 0x00;
			dataIf->bInterfaceProtocol = 0x00;
			dataIf->iInterface = 0;
			offset += sizeof(USBInterfaceDescriptor);

			// --- و) Data OUT & IN Endpoints ---
			USBEndpointDescriptor* outEp = reinterpret_cast<USBEndpointDescriptor*>(buffer + offset);
			outEp->bLength = sizeof(USBEndpointDescriptor);
			outEp->bDescriptorType = 0x05;
			outEp->bEndpointAddress = node.CDC.CustomEndpointOut;
			outEp->bmAttributes = 0x02; // Bulk
			outEp->wMaxPacketSize = 64;
			outEp->bInterval = 0;
			offset += sizeof(USBEndpointDescriptor);

			USBEndpointDescriptor* inEp = reinterpret_cast<USBEndpointDescriptor*>(buffer + offset);
			inEp->bLength = sizeof(USBEndpointDescriptor);
			inEp->bDescriptorType = 0x05;
			inEp->bEndpointAddress = node.CDC.CustomEndpointIn;
			inEp->bmAttributes = 0x02; // Bulk
			inEp->wMaxPacketSize = 64;
			inEp->bInterval = 0;
			offset += sizeof(USBEndpointDescriptor);

			interfaceCounter += 2; // مصرف شدن دو اینترفیس توسط این CDC
		}
		else if (node.Class == USBDeviceClassses::AMC)
		{
			// ساخت توصیف‌گرهای کارت صدا بر اساس node.AMC به همین روش اضافه می‌شود
			// interfaceCounter به تعداد اینترفیس‌های Audio افزایش می‌یابد
		}
	}

	// ۳. پر کردن هدر اصلی کانفیگ پس از مشخص شدن طول کل
	config->bLength = sizeof(USBConfigurationDescriptor);
	config->bDescriptorType = 0x02;
	config->wTotalLength = offset; // طول پویای کل پکت
	config->bNumInterfaces = interfaceCounter; // تعداد کل اینترفیس‌ها
	config->bConfigurationValue = 1;
	config->iConfiguration = 0;
	config->bmAttributes = Profile.IsSelfPowered ? 0xC0 : 0x80;
	config->bMaxPower = Profile.MaxPowerCurrent / 2;

	return offset; // ارسال طول کل برای HAL_PCD_EP_Transmit
}

void DaisyUSBInterface::DeviceReceive(uint8* Buffer, uint16 Length)
{
	HAL_PCD_EP_Receive(&m_DeviceHandle, USB_EP0_OUT, Buffer, Length);
}

void DaisyUSBInterface::DeviceTransmit(uint8* Buffer, uint16 Length)
{
	HAL_PCD_EP_Transmit(&m_DeviceHandle, USB_EP0_IN, Buffer, Length);
}

#endif