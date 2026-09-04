#ifndef HAL_BUTTON_H
#define HAL_BUTTON_H

#include "hal_config.h"

#define BUTTON_PRESSED      (1 << 0)
#define BUTTON_RELEASED     (1 << 1)

void BUTTON_Init(void);
uint8_t BUTTON_GetState(uint8_t button);
uint8_t BUTTON_GetDebounced(uint8_t button);

#endif
