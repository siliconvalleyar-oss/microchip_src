#ifndef OLED_H
#define OLED_H

#include "config.h"

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Update(void);
void OLED_PutStr(uint8_t row, uint8_t col, const char *str);
void OLED_FillScreen(uint8_t pattern);

#endif
