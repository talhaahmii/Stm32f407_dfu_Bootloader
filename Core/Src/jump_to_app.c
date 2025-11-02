// FILE: bootloader/jump_to_app.c

#include "jump_to_app.h"
#include "stm32f4xx_hal.h"

typedef void (*pAppEntry_t)(void);

bool is_valid_application(void)
{
    uint32_t app_msp = *((uint32_t*)APP_START_ADDRESS);
    uint32_t app_reset = *((uint32_t*)(APP_START_ADDRESS + 4));

    // Check 1: MSP should point to SRAM (0x20000000 - 0x2001FFFF for STM32F407)
    if ((app_msp >= 0x20000000) && (app_msp <= 0x2001FFFF))
    {
        // Check 2: Reset vector should point to valid flash area (after bootloader, within flash)
        if ((app_reset >= APP_START_ADDRESS) && (app_reset <= 0x080FFFFF))
        {
            // Check 3: Reset vector should be properly aligned
            if ((app_reset & 0x1) == 0)
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

    if ((app_stack & 0x2FFE0000U) != APP_STACK_SRANGE_MIN)
        return; // invalid MSP — do not jump

    // Deinit HAL and peripherals to safe state
    __disable_irq();

    HAL_RCC_DeInit();
    HAL_DeInit();

    // Disable SysTick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    // Set vector table to application
    SCB->VTOR = APP_START_ADDRESS;

    // Set Main Stack Pointer to application's stack
    __set_MSP(app_stack);

    // Jump to application Reset Handler
    pAppEntry_t appEntry = (pAppEntry_t)app_reset;
    appEntry();
}
