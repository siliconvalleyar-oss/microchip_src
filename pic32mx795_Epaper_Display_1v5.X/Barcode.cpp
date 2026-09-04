#include "Barcode.h"
#include "EPD_Graphics.h"

// Tabla de patrones para Code 39 (0=blanco, 1=negro)
// Cada carácter son 9 módulos (5 barras y 4 espacios), ancho variable
static const uint16_t code39Patterns[43] = {
    // 0-9
    0x034, 0x121, 0x061, 0x160, 0x031, 0x130, 0x070, 0x025, 0x124, 0x064,
    // A-Z
    0x025, 0x124, 0x064, 0x109, 0x049, 0x148, 0x019, 0x118, 0x058, 0x00D,
    0x10C, 0x04C, 0x14C, 0x01C, 0x11C, 0x05C, 0x103, 0x043, 0x142, 0x013,
    // otros
    0x112, 0x052, 0x007, 0x106, 0x046, 0x146, 0x016, 0x116, 0x056, 0x00E,
    0x10E, 0x04E, 0x14E
};

static char code39_charToIndex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    if (c == '-') return 36;
    if (c == '.') return 37;
    if (c == ' ') return 38;
    if (c == '*') return 39;  // carácter de inicio/fin
    if (c == '$') return 40;
    if (c == '/') return 41;
    if (c == '+') return 42;
    return 38; // espacio por defecto
}

void Barcode::drawCode39(uint8_t* buffer, uint16_t x, uint16_t y,
                         uint16_t height, const char* text,
                         uint8_t color, uint8_t bgColor) {
    // Añadir asteriscos al inicio y final
    char encoded[128];
    encoded[0] = '*';
    strcpy(encoded + 1, text);
    strcat(encoded, "*");
    
    uint16_t currentX = x;
    for (uint16_t i = 0; encoded[i]; i++) {
        char idx = code39_charToIndex(encoded[i]);
        uint16_t pattern = code39Patterns[idx];
        
        // Dibujar 9 módulos por carácter
        for (uint8_t mod = 0; mod < 9; mod++) {
            uint16_t width = 1;
            // Si el módulo es negro (bit 1) y no es el último, puede ser ancho (pero simplificamos ancho 2)
            // Para simplificar, todos los módulos de ancho 1 (código de barras simple)
            if (pattern & (1 << (8 - mod))) {
                // Barra negra
                epd_drawRect(buffer, currentX, y, width, height, color, 1);
            } else {
                // Espacio blanco (opcional, si bgColor != color)
                if (bgColor != color)
                    epd_drawRect(buffer, currentX, y, width, height, bgColor, 1);
            }
            currentX += width;
        }
        // Espacio entre caracteres (1 módulo blanco)
        currentX += 1;
    }
}