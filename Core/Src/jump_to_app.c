// FILE: bootloader/jump_to_app.c

#include "jump_to_app.h"
#include "stm32f4xx_hal.h"

typedef void (*pAppEntry_t)(void);

bool is_valid_application(void)
{
    uint32_t app_msp = *((uint32_t*)APP_START_ADDRESS);
    uint32_t app_reset = *((uint32_t*)(APP_START_ADDRESS + 4));

    // Check 1: MSP should point to SRAM (0x20000000 - 0x20020000 for STM32F407)
    if ((app_msp >= 0x20000000) && (app_msp <= 0x20020000))
    {
        // Check 2: Reset vector should point to valid flash area (after bootloader, within flash)
        if ((app_reset >= APP_START_ADDRESS) && (app_reset <= 0x080FFFFF))
        {
            // Check 3: Reset vector should point to Thumb code (LSB set to 1)
            if ((app_reset & 0x1) == 1)
            {
                return true;
            }
        }
    }
    return false;
}

void jump_to_application(void)
{
    uint32_t app_stack = *((uint32_t*) APP_START_ADDRESS);
    uint32_t app_reset = *((uint32_t*) (APP_START_ADDRESS + 4));

    // Check if stack pointer is within valid SRAM range
    if ((app_stack < 0x20000000) || (app_stack > 0x20020000))
        return; // invalid MSP — do not jump

    // Disable all interrupts
    __disable_irq();
    
    // Disable all interrupt handlers
    for (int i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // Disable SysTick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    // Reset all peripherals
    HAL_DeInit();
    
    // Reset GPIO pins used by bootloader
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_12|GPIO_PIN_13);
    
    // Reset USB peripheral
    USB_OTG_FS->GCCFG = 0;  // Disable USB peripheral
    
    // Reset RCC to default state
    RCC->CR |= RCC_CR_HSION;                    // Enable HSI
    while(!(RCC->CR & RCC_CR_HSIRDY));          // Wait for HSI ready
    RCC->CFGR = 0x00000000;                     // Reset CFGR
    RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_PLLI2SON); // Disable PLL and PLLI2S
    RCC->CR &= ~RCC_CR_HSEON;                   // Disable HSE
    while((RCC->CR & RCC_CR_PLLRDY) || (RCC->CR & RCC_CR_HSERDY)); // Wait for PLLs and HSE to stop
    RCC->CIR = 0x00000000;                      // Disable all RCC interrupts

    // Clear all pending interrupts
    SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;       // Clear PendSV
    SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;       // Clear SysTick

    // Set vector table offset to application's vector table
    SCB->VTOR = APP_START_ADDRESS;
    __DSB();                                    // Ensure VTOR update completes
    __ISB();                                    // Flush pipeline

    // Reset core registers
    __set_PRIMASK(1);                           // Disable interrupts
    __set_FAULTMASK(1);                         // Disable all exceptions
    __set_CONTROL(0);                           // Switch to privileged mode, MSP stack
    __set_PSP(0);                               // Clear process stack pointer
    
    // Set main stack pointer
    __set_MSP(app_stack);                       // Set main stack pointer
    __DSB();                                    // Data barrier
    
    // Flush FPU state (if used)
//    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
//        __FPU_DISABLE();                        // Disable FPU
//        __set_FPSCR(0);                         // Clear FPU status register
//    #endif
    
    // Final barriers before jump
    __DSB();                                    // Ensure all memory accesses complete
    __ISB();                                    // Flush pipeline
    
    // Jump to application Reset Handler
    pAppEntry_t appEntry = (pAppEntry_t)(app_reset & ~1UL);
    appEntry();                                 // Jump to application
    
    // Should never reach here
    while(1);
}
