#include "EPD_Driver.h"
#include "EPD_Graphics.h"
#include "HardwareProfile.h"
#include "Terminal16x24e.h"
#include <string.h>
#include <xc.h>
#include <stdint.h>
#include <stdio.h>

static uint8_t previousBW[5624];
static uint8_t imageBW[5624] = {0};
static uint8_t imageRed[5624] = {0};

__attribute__((section(".heap"))) uint8_t _heap[4096];

#pragma config FNOSC = PRIPLL, POSCMOD = HS, FPLLIDIV = DIV_2, FPLLMUL = MUL_20
#pragma config FPLLODIV = DIV_1, FPBDIV = DIV_1, FWDTEN = OFF, ICESEL = ICS_PGx1, CP = OFF

const pins_t boardConfig = {
    .panelBusy = 32,
    .panelDC = 33,
    .panelReset = 34,
    .panelCS = 35,
    .panelON_EXT2 = NOT_CONNECTED,
    .panelSPI43_EXT2 = NOT_CONNECTED,
    .flashCS = NOT_CONNECTED
};

static inline uint32_t ReadCoreTimer(void) {
    uint32_t count;
    __asm__ volatile("mfc0 %0, $9" : "=r"(count));
    return count;
}

void led_init(void) {
    TRISECLR = (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2);
    LED0_OFF(); LED1_OFF(); LED2_OFF(); LED3_OFF();
    LED4_OFF(); LED5_OFF(); LED6_OFF(); LED7_OFF();
    TRISDCLR = (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2);
    LED8_OFF(); LED9_OFF(); LED10_OFF(); LED11_OFF();
    LED12_OFF(); LED13_OFF(); LED14_OFF(); LED15_OFF();
}

// ==================== BARCODE (igual que antes) ====================
class Barcode {
public:
    static void drawCode39(uint8_t* buffer, uint16_t x, uint16_t y, 
                           uint16_t height, const char* text, 
                           uint8_t color, uint8_t bgColor);
};

static const uint16_t code39Patterns[43] = {
    0x034, 0x121, 0x061, 0x160, 0x031, 0x130, 0x070, 0x025, 0x124, 0x064,
    0x025, 0x124, 0x064, 0x109, 0x049, 0x148, 0x019, 0x118, 0x058, 0x00D,
    0x10C, 0x04C, 0x14C, 0x01C, 0x11C, 0x05C, 0x103, 0x043, 0x142, 0x013,
    0x112, 0x052, 0x007, 0x106, 0x046, 0x146, 0x016, 0x116, 0x056, 0x00E,
    0x10E, 0x04E, 0x14E
};

static char code39_charToIndex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    if (c == '-') return 36;
    if (c == '.') return 37;
    if (c == ' ') return 38;
    if (c == '*') return 39;
    if (c == '$') return 40;
    if (c == '/') return 41;
    if (c == '+') return 42;
    return 38;
}

void Barcode::drawCode39(uint8_t* buffer, uint16_t x, uint16_t y,
                         uint16_t height, const char* text,
                         uint8_t color, uint8_t bgColor) {
    char encoded[128];
    encoded[0] = '*';
    strcpy(encoded + 1, text);
    strcat(encoded, "*");
    uint16_t currentX = x;
    for (uint16_t i = 0; encoded[i]; i++) {
        char idx = code39_charToIndex(encoded[i]);
        uint16_t pattern = code39Patterns[idx];
        for (uint8_t mod = 0; mod < 9; mod++) {
            uint8_t width = 1;
            if (pattern & (1 << (8 - mod))) {
                epd_drawRect(buffer, currentX, y, width, height, color, 1);
            } else if (bgColor != color) {
                epd_drawRect(buffer, currentX, y, width, height, bgColor, 1);
            }
            currentX += width;
        }
        currentX += 1;
    }
}

// ==================== DIBUJO DEL RELOJ (fuente grande) ====================
void drawClock(uint8_t* buffer, uint8_t h, uint8_t m, uint8_t s) {
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", h, m, s);
    uint16_t strWidth = strlen(timeStr) * 16;   // 16 píxeles por carácter
    uint16_t x = (EPD_WIDTH - strWidth) / 2;
    uint16_t y = (EPD_HEIGHT - 24) / 2;
    
    // Borrar el área del reloj (fondo negro)
    epd_drawRect(buffer, x, y, strWidth, 24, COLOR_BLACK, 1);
    
    // Dibujar cada carácter con la fuente de 16x24
    for (uint16_t i = 0; i < strlen(timeStr); i++) {
        epd_drawChar16x24(buffer, x + i * 16, y, timeStr[i], COLOR_WHITE, COLOR_BLACK);
    }
}

