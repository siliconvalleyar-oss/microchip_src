/**
 * @file    image_266.h
 * @brief   Imagen de ejemplo para display 2.66" (296x152)
 * 
 * @note    Esta imagen muestra un patrón de cuadrícula y círculos
 *          Puedes reemplazar con tu propia imagen binaria
 */

#ifndef IMAGE_266_H
#define IMAGE_266_H

#include <stdint.h>

// Imagen de prueba: cuadrícula + borde + patrones
// Tamaño: 5624 bytes (296x152/8)
static const uint8_t testImage_266[5624] = {
    // Borde superior - línea completa negra
    [0 ... 36] = 0xFF,           // Fila 0: todos negros
    [37 ... 73] = 0xFF,          // Fila 1
    [74 ... 110] = 0xFF,         // Fila 2
    [111 ... 147] = 0xFF,        // Fila 3
    
    // Patrón de ajedrez en el centro
    // Las primeras 4 filas ya están hechas, continuamos...
};

// Función para generar imagen de prueba programáticamente
static inline void generateTestImage_266(uint8_t* buffer) {
    uint16_t width = 296;
    uint16_t height = 152;
    uint16_t bytesPerRow = width / 8;
    
    // Limpiar buffer (blanco)
    memset(buffer, 0x00, 5624);
    
    // Dibujar borde negro
    for (uint16_t x = 0; x < width; x++) {
        // Borde superior e inferior
        uint32_t topByte = 0 * bytesPerRow + (x / 8);
        uint32_t bottomByte = (height - 1) * bytesPerRow + (x / 8);
        uint8_t bitMask = 0x80 >> (x % 8);
        
        buffer[topByte] |= bitMask;
        buffer[bottomByte] |= bitMask;
    }
    
    for (uint16_t y = 0; y < height; y++) {
        // Borde izquierdo y derecho
        uint32_t leftByte = y * bytesPerRow + (0 / 8);
        uint32_t rightByte = y * bytesPerRow + ((width - 1) / 8);
        uint8_t leftMask = 0x80 >> (0 % 8);
        uint8_t rightMask = 0x80 >> ((width - 1) % 8);
        
        buffer[leftByte] |= leftMask;
        buffer[rightByte] |= rightMask;
    }
    
    // Dibujar patrón de ajedrez en el centro (área de 200x100)
    for (uint16_t y = 26; y < 126; y++) {
        for (uint16_t x = 48; x < 248; x++) {
            if (((x + y) & 1) == 0) {  // Patrón de ajedrez
                uint32_t byteIndex = y * bytesPerRow + (x / 8);
                uint8_t bitMask = 0x80 >> (x % 8);
                buffer[byteIndex] |= bitMask;
            }
        }
    }
    
    // Dibujar línea horizontal central
    for (uint16_t x = 0; x < width; x++) {
        uint32_t byteIndex = (height / 2) * bytesPerRow + (x / 8);
        uint8_t bitMask = 0x80 >> (x % 8);
        buffer[byteIndex] |= bitMask;
    }
    
    // Dibujar línea vertical central
    for (uint16_t y = 0; y < height; y++) {
        uint32_t byteIndex = y * bytesPerRow + ((width / 2) / 8);
        uint8_t bitMask = 0x80 >> ((width / 2) % 8);
        buffer[byteIndex] |= bitMask;
    }
}

#endif /* IMAGE_266_H */
