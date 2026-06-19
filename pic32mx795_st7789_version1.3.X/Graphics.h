/**
 * @file Graphics.h
 * @brief Primitivas gráficas y dibujo de naves (usa st7789_driver.c como base).
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "HardwareProfile.h"

// Primitivas vía st7789_driver (C-linkage, envueltas en namespace)
namespace Graphics {
    inline void fill_screen(uint16_t c) { ::fill_screen(c); }
    inline void draw_pixel(int16_t x, int16_t y, uint16_t c) { ::draw_pixel(x, y, c); }
    inline void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) { ::fill_rect(x, y, w, h, c); }
    inline void draw_hline(int16_t x, int16_t y, int16_t l, uint16_t c) { ::draw_hline(x, y, l, c); }
    inline void draw_vline(int16_t x, int16_t y, int16_t l, uint16_t c) { ::draw_vline(x, y, l, c); }
    inline void draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t c) { ::draw_circle(cx, cy, r, c); }
    inline void draw_char(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t s) { ::draw_char(x, y, c, fg, bg, s); }
    inline void draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale) { ::draw_string(x, y, s, fg, bg, scale); }

    // Dibujo de naves con paleta de colores
    void draw_ship_palette(int16_t x, int16_t y, uint16_t body, uint16_t cockpit,
                           uint16_t engine, uint16_t wing, uint16_t detail,
                           uint16_t bg, bool flip = false);
    void draw_ship(int16_t x, int16_t y, uint16_t bg_color);
    void erase_ship(int16_t x, int16_t y, uint16_t bg_color);
}

#endif // GRAPHICS_H
