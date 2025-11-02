// FILE: bootloader/jump_to_app.h

#ifndef JUMP_TO_APP_H
#define JUMP_TO_APP_H

#include <stdint.h>
#include <stdbool.h>

#define APP_START_ADDRESS  ((uint32_t)0x08010000U)  // 64KB (0x10000) offset for bootloader
#define APP_STACK_SRANGE_MIN 0x20000000U

bool is_valid_application(void);
void jump_to_application(void);

#endif // JUMP_TO_APP_H
