/*
#ifndef OLED_H
#define OLED_H

#include <xc.h>
#include <stdint.h>

// ============================================================================
//  Definiciones del OLED (128x32)
// ============================================================================
#define OLED_ADDR           0x78    // Dirección I2C (0x3C << 1)
#define OLED_WIDTH          128
#define OLED_HEIGHT         32
#define OLED_PAGES          4       // 32/8 = 4 páginas

// ============================================================================
//  Variables externas
// ============================================================================
extern uint8_t oled_buffer[OLED_WIDTH * OLED_PAGES];

// ============================================================================
//  Prototipos I2C
// ============================================================================
void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
uint8_t I2C_Write(uint8_t data);
void I2C_Wait(void);

// ============================================================================
//  Prototipos OLED
// ============================================================================
void OLED_Init(void);
void OLED_Command(uint8_t cmd);
void OLED_PrintChar(char c, uint8_t x, uint8_t y);
void OLED_PrintText(const char* str, uint8_t x, uint8_t y);
void OLED_UpdateScreen(void);
void OLED_ClearBuffer(void);

#endif

*/