/*
 * File:   oled.c
 * Author: optimus
 *
 * Created on 2 de abril de 2026, 17:04
 */


#include <xc.h>
#include "oled.h"
#include "hardware_cfg.h"
#include "i2c.h"
#include "fonts.h"

//----------------------------------------
// FUNCIONES DEL OLED SSD1306
//----------------------------------------
void OLED_WriteCmd(uint8_t cmd)
{
    I2C_Start();
    I2C_Write(OLED_ADDR);
    I2C_Write(OLED_CMD);
    I2C_Write(cmd);
    I2C_Stop();
}

void OLED_WriteData(uint8_t data)
{
    I2C_Start();
    I2C_Write(OLED_ADDR);
    I2C_Write(OLED_DATA);
    I2C_Write(data);
    I2C_Stop();
}

void OLED_Init(void)
{
    __delay_ms(100);
    
    OLED_WriteCmd(0xAE);  // Display OFF
    __delay_ms(10);
    
    OLED_WriteCmd(0xD5);  // Clock divide ratio
    OLED_WriteCmd(0x80);
        OLED_WriteCmd(0xA8);  // Multiplex ratio
#ifdef DISPLAY_128_X_32
    OLED_WriteCmd(0x1F);
  #elif defined(DISPLAY_128_X_64)
    OLED_WriteCmd(0x3F);
#endif    
    
    OLED_WriteCmd(0xD3);  // Display offset
    OLED_WriteCmd(0x00);
    
    OLED_WriteCmd(0x40);  // Start line
    
    OLED_WriteCmd(0x8D);  // Charge pump
    OLED_WriteCmd(0x14);  // Enable charge pump
    
    OLED_WriteCmd(0x20);  // Memory mode
    OLED_WriteCmd(0x00);
    
    OLED_WriteCmd(0xA1);  // Segment remap
    
    OLED_WriteCmd(0xC8);  // COM scan direction
    
    OLED_WriteCmd(0xDA);  // COM pins
   // OLED_WriteCmd(0x12);
    
 #ifdef DISPLAY_128_X_32
        OLED_WriteCmd(0x02);
  #elif defined(DISPLAY_128_X_64)
        OLED_WriteCmd(0x12);
#endif    
    
    OLED_WriteCmd(0x81);  // Contrast
    OLED_WriteCmd(0xCF);
    
    OLED_WriteCmd(0xD9);  // Precharge period
    OLED_WriteCmd(0xF1);
    
    OLED_WriteCmd(0xDB);  // VCOM deselect
    OLED_WriteCmd(0x40);
    
    OLED_WriteCmd(0xA4);  // Resume to RAM content
    OLED_WriteCmd(0xA6);  // Normal display
    OLED_WriteCmd(0x2E);  // Deactivate scroll
    OLED_WriteCmd(0xAF);  // Display ON
    
    __delay_ms(100);
    OLED_Clear();
}

void OLED_Clear(void)
{
    for(uint8_t page = 0; page < 8; page++) {
        OLED_SetCursor(0, page);
        for(uint8_t col = 0; col < 128; col++) {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_SetCursor(uint8_t x, uint8_t y)
{
    OLED_WriteCmd(0xB0 + y);                    // Set page address
    OLED_WriteCmd(((x & 0xF0) >> 4) | 0x10);   // Set column high address
    OLED_WriteCmd((x & 0x0F) | 0x00);          // Set column low address
}

void OLED_PrintChar(char c)
{
    uint8_t index;
    
    if(c < 32 || c > 90)
        c = 32;
    
    index = c - 32;
    
    for(uint8_t i = 0; i < 5; i++)
    {
        OLED_WriteData(font[index][i]);
    }
    OLED_WriteData(0x00);  // Espacio entre caracteres
}

void OLED_PrintString(const char* str)
{
    while(*str)
    {
        OLED_PrintChar(*str++);
    }
}

void OLED_PrintNumber(uint8_t num)
{
    if(num < 10)
    {
        OLED_PrintChar('0' + num);
    }
    else
    {
        OLED_PrintChar('0' + (num / 10));
        OLED_PrintChar('0' + (num % 10));
    }
}
