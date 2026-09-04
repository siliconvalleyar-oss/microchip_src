#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include <stdbool.h>

bool OLED_Init(void);
void OLED_Clear(void);
void OLED_Update(void);
void OLED_DrawChar(uint8_t x, uint8_t y, char c, uint8_t size, bool color);
void OLED_DrawString(uint8_t x, uint8_t y, const char* str, uint8_t size, bool color);
void OLED_DrawDec32(uint32_t val, uint8_t x, uint8_t y, uint8_t size);

#endif
