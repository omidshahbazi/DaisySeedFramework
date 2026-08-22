#ifdef ON_HARDWARE

#define STM32H750xx

#if BOOT_TYPE != 1
#define BOOT_APP
#endif

#define STM32H750xx

#include "../Framework/libDaisy/core/startup_stm32h750xx.c"

#endif