#ifdef ON_HARDWARE

#include <stddef.h>

#ifdef BOOT_TYPE
#if BOOT_TYPE < 1 || BOOT_TYPE > 3
#error "BOOT_TYPE with one of the 1=FLASH, 2=SRAM or 3=QSPI must be defined."
#else
#if BOOT_TYPE == 1
#define BOOT_FLASH
#elif BOOT_TYPE == 2
#define BOOT_SRAM
#elif BOOT_TYPE == 3
#define BOOT_QSPI
#endif
#endif
#else
#error "BOOT_TYPE is not defined."
#endif

extern "C"
{
	/*
		This file contains the entry point (Reset_Handler) of your firmware project.
		The reset handled initializes the RAM and calls system library initializers as well as
		the platform-specific initializer and the main() function.
	*/

	extern void* _estack;

	void Reset_Handler();
	void Default_Handler();

	void NMI_Handler()                    __attribute__((weak, alias("Default_Handler")));
	void HardFault_Handler()              __attribute__((weak, alias("Default_Handler")));
	void MemManage_Handler()              __attribute__((weak, alias("Default_Handler")));
	void BusFault_Handler()               __attribute__((weak, alias("Default_Handler")));
	void UsageFault_Handler()             __attribute__((weak, alias("Default_Handler")));
	void SVC_Handler()                    __attribute__((weak, alias("Default_Handler")));
	void DebugMon_Handler()               __attribute__((weak, alias("Default_Handler")));
	void PendSV_Handler()                 __attribute__((weak, alias("Default_Handler")));
	void SysTick_Handler()                __attribute__((weak, alias("Default_Handler")));
	void WWDG_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void PVD_AVD_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void TAMP_STAMP_IRQHandler()          __attribute__((weak, alias("Default_Handler")));
	void RTC_WKUP_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
	void FLASH_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void RCC_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
	void EXTI0_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void EXTI1_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void EXTI2_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void EXTI3_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void EXTI4_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void DMA1_Stream0_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA1_Stream1_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA1_Stream2_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA1_Stream3_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA1_Stream4_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA1_Stream5_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA1_Stream6_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void ADC_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
	void FDCAN1_IT0_IRQHandler()          __attribute__((weak, alias("Default_Handler")));
	void FDCAN2_IT0_IRQHandler()          __attribute__((weak, alias("Default_Handler")));
	void FDCAN1_IT1_IRQHandler()          __attribute__((weak, alias("Default_Handler")));
	void FDCAN2_IT1_IRQHandler()          __attribute__((weak, alias("Default_Handler")));
	void EXTI9_5_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void TIM1_BRK_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
	void TIM1_UP_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void TIM1_TRG_COM_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void TIM1_CC_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void TIM2_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void TIM3_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void TIM4_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void I2C1_EV_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void I2C1_ER_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void I2C2_EV_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void I2C2_ER_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void SPI1_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void SPI2_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void USART1_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void USART2_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void USART3_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void EXTI15_10_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
	void RTC_Alarm_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
	void TIM8_BRK_TIM12_IRQHandler()      __attribute__((weak, alias("Default_Handler")));
	void TIM8_UP_TIM13_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void TIM8_TRG_COM_TIM14_IRQHandler()  __attribute__((weak, alias("Default_Handler")));
	void TIM8_CC_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void DMA1_Stream7_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void FMC_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
	void SDMMC1_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void TIM5_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void SPI3_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void UART4_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void UART5_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void TIM6_DAC_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
	void TIM7_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void DMA2_Stream0_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA2_Stream1_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA2_Stream2_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA2_Stream3_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA2_Stream4_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void ETH_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
	void ETH_WKUP_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
	void FDCAN_CAL_IRQHandler()           __attribute__((weak, alias("Default_Handler")));
	void DMA2_Stream5_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA2_Stream6_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void DMA2_Stream7_IRQHandler()        __attribute__((weak, alias("Default_Handler")));
	void USART6_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void I2C3_EV_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void I2C3_ER_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void OTG_HS_EP1_OUT_IRQHandler()      __attribute__((weak, alias("Default_Handler")));
	void OTG_HS_EP1_IN_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void OTG_HS_WKUP_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void OTG_HS_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void DCMI_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void CRYP_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void HASH_RNG_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
	void FPU_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
	void UART7_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void UART8_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void SPI4_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void SPI5_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void SPI6_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void SAI1_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void LTDC_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void LTDC_ER_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void DMA2D_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void SAI2_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void QUADSPI_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void LPTIM1_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void CEC_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
	void I2C4_EV_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void I2C4_ER_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void SPDIF_RX_IRQHandler()            __attribute__((weak, alias("Default_Handler")));
	void OTG_FS_EP1_OUT_IRQHandler()      __attribute__((weak, alias("Default_Handler")));
	void OTG_FS_EP1_IN_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void OTG_FS_WKUP_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void OTG_FS_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void DMAMUX1_OVR_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void HRTIM1_Master_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void HRTIM1_TIMA_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void HRTIM1_TIMB_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void HRTIM1_TIMC_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void HRTIM1_TIMD_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void HRTIM1_TIME_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void HRTIM1_FLT_IRQHandler()          __attribute__((weak, alias("Default_Handler")));
	void DFSDM1_FLT0_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void DFSDM1_FLT1_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void DFSDM1_FLT2_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void DFSDM1_FLT3_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void SAI3_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void SWPMI1_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void TIM15_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void TIM16_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void TIM17_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void MDIOS_WKUP_IRQHandler()          __attribute__((weak, alias("Default_Handler")));
	void MDIOS_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void JPEG_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void MDMA_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void SDMMC2_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void HSEM1_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void ADC3_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void DMAMUX2_OVR_IRQHandler()         __attribute__((weak, alias("Default_Handler")));
	void BDMA_Channel0_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void BDMA_Channel1_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void BDMA_Channel2_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void BDMA_Channel3_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void BDMA_Channel4_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void BDMA_Channel5_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void BDMA_Channel6_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void BDMA_Channel7_IRQHandler()       __attribute__((weak, alias("Default_Handler")));
	void COMP1_IRQHandler()               __attribute__((weak, alias("Default_Handler")));
	void LPTIM2_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void LPTIM3_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void LPTIM4_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void LPTIM5_IRQHandler()              __attribute__((weak, alias("Default_Handler")));
	void LPUART1_IRQHandler()             __attribute__((weak, alias("Default_Handler")));
	void CRS_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
	void ECC_IRQHandler()                 __attribute__((weak, alias("Default_Handler")));
	void SAI4_IRQHandler()                __attribute__((weak, alias("Default_Handler")));
	void WAKEUP_PIN_IRQHandler()          __attribute__((weak, alias("Default_Handler")));

	typedef void (*pfn_t)(void);

	pfn_t g_pfnVectors[0xa6] __attribute__((section(".isr_vector"), used)) =
	{
		(pfn_t)& _estack,
		&Reset_Handler,
		&NMI_Handler,
		&HardFault_Handler,
		&MemManage_Handler,
		&BusFault_Handler,
		&UsageFault_Handler,
		NULL,
		NULL,
		NULL,
		NULL,
		&SVC_Handler,
		&DebugMon_Handler,
		NULL,
		&PendSV_Handler,
		&SysTick_Handler,
		&WWDG_IRQHandler,
		&PVD_AVD_IRQHandler,
		&TAMP_STAMP_IRQHandler,
		&RTC_WKUP_IRQHandler,
		&FLASH_IRQHandler,
		&RCC_IRQHandler,
		&EXTI0_IRQHandler,
		&EXTI1_IRQHandler,
		&EXTI2_IRQHandler,
		&EXTI3_IRQHandler,
		&EXTI4_IRQHandler,
		&DMA1_Stream0_IRQHandler,
		&DMA1_Stream1_IRQHandler,
		&DMA1_Stream2_IRQHandler,
		&DMA1_Stream3_IRQHandler,
		&DMA1_Stream4_IRQHandler,
		&DMA1_Stream5_IRQHandler,
		&DMA1_Stream6_IRQHandler,
		&ADC_IRQHandler,
		&FDCAN1_IT0_IRQHandler,
		&FDCAN2_IT0_IRQHandler,
		&FDCAN1_IT1_IRQHandler,
		&FDCAN2_IT1_IRQHandler,
		&EXTI9_5_IRQHandler,
		&TIM1_BRK_IRQHandler,
		&TIM1_UP_IRQHandler,
		&TIM1_TRG_COM_IRQHandler,
		&TIM1_CC_IRQHandler,
		&TIM2_IRQHandler,
		&TIM3_IRQHandler,
		&TIM4_IRQHandler,
		&I2C1_EV_IRQHandler,
		&I2C1_ER_IRQHandler,
		&I2C2_EV_IRQHandler,
		&I2C2_ER_IRQHandler,
		&SPI1_IRQHandler,
		&SPI2_IRQHandler,
		&USART1_IRQHandler,
		&USART2_IRQHandler,
		&USART3_IRQHandler,
		&EXTI15_10_IRQHandler,
		&RTC_Alarm_IRQHandler,
		NULL,
		&TIM8_BRK_TIM12_IRQHandler,
		&TIM8_UP_TIM13_IRQHandler,
		&TIM8_TRG_COM_TIM14_IRQHandler,
		&TIM8_CC_IRQHandler,
		&DMA1_Stream7_IRQHandler,
		&FMC_IRQHandler,
		&SDMMC1_IRQHandler,
		&TIM5_IRQHandler,
		&SPI3_IRQHandler,
		&UART4_IRQHandler,
		&UART5_IRQHandler,
		&TIM6_DAC_IRQHandler,
		&TIM7_IRQHandler,
		&DMA2_Stream0_IRQHandler,
		&DMA2_Stream1_IRQHandler,
		&DMA2_Stream2_IRQHandler,
		&DMA2_Stream3_IRQHandler,
		&DMA2_Stream4_IRQHandler,
		&ETH_IRQHandler,
		&ETH_WKUP_IRQHandler,
		&FDCAN_CAL_IRQHandler,
		NULL,
		NULL,
		NULL,
		NULL,
		&DMA2_Stream5_IRQHandler,
		&DMA2_Stream6_IRQHandler,
		&DMA2_Stream7_IRQHandler,
		&USART6_IRQHandler,
		&I2C3_EV_IRQHandler,
		&I2C3_ER_IRQHandler,
		&OTG_HS_EP1_OUT_IRQHandler,
		&OTG_HS_EP1_IN_IRQHandler,
		&OTG_HS_WKUP_IRQHandler,
		&OTG_HS_IRQHandler,
		&DCMI_IRQHandler,
		&CRYP_IRQHandler,
		&HASH_RNG_IRQHandler,
		&FPU_IRQHandler,
		&UART7_IRQHandler,
		&UART8_IRQHandler,
		&SPI4_IRQHandler,
		&SPI5_IRQHandler,
		&SPI6_IRQHandler,
		&SAI1_IRQHandler,
		&LTDC_IRQHandler,
		&LTDC_ER_IRQHandler,
		&DMA2D_IRQHandler,
		&SAI2_IRQHandler,
		&QUADSPI_IRQHandler,
		&LPTIM1_IRQHandler,
		&CEC_IRQHandler,
		&I2C4_EV_IRQHandler,
		&I2C4_ER_IRQHandler,
		&SPDIF_RX_IRQHandler,
		&OTG_FS_EP1_OUT_IRQHandler,
		&OTG_FS_EP1_IN_IRQHandler,
		&OTG_FS_WKUP_IRQHandler,
		&OTG_FS_IRQHandler,
		&DMAMUX1_OVR_IRQHandler,
		&HRTIM1_Master_IRQHandler,
		&HRTIM1_TIMA_IRQHandler,
		&HRTIM1_TIMB_IRQHandler,
		&HRTIM1_TIMC_IRQHandler,
		&HRTIM1_TIMD_IRQHandler,
		&HRTIM1_TIME_IRQHandler,
		&HRTIM1_FLT_IRQHandler,
		&DFSDM1_FLT0_IRQHandler,
		&DFSDM1_FLT1_IRQHandler,
		&DFSDM1_FLT2_IRQHandler,
		&DFSDM1_FLT3_IRQHandler,
		&SAI3_IRQHandler,
		&SWPMI1_IRQHandler,
		&TIM15_IRQHandler,
		&TIM16_IRQHandler,
		&TIM17_IRQHandler,
		&MDIOS_WKUP_IRQHandler,
		&MDIOS_IRQHandler,
		&JPEG_IRQHandler,
		&MDMA_IRQHandler,
		NULL,
		&SDMMC2_IRQHandler,
		&HSEM1_IRQHandler,
		NULL,
		&ADC3_IRQHandler,
		&DMAMUX2_OVR_IRQHandler,
		&BDMA_Channel0_IRQHandler,
		&BDMA_Channel1_IRQHandler,
		&BDMA_Channel2_IRQHandler,
		&BDMA_Channel3_IRQHandler,
		&BDMA_Channel4_IRQHandler,
		&BDMA_Channel5_IRQHandler,
		&BDMA_Channel6_IRQHandler,
		&BDMA_Channel7_IRQHandler,
		&COMP1_IRQHandler,
		&LPTIM2_IRQHandler,
		&LPTIM3_IRQHandler,
		&LPTIM4_IRQHandler,
		&LPTIM5_IRQHandler,
		&LPUART1_IRQHandler,
		NULL,
		&CRS_IRQHandler,
		&ECC_IRQHandler,
		&SAI4_IRQHandler,
		NULL,
		NULL,
		&WAKEUP_PIN_IRQHandler,
	};

	void SystemInit();
	void __libc_init_array();
	int main();

	extern void* _sidata, * _sdata, * _edata;
	extern void* _sbss, * _ebss;

	void __attribute__((noreturn)) Reset_Handler()
	{
		//Normally the CPU should will setup the based on the value from the first entry in the vector table.
		//If you encounter problems with accessing stack variables during initialization, ensure the line below is enabled.
#ifdef sram_layout
		asm("ldr sp, =_estack");
#endif

		void** pSource, ** pDest;
		for (pSource = &_sidata, pDest = &_sdata; pDest != &_edata; pSource++, pDest++)
			*pDest = *pSource;

		for (pDest = &_sbss; pDest != &_ebss; pDest++)
			*pDest = 0;

#ifdef BOOT_FLASH
		SystemInit();
#endif
		__libc_init_array();


		(void)main();
		for (;;);
	}

	void __attribute__((interrupt("UNDEF"), noreturn)) Default_Handler()
	{
		//If you get stuck here, your code is missing a handler for some interrupt.
		//Define a 'DEBUG_DEFAULT_INTERRUPT_HANDLERS' macro via VisualGDB Project Properties and rebuild your project.
		//This will pinpoint a specific missing vector.
		for (;;);
	}
}

#endif