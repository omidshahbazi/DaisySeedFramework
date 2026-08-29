#ifdef ON_HARDWARE

#include "DaisySeedFramework/DaisyInclude.h"

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
}

#endif