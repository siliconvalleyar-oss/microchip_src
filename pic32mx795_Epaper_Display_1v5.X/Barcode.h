#ifndef BARCODE_H
#define BARCODE_H

#include <stdint.h>
#include <string.h>

class Barcode {
public:
    // Dibuja un código de barras estilo Code 39 simple
    static void drawCode39(uint8_t* buffer, uint16_t x, uint16_t y, 
                           uint16_t height, const char* text, 
                           uint8_t color, uint8_t bgColor);
};

#endif