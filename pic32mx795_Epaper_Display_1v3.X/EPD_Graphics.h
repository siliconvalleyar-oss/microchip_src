/**
 * @file    EPD_Graphics.h
 * @brief   Funciones gráficas para E-Paper display - Versión Raspberry Pi
 * @version 2.0 (sincronizada con RPi)
 */

#ifndef EPD_GRAPHICS_H
#define EPD_GRAPHICS_H

#include <stdint.h>
#include <string.h>
#include "EPD_Driver.h"

/*==============================================================================
 * CONSTANTES PARA DISPLAY 2.66" (296x152)
 *============================================================================*/
#define EPD_WIDTH       296
#define EPD_HEIGHT      152
#define EPD_BYTES_PER_ROW   37      // 296 / 8 = 37 (no se usa realmente, se usa column-major)
#define EPD_BUFFER_SIZE     5624    // 296 * 152 / 8

/*==============================================================================
 * COLORES (monocromo)
 *============================================================================*/
#define COLOR_WHITE     0x00    // Píxel blanco (0)
#define COLOR_BLACK     0xFF    // Píxel negro (1)

/*==============================================================================
 * PROTOTIPOS DE FUNCIONES (misma interfaz que antes)
 *============================================================================*/

void epd_clearBuffer(uint8_t* buffer, uint32_t size, uint8_t color);
void epd_drawPixel(uint8_t* buffer, uint16_t x, uint16_t y, uint8_t color);
uint8_t epd_getPixel(const uint8_t* buffer, uint16_t x, uint16_t y);
void epd_drawHLine(uint8_t* buffer, uint16_t x0, uint16_t x1, uint16_t y, uint8_t color);
void epd_drawVLine(uint8_t* buffer, uint16_t x, uint16_t y0, uint16_t y1, uint8_t color);
void epd_drawRect(uint8_t* buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color, uint8_t fill);
void epd_drawChar(uint8_t* buffer, uint16_t x, uint16_t y, char c, uint8_t color, uint8_t bgColor);
void epd_drawString(uint8_t* buffer, uint16_t x, uint16_t y, const char* str, uint8_t color, uint8_t bgColor);
void epd_loadImage(uint8_t* buffer, const uint8_t* imageData, uint32_t size);
void epd_invertBuffer(uint8_t* buffer, uint32_t size);

#endif /* EPD_GRAPHICS_H */
