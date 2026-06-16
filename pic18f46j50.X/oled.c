#include <string.h>
#include "oled.h"
#include "sdcard.h"
#include "fonts.h"

#define _XTAL_FREQ 4000000

uint8_t oled_buffer[OLED_WIDTH * (OLED_HEIGHT/8)];
const uint8_t* ActiveFont = 0;
uint8_t FontWidth = 0;
uint8_t FontHeight = 0;


// ------------------ I2C ------------------
void I2C_Init(void) {
    TRISB0 = 1;   // SDA
    TRISB1 = 1;   // SCL
    SSPCON1 = 0x28;
    SSPCON2 = 0x00;
    SSPSTAT = 0x00;
    SSPADD = 9;   // 100kHz
}

void I2C_Write(uint8_t data) {
    SSPBUF = data;
    while(SSPSTATbits.BF);
    while(SSPCON2bits.ACKSTAT);
}

void I2C_Start(void) {
    SEN = 1;
    while(SEN);
}

void I2C_Stop(void) {
    PEN = 1;
    while(PEN);
}

// ------------------ OLED básico ------------------
void OLED_Command(uint8_t cmd) {
    I2C_Start();
    I2C_Write(OLED_ADDR << 1);
    I2C_Write(0x00); // control byte
    I2C_Write(cmd);
    I2C_Stop();
}

void OLED_Data(uint8_t data) {
    I2C_Start();
    I2C_Write(OLED_ADDR << 1);
    I2C_Write(0x40); // control byte para data
    I2C_Write(data);
    I2C_Stop();
}

void OLED_Init(void) {
    __delay_ms(100);
    OLED_Command(0xAE); OLED_Command(0x20); OLED_Command(0x10);
    OLED_Command(0xB0); OLED_Command(0xC8); OLED_Command(0x00);
    OLED_Command(0x10); OLED_Command(0x40); OLED_Command(0x81);
    OLED_Command(0xFF); OLED_Command(0xA1); OLED_Command(0xA6);
    OLED_Command(0xA8); OLED_Command(0x3F); OLED_Command(0xA4);
    OLED_Command(0xD3); OLED_Command(0x00); OLED_Command(0xD5);
    OLED_Command(0xF0); OLED_Command(0xD9); OLED_Command(0x22);
    OLED_Command(0xDA); OLED_Command(0x12); OLED_Command(0xDB);
    OLED_Command(0x20); OLED_Command(0x8D); OLED_Command(0x14);
    OLED_Command(0xAF);
}

void OLED_Clear(void) {
    for(uint8_t page = 0; page < 8; page++) {
        OLED_Command(0xB0 + page);
        OLED_Command(0x00);
        OLED_Command(0x10);
        for(uint8_t i = 0; i < 128; i++)
            OLED_Data(0x00);
    }
}

void OLED_SetCursor(uint8_t page, uint8_t col) {
    OLED_Command(0xB0 + page);
    OLED_Command(((col & 0xF0) >> 4) | 0x10);
    OLED_Command((col & 0x0F) | 0x01);
}

void OLED_DrawRect(void) {
    for(uint8_t page = 0; page < 8; page++) {
        OLED_Command(0xB0 + page);
        OLED_Command(0x00);
        OLED_Command(0x10);
        for(uint8_t col = 0; col < 128; col++) {
            if(page == 0 || page == 7 || col == 0 || col == 127)
                OLED_Data(0xFF);
            else
                OLED_Data(0x00);
        }
    }
}


void OLED_DrawBuffer(uint8_t *buf) {
    uint16_t i = 0;
    for(uint8_t page = 0; page < 8; page++) {
        OLED_Command(0xB0 + page);
        OLED_Command(0x00);
        OLED_Command(0x10);
        for(uint8_t col = 0; col < 128; col++)
            OLED_Data(buf[i++]);
    }
}

void OLED_DrawFromSD() {
    uint8_t page, col, data;
    for(page=0; page<8; page++) {
        OLED_Command(0xB0 + page);
        OLED_Command(0x00);
        OLED_Command(0x10);
        for(col=0; col<128; col++) {
            data = SD_ReadByte();
            OLED_Data(data);
        }
    }
}

// ------------------ Texto ------------------
void OLED_DrawChar(uint8_t page, uint8_t col, char c) {
    uint8_t index = c - 32;
    if(index > 94) return;
    for(uint8_t i = 0; i < 7; i++) {
        OLED_SetCursor(page, col + i);
        OLED_Data(Font_Three[index * 7 + i]);
    }
}

void OLED_Print(uint8_t page, uint8_t col, const char *str) {
    while(*str) {
        OLED_DrawChar(page, col, *str++);
        col += 8;
        if(col > 121) { col = 0; page++; if(page > 7) break; }
    }
}

// ------------------ Buffer avanzado ------------------
void OLED_DrawColumn(uint8_t x, uint8_t y, uint8_t data) {
    if(x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    uint16_t index = x + (y/8) * OLED_WIDTH;
    oled_buffer[index] = data;
}

void OLED_UpdateScreen(void) {
    for(uint16_t i=0; i < sizeof(oled_buffer); i++)
        OLED_Data(oled_buffer[i]);
}

void OLED_SetFont(const uint8_t* font, uint8_t width, uint8_t height) {
    ActiveFont = font;
    FontWidth = width;
    FontHeight = height;
}

void OLED_PrintChar(char c, uint8_t x, uint8_t y) {
    if(!ActiveFont) return;
    uint16_t index = (c - 32) * FontWidth;
    for(uint8_t i=0; i < FontWidth; i++)
        OLED_DrawColumn(x + i, y, ActiveFont[index + i]);
}

void OLED_PrintText(const char* str, uint8_t x, uint8_t y) {
    while(*str) {
        OLED_PrintChar(*str++, x, y);
        x += FontWidth;
        if(x + FontWidth > OLED_WIDTH) { x = 0; y += FontHeight; }
    }
}
/*

void OLED_DrawLogo(const uint8_t *logo) {
    memcpy(oled_buffer, logo, OLED_WIDTH * (OLED_HEIGHT/8));
    OLED_UpdateScreen();
}*/

void OLED_DrawLogo(const uint8_t *logo) {
    // Logo de 128x64 píxeles, bitmap normal: 1 bit por pixel, fila por fila
    for(uint8_t page = 0; page < 8; page++) {       // 8 páginas
        for(uint8_t col = 0; col < 128; col++) {    // 128 columnas
            uint8_t byte = 0;
            for(uint8_t bit = 0; bit < 8; bit++) {  // cada bit = pixel vertical
                uint8_t y = page * 8 + bit;
                uint16_t index = y * 128 + col;     // index fila por fila
                uint8_t pixel = (logo[index / 8] >> (7 - (index % 8))) & 0x01;
                if(pixel) byte |= (1 << bit);      // bit vertical en el byte
            }
            oled_buffer[page*128 + col] = byte;
        }
    }
    OLED_UpdateScreen();
}


// Limpia el buffer antes de dibujar cualquier cosa nueva
void OLED_ClearBuffer(void) {
    memset(oled_buffer, 0x00, sizeof(oled_buffer));
}