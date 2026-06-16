/**
 * @file    oled.c
 * @brief   Implementación para OLED SSD1306 128x32
 * @version 1.0
 */
/*
#include "oled.h"
#include <stdio.h>

#define _XTAL_FREQ  4000000   // 4 MHz

uint8_t oled_buffer[OLED_WIDTH * OLED_PAGES];

// Fuente 5x7
static const uint8_t font[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // espacio
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}, // 9
    {0x7E,0x11,0x11,0x11,0x7E}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x09,0x01}, // F
    {0x3E,0x41,0x49,0x49,0x7A}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x0C,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x7F,0x20,0x18,0x20,0x7F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x07,0x08,0x70,0x08,0x07}, // Y
    {0x61,0x51,0x49,0x45,0x43}, // Z
};

// ============================================================================
//  Funciones I2C
// ============================================================================

void I2C_Wait(void) {
    // Esperar a que el bus esté libre
    while ((SSP1STAT & 0x04) || (SSP1CON2 & 0x1F));
}

void I2C_Init(void) {
    // Configurar pines RB0 (SCL) y RB1 (SDA) como entradas
    TRISBbits.TRISB0 = 1;
    TRISBbits.TRISB1 = 1;
    
    // Configurar módulo MSSP1 como I2C maestro
    SSP1CON1 = 0x28;        // Habilitar SSP, modo I2C maestro
    SSP1ADD = 9;            // 400 kHz con Fosc = 4 MHz? Ajustar
    SSP1STAT = 0x00;
}

void I2C_Start(void) {
    I2C_Wait();
    SSP1CON2bits.SEN = 1;
    while(SSP1CON2bits.SEN);
}

void I2C_Stop(void) {
    I2C_Wait();
    SSP1CON2bits.PEN = 1;
    while(SSP1CON2bits.PEN);
}

uint8_t I2C_Write(uint8_t data) {
    I2C_Wait();
    SSP1BUF = data;
    while(SSP1STATbits.BF);
    while(SSP1CON2bits.ACKSTAT);
    return !SSP1CON2bits.ACKSTAT;
}

// ============================================================================
//  Funciones OLED
// ============================================================================

void OLED_Command(uint8_t cmd) {
    I2C_Start();
    I2C_Write(OLED_ADDR);
    I2C_Write(0x00);        // Byte de control: comando
    I2C_Write(cmd);
    I2C_Stop();
}

void OLED_Init(void) {
    // Delay de estabilización
    for(uint16_t i = 0; i < 100; i++) {
        NOP(); NOP(); NOP(); NOP();
    }
    
    // Secuencia de inicialización para SSD1306 128x32
    OLED_Command(0xAE);     // Display OFF
    OLED_Command(0xD5);     // Frecuencia de oscilador
    OLED_Command(0x80);
    OLED_Command(0xA8);     // Multiplexor
    OLED_Command(0x1F);     // 32 líneas (para 128x32)
    OLED_Command(0xD3);     // Offset de display
    OLED_Command(0x00);
    OLED_Command(0x40);     // Línea de inicio
    OLED_Command(0x8D);     // Regulador de carga
    OLED_Command(0x14);     // Habilita regulador de carga
    OLED_Command(0x20);     // Modo de direccionamiento
    OLED_Command(0x00);     // Horizontal
    OLED_Command(0xA1);     // Segment remap
    OLED_Command(0xC8);     // Com remap
    OLED_Command(0xDA);     // Pines de hardware COM
    OLED_Command(0x02);     // Para 128x32
    OLED_Command(0x81);     // Contraste
    OLED_Command(0xCF);
    OLED_Command(0xD9);     // Pre-carga
    OLED_Command(0xF1);
    OLED_Command(0xDB);     // VCOM deseleccionado
    OLED_Command(0x40);
    OLED_Command(0xA4);     // Display ON (normal)
    OLED_Command(0xA6);     // Display normal
    OLED_Command(0x2E);     // Desactivar desplazamiento
    OLED_Command(0xAF);     // Display ON
}

void OLED_SetCursor(uint8_t page, uint8_t col) {
    OLED_Command(0xB0 + page);
    OLED_Command(col & 0x0F);
    OLED_Command(0x10 | (col >> 4));
}

void OLED_PrintChar(char c, uint8_t x, uint8_t y) {
    if(x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    uint8_t idx = 0;
    
    if(c >= '0' && c <= '9') {
        idx = c - '0' + 1;
    } else if(c >= 'A' && c <= 'Z') {
        idx = c - 'A' + 11;
    } else if(c == ' ') {
        idx = 0;
    } else {
        return;
    }
    
    for(uint8_t i = 0; i < 5; i++) {
        if(x + i < OLED_WIDTH) {
            uint8_t col = font[idx][i];
            for(uint8_t j = 0; j < 7; j++) {
                if(col & (1 << j)) {
                    if(page == (y + j) / 8) {
                        uint8_t new_bit = (y + j) % 8;
                        oled_buffer[page * OLED_WIDTH + x + i] |= (1 << new_bit);
                    }
                }
            }
        }
    }
}

void OLED_PrintText(const char* str, uint8_t x, uint8_t y) {
    while(*str && x < OLED_WIDTH) {
        OLED_PrintChar(*str++, x, y);
        x += 6;
    }
}

void OLED_UpdateScreen(void) {
    for(uint8_t page = 0; page < OLED_PAGES; page++) {
        OLED_SetCursor(page, 0);
        
        I2C_Start();
        I2C_Write(OLED_ADDR);
        I2C_Write(0x40);        // Modo datos
        
        for(uint8_t col = 0; col < OLED_WIDTH; col++) {
            I2C_Write(oled_buffer[page * OLED_WIDTH + col]);
        }
        
        I2C_Stop();
    }
}

void OLED_ClearBuffer(void) {
    for(uint16_t i = 0; i < (OLED_WIDTH * OLED_PAGES); i++) {
        oled_buffer[i] = 0x00;
    }
}
 * */