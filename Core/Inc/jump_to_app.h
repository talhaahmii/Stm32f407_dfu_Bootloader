// FILE: bootloader/jump_to_app.h

#ifndef JUMP_TO_APP_H
#define JUMP_TO_APP_H

#include <stdint.h>
#include <stdbool.h>

#define APP_START_ADDRESS  ((uint32_t)0x08008000U)
#define APP_STACK_SRANGE_MIN 0x20000000U

bool is_valid_application(void);
void jump_to_application(void);

#endif // JUMP_TO_APP_H
