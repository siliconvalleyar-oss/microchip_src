#ifndef SSD1306_H
#define SSD1306_H

#include "hal_config.h"

#define SSD1306_I2C_ADDR    0x3C
#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      64

void SSD1306_Init(void);
void SSD1306_Clear(void);
void SSD1306_Update(void);
void SSD1306_SetPixel(uint8_t x, uint8_t y, uint8_t color);
void SSD1306_DrawChar(uint8_t x, uint8_t y, char c);
void SSD1306_PrintString(uint8_t row, uint8_t col, const char* str);
void SSD1306_PrintStringPGM(uint8_t row, uint8_t col, const char* str);
void SSD1306_Fill(uint8_t color);

#endif
