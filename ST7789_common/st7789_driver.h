/**
 * @file st7789_driver.h
 * @brief Driver base reutilizable para display ST7789 240x240
 *        PIC32MX795F512H | SPI4 - Modo 3 | 80MHz
 *
 * Compatible con los 6 proyectos ST7789 (v1.0 - v1.5).
 * Incluye: inicialización SPI/display, primitivas gráficas,
 *          manejo de fuente 5x7 y retardos.
 */

#ifndef ST7789_DRIVER_H
#define ST7789_DRIVER_H

#include <xc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==================== CONFIGURACIÓN DEL SISTEMA ====================
// (Los #pragma config están en st7789_driver.c para evitar duplicados)
#define SYS_FREQ 80000000UL

// ==================== PINES DE PANTALLA (RB0-RB2) ====================
#define PIN_BL      0   // RB0 - Backlight
#define PIN_DC      1   // RB1 - Data/Command
#define PIN_RST     2   // RB2 - Reset
#define BL_LAT      LATBbits.LATB0
#define DC_LAT      LATBbits.LATB1
#define RST_LAT     LATBbits.LATB2
#define BL_TRIS     TRISBbits.TRISB0
#define DC_TRIS     TRISBbits.TRISB1
#define RST_TRIS    TRISBbits.TRISB2

// ==================== PARÁMETROS DE PANTALLA ====================
#define TFT_W         240
#define TFT_H         240

// ==================== COLORES RGB565 ====================
#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define ORANGE      0xFC00
#define PINK        0xF81F
#define LIME        0xAFE0
#define GRAY        0x8410
#define DARK_BLUE   0x0010
#define DARK_GREEN  0x0400

#define COLOR565(r,g,b) ((uint16_t)((((r)&0xF8)<<8) | (((g)&0xFC)<<3) | ((b)>>3)))

// ==================== DELAY ====================
static inline void delay_cycles(uint32_t c) {
    uint32_t s = _CP0_GET_COUNT();
    while((_CP0_GET_COUNT() - s) < c);
}
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

// ==================== SPI4 LOW-LEVEL ====================
void spi4_init(void);
void spi4_write(uint8_t d);
void write_cmd(uint8_t c);
void write_data(uint8_t d);
void push_color(uint16_t color);

// ==================== DISPLAY INIT ====================
void reset_display(void);
void init_display(void);
void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// ==================== PRIMITIVAS GRÁFICAS ====================
void fill_screen(uint16_t color);
void draw_pixel(int16_t x, int16_t y, uint16_t color);
void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void draw_hline(int16_t x, int16_t y, int16_t len, uint16_t color);
void draw_vline(int16_t x, int16_t y, int16_t len, uint16_t color);
void draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color);

// ==================== TEXTO (Fuente 5x7) ====================
void draw_char(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale);
void draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale);

// ==================== UTILIDADES ====================
void draw_rect_outline(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
uint32_t lcg_rand(void);
void lcg_srand(uint32_t seed);

#ifdef __cplusplus
}
#endif

#endif // ST7789_DRIVER_H
