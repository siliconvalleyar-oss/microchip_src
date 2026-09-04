#ifndef HAL_LED_H
#define HAL_LED_H

#include "hal_config.h"

#define LED0    0
#define LED1    1

void LED_Init(void);
void LED_On(uint8_t led);
void LED_Off(uint8_t led);
void LED_Toggle(uint8_t led);

#endif
