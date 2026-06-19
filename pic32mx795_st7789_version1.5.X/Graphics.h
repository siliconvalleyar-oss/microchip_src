#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "HardwareProfile.h"

namespace Graphics {
    // Primitivas base (envueltas desde st7789_driver)
    inline void fill_screen(uint16_t c) { ::fill_screen(c); }
    inline void draw_pixel(int16_t x, int16_t y, uint16_t c) { ::draw_pixel(x, y, c); }
    inline void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) { ::fill_rect(x, y, w, h, c); }
    inline void draw_hline(int16_t x, int16_t y, int16_t l, uint16_t c) { ::draw_hline(x, y, l, c); }
    inline void draw_vline(int16_t x, int16_t y, int16_t l, uint16_t c) { ::draw_vline(x, y, l, c); }
    inline void draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t c) { ::draw_circle(cx, cy, r, c); }
    inline void draw_char(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t s) { ::draw_char(x, y, c, fg, bg, s); }
    inline void draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale) { ::draw_string(x, y, s, fg, bg, scale); }

    // ---- Sprites de juego ----
    void draw_pacman(int16_t x, int16_t y, bool mouth_open, uint8_t dir);
    void draw_pacman_scaled(int16_t cx, int16_t cy, bool mouth_open, uint8_t dir, uint8_t scale);
    void draw_ghost(int16_t x, int16_t y, uint16_t color, bool frightened);
    void draw_ghost_scaled(int16_t cx, int16_t cy, uint16_t color, bool frightened, uint8_t scale);

    // ---- Laberinto ----
    void draw_dot(int16_t cellPx, int16_t cellPy);
    void draw_power(int16_t cellPx, int16_t cellPy);
    void draw_wall_cell(int16_t px, int16_t py, bool top, bool bot, bool lft, bool rgt);
    void draw_score_popup(int16_t x, int16_t y, uint16_t points);
}

#endif
