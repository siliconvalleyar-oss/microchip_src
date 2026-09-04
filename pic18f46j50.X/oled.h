#ifndef OLED_H
#define OLED_H

#include <xc.h>
#include <stdint.h>

#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64


extern uint8_t oled_buffer[OLED_WIDTH * (OLED_HEIGHT/8)];

// Fuentes
extern const uint8_t* ActiveFont;
extern uint8_t FontWidth;
extern uint8_t FontHeight;

// I2C
void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Write(uint8_t data);

// OLED básico
void OLED_Init(void);
void OLED_Command(uint8_t cmd);
void OLED_Data(uint8_t data);
void OLED_Clear(void);
void OLED_SetCursor(uint8_t page, uint8_t col);
void OLED_DrawRect(void);
void OLED_DrawLogo(const uint8_t *logo);
void OLED_DrawBuffer(uint8_t *buf);
void OLED_DrawFromSD(void);

// Texto y fuentes
void OLED_DrawChar(uint8_t page, uint8_t col, char c);
void OLED_Print(uint8_t page, uint8_t col, const char *str);
void OLED_SetFont(const uint8_t* font, uint8_t width, uint8_t height);
void OLED_PrintChar(char c, uint8_t x, uint8_t y);
void OLED_PrintText(const char* str, uint8_t x, uint8_t y);

// Buffer
void OLED_DrawColumn(uint8_t x, uint8_t y, uint8_t data);
void OLED_UpdateScreen(void);
void OLED_ClearBuffer(void) ;
#endif