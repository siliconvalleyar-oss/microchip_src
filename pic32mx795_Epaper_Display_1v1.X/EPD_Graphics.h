/**
 * @file    EPD_Graphics.h
 * @brief   Funciones gráficas para E-Paper display
 * @version 1.0
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
#define EPD_BYTES_PER_ROW   37      // 296 / 8 = 37
#define EPD_BUFFER_SIZE     5624    // 296 * 152 / 8

/*==============================================================================
 * COLORES (monocromo)
 *============================================================================*/
#define COLOR_WHITE     0x00    // Píxel blanco (0)
#define COLOR_BLACK     0xFF    // Píxel negro (1)

/*==============================================================================
 * PROTOTIPOS DE FUNCIONES
 *============================================================================*/

/**
 * @brief Limpia el buffer con un color específico
 * @param buffer Puntero al buffer
 * @param size Tamaño en bytes
 * @param color COLOR_WHITE o COLOR_BLACK
 */
void epd_clearBuffer(uint8_t* buffer, uint32_t size, uint8_t color);

/**
 * @brief Dibuja un píxel en el buffer
 * @param buffer Puntero al buffer
 * @param x Coordenada X (0-295)
 * @param y Coordenada Y (0-151)
 * @param color COLOR_WHITE o COLOR_BLACK
 */
void epd_drawPixel(uint8_t* buffer, uint16_t x, uint16_t y, uint8_t color);

/**
 * @brief Lee un píxel del buffer
 * @param buffer Puntero al buffer
 * @param x Coordenada X
 * @param y Coordenada Y
 * @return COLOR_WHITE o COLOR_BLACK
 */
uint8_t epd_getPixel(const uint8_t* buffer, uint16_t x, uint16_t y);

/**
 * @brief Dibuja una línea horizontal
 * @param buffer Puntero al buffer
 * @param x0 X inicial
 * @param x1 X final
 * @param y Y fija
 * @param color Color
 */
void epd_drawHLine(uint8_t* buffer, uint16_t x0, uint16_t x1, uint16_t y, uint8_t color);

/**
 * @brief Dibuja una línea vertical
 * @param buffer Puntero al buffer
 * @param x X fija
 * @param y0 Y inicial
 * @param y1 Y final
 * @param color Color
 */
void epd_drawVLine(uint8_t* buffer, uint16_t x, uint16_t y0, uint16_t y1, uint8_t color);

/**
 * @brief Dibuja un rectángulo
 * @param buffer Puntero al buffer
 * @param x Esquina X
 * @param y Esquina Y
 * @param w Ancho
 * @param h Alto
 * @param color Color
 * @param fill 1=relleno, 0=solo borde
 */
void epd_drawRect(uint8_t* buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color, uint8_t fill);

/**
 * @brief Dibuja un carácter en el buffer
 * @param buffer Puntero al buffer
 * @param x Posición X
 * @param y Posición Y
 * @param c Carácter (ASCII 32-127)
 * @param color Color del texto
 * @param bgColor Color de fondo (usar COLOR_WHITE para transparente)
 */
void epd_drawChar(uint8_t* buffer, uint16_t x, uint16_t y, char c, uint8_t color, uint8_t bgColor);

/**
 * @brief Dibuja un string en el buffer
 * @param buffer Puntero al buffer
 * @param x Posición X
 * @param y Posición Y
 * @param str String a dibujar
 * @param color Color del texto
 * @param bgColor Color de fondo
 */
void epd_drawString(uint8_t* buffer, uint16_t x, uint16_t y, const char* str, uint8_t color, uint8_t bgColor);

/**
 * @brief Carga una imagen desde un array de bytes
 * @param buffer Buffer destino
 * @param imageData Array de la imagen
 * @param size Tamaño de la imagen en bytes
 */
void epd_loadImage(uint8_t* buffer, const uint8_t* imageData, uint32_t size);

/**
 * @brief Invierte los colores del buffer
 * @param buffer Puntero al buffer
 * @param size Tamaño del buffer
 */
void epd_invertBuffer(uint8_t* buffer, uint32_t size);

#endif /* EPD_GRAPHICS_H */