void drawBarcode(uint8_t* buffer, const char* text) {
    Barcode::drawCode39(buffer, 10, 130, 20, text, COLOR_BLACK, COLOR_WHITE);
}
// Prueba de fuente Terminal16x24e
// Modo = 0: interpretación estándar (16 columnas × 24 filas, MSB first)
// Modo = 1: alternativa (intercambia filas y columnas, o invierte bits)
void testFont(uint8_t mode) {
    // Limpiar buffer completamente (fondo blanco)
    epd_clearBuffer(imageBW, 5624, COLOR_WHITE);
    
    const char* text = "ABCDE";
    uint16_t x = 0;
    uint16_t y = 0;
    uint8_t color = COLOR_BLACK;
    uint8_t bgColor = COLOR_WHITE;
    
    for (uint16_t i = 0; i < strlen(text); i++) {
        char c = text[i];
        if (c < 32 || c > 255) continue;
        uint8_t index = c - 32;
        
        // Dibujar cada carácter según el modo
        for (uint8_t col = 0; col < 16; col++) {
            for (uint8_t row = 0; row < 24; row++) {
                uint8_t byteIndex, bitMask;
                if (mode == 0) {
                    // Modo estándar: asume que la fuente está en columnas de 3 bytes (24 bits)
                    byteIndex = col * 3 + (row / 8);
                    bitMask = 0x80 >> (row % 8);
                } else {
                    // Modo alternativo: prueba intercambiando fila/columna o invirtiendo bit
                    // Por ejemplo, tratar como filas de 16 bits (2 bytes por fila) pero la fuente tiene 48 bytes, no es exacto.
                    // Mejor: invertir el orden de los bits (LSB first)
                    byteIndex = col * 3 + (row / 8);
                    bitMask = 1 << (row % 8);   // LSB first
                }
                
                if (Terminal16x24e[index][byteIndex] & bitMask) {
                    epd_drawPixel(imageBW, x + col, y + row, color);
                } else if (bgColor != color) {
                    epd_drawPixel(imageBW, x + col, y + row, bgColor);
                }
            }
        }
        x += 16;  // avanzar 16 píxeles para el siguiente carácter
    }
    
    // Actualizar pantalla (usar globalUpdate para ver resultado claro)

}




// ==================== MAIN ====================
int main(void) {
    CHECONbits.PFMWS = 2;
    CHECONbits.PREFEN = 1;
    INTCONbits.MVEC = 1;
    __builtin_enable_interrupts();

    spi_init();
    led_init();
    LED0_ON();

    EPD_Driver display(eScreen_EPD_266, boardConfig);
    delayMs(100);
    display.COG_initial();

    // 1. Fondo negro completo (primera imagen)
    epd_clearBuffer(imageBW, 5624, COLOR_BLACK);
    display.globalUpdate(imageBW, imageBW);
    delayMs(500);
    
    
    
    
    // Probar modo 0
testFont(0);
    display.globalUpdate(imageBW, imageBW);
    delayMs(5000);  // mantener 5 segundos para observar
//delayMs(5000);

// Probar modo 1
testFont(1);
    display.globalUpdate(imageBW, imageBW);
    delayMs(5000);  // mantener 5 segundos para observar
//delayMs(5000);



    
    
    // Copiar el estado inicial a previousBW
    memcpy(previousBW, imageBW, 5624);
    
    uint32_t startTicks = ReadCoreTimer();
    uint8_t horas = 0, minutos = 0, segundos = 0;
    char barcodeText[10];
    
    while (1) {
        uint32_t nowTicks = ReadCoreTimer();
        uint32_t elapsedTicks = nowTicks - startTicks;
        uint32_t elapsedSeconds = elapsedTicks / (SYS_FREQ / 2);
        
        // Convertir a horas, minutos, segundos
        uint32_t total = elapsedSeconds;
        uint8_t newHoras = (total / 3600) % 24;
        uint8_t newMinutos = (total / 60) % 60;
        uint8_t newSegundos = total % 60;
        
        // Si cambió algún valor, actualizar pantalla
        if (newHoras != horas || newMinutos != minutos || newSegundos != segundos) {
            horas = newHoras;
            minutos = newMinutos;
            segundos = newSegundos;
            
            // Dibujar en el buffer actual
            drawClock(imageBW, horas, minutos, segundos);
            sprintf(barcodeText, "%02d%02d%02d", horas, minutos, segundos);
            drawBarcode(imageBW, barcodeText);
            
            // Actualizar con fast update (suave)
            display.fastUpdate(previousBW, imageBW);
        }
        
        // Pequeña espera para no saturar el bus
        delayMs(50);
//        LED0_TOGGLE();  // si tienes definido LED0_TOGGLE, o haz LED0_OFF(); delayMs(50); LED0_ON();
    }
    return 0;
}