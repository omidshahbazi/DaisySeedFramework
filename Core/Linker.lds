#ifdef BOOT_TYPE
	#if BOOT_TYPE == 1
		INCLUDE "../Framework/Libraries/libDaisy/core/STM32H750IB_flash.lds"
	#elif BOOT_TYPE == 2
		INCLUDE "../Framework/Libraries/libDaisy/core/STM32H750IB_sram.lds"
	#elif BOOT_TYPE == 3
		INCLUDE "../Framework/Libraries/libDaisy/core/STM32H750IB_qspi.lds"
	#else
		#error "BOOT_TYPE with one of the 1=FLASH, 2=SRAM or 3=QSPI must be defined."
	#endif
#else
	#error "BOOT_TYPE is not defined."
#endif