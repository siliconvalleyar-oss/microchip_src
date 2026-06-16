/**
 * @file Graphics.cpp
 * @brief Implementación de gráficos.
 */

#include "Graphics.h"
#include "HardwareProfile.h"

// Sprite de la nave (16x24) con valores simbólicos (se reemplazan por paleta)
// 0=transparente, 1=cuerpo, 2=cabina, 3=motor, 4=ala, 5=detalle, 6=llama
static const uint8_t ship_sprite_symbolic[24][16] = {
    {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0},
    {0,0,0,0,0,1,2,2,2,2,1,0,0,0,0,0},
    {0,0,0,0,1,2,2,2,2,2,2,1,0,0,0,0},
    {0,0,0,1,2,2,5,2,2,5,2,2,1,0,0,0},
    {0,0,4,1,1,1,1,1,1,1,1,1,1,4,0,0},
    {0,4,4,1,2,2,2,2,2,2,2,2,1,4,4,0},
    {4,4,4,1,2,2,2,2,2,2,2,2,1,4,4,4},
    {4,4,4,1,2,5,2,2,2,2,5,2,1,4,4,4},
    {4,4,1,1,1,1,1,1,1,1,1,1,1,1,4,4},
    {0,4,1,1,5,1,3,3,3,3,1,5,1,1,4,0},
    {0,4,1,1,1,1,3,3,3,3,1,1,1,1,4,0},
    {0,0,4,1,1,1,1,1,1,1,1,1,1,4,0,0},
    {0,0,4,1,3,3,1,1,1,1,3,3,1,4,0,0},
    {0,0,4,1,3,3,1,1,1,1,3,3,1,4,0,0},
    {0,0,0,1,3,3,1,3,3,1,3,3,1,0,0,0},
    {0,0,0,1,3,3,1,3,3,1,3,3,1,0,0,0},
    {0,0,0,0,6,6,0,6,6,0,6,6,0,0,0,0},
    {0,0,0,0,6,3,0,6,3,0,6,3,0,0,0,0},
    {0,0,0,0,3,6,0,3,6,0,3,6,0,0,0,0},
    {0,0,0,0,0,6,0,0,6,0,0,6,0,0,0,0},
    {0,0,0,0,0,3,0,0,3,0,0,3,0,0,0,0},
    {0,0,0,0,0,6,0,0,6,0,0,6,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

void Graphics::fill_screen(uint16_t color) {
    set_window(0, 0, TFT_W-1, TFT_H-1);
    uint32_t n = (uint32_t)TFT_W * TFT_H;
    uint8_t hi = color>>8, lo = color&0xFF;
    for(uint32_t i=0; i<n; i++) { spi4_write(hi); spi4_write(lo); }
}

void Graphics::draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if((uint16_t)x >= TFT_W || (uint16_t)y >= TFT_H) return;
    set_window(x, y, x, y);
    push_color(color);
}

void Graphics::fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if(x>=TFT_W || y>=TFT_H || w<=0 || h<=0) return;
    if(x<0){ w+=x; x=0; }
    if(y<0){ h+=y; y=0; }
    if(x+w>TFT_W) w = TFT_W - x;
    if(y+h>TFT_H) h = TFT_H - y;
    set_window(x, y, x+w-1, y+h-1);
    uint32_t n = (uint32_t)w * h;
    uint8_t hi = color>>8, lo = color&0xFF;
    for(uint32_t i=0; i<n; i++) { spi4_write(hi); spi4_write(lo); }
}

void Graphics::draw_hline(int16_t x, int16_t y, int16_t len, uint16_t color) { fill_rect(x, y, len, 1, color); }
void Graphics::draw_vline(int16_t x, int16_t y, int16_t len, uint16_t color) { fill_rect(x, y, 1, len, color); }

void Graphics::draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    int16_t x = 0, y = r, d = 3 - 2*r;
    while(x <= y) {
        draw_pixel(cx+x, cy+y, color); draw_pixel(cx-x, cy+y, color);
        draw_pixel(cx+x, cy-y, color); draw_pixel(cx-x, cy-y, color);
        draw_pixel(cx+y, cy+x, color); draw_pixel(cx-y, cy+x, color);
        draw_pixel(cx+y, cy-x, color); draw_pixel(cx-y, cy-x, color);
        if(d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
}

void Graphics::draw_char(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale) {
    if(c < 0x20 || c > 0x7E) c = ' ';
    uint8_t idx = c - 0x20;
    for(uint8_t col=0; col<5; col++) {
        uint8_t line = font5x7[idx][col];
        for(uint8_t row=0; row<7; row++) {
            uint16_t color = (line & (1<<row)) ? fg : bg;
            fill_rect(x + col*scale, y + row*scale, scale, scale, color);
        }
    }
}

void Graphics::draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale) {
    while(*s) {
        draw_char(x, y, *s, fg, bg, scale);
        x += 6*scale;
        s++;
    }
}

void Graphics::draw_ship_palette(int16_t x, int16_t y, uint16_t body, uint16_t cockpit, uint16_t engine, uint16_t wing, uint16_t detail, uint16_t bg, bool flip) {
    for(int16_t row=0; row<SHIP_H; row++) {
        int16_t srcRow = flip ? (SHIP_H-1-row) : row;
        for(int16_t col=0; col<SHIP_W; col++) {
            uint8_t symbol = ship_sprite_symbolic[srcRow][col];
            uint16_t color = bg;
            switch(symbol) {
                case 1: color = body; break;
                case 2: color = cockpit; break;
                case 3: color = engine; break;
                case 4: color = wing; break;
                case 5: color = detail; break;
                case 6: color = engine; break; // llama
                default: color = bg; break;
            }
            draw_pixel(x+col, y+row, color);
        }
    }
}

/*
void Graphics::draw_ship_palette(int16_t x, int16_t y, uint16_t body, uint16_t cockpit, uint16_t engine, uint16_t wing, uint16_t detail, uint16_t bg, bool flip) {
    for(int16_t row=0; row<SHIP_H; row++) {
        int16_t srcRow = flip ? (SHIP_H-1-row) : row;
        for(int16_t col=0; col<SHIP_W; col++) {
            uint16_t px = ship_sprite[srcRow][col];
            uint16_t color = bg;
            if(px == COLOR565(100,220,255)) color = body;
            else if(px == COLOR565(200,240,255)) color = cockpit;
            else if(px == COLOR565(255,120,0)) color = engine;
            else if(px == COLOR565(0,80,200)) color = wing;
            else if(px == COLOR565(160,200,220)) color = detail;
            else if(px == COLOR565(255,240,0) || px == COLOR565(255,160,0)) color = engine;
            else if(px == 0x0000) color = bg;
            else color = px;
            draw_pixel(x+col, y+row, color);
        }
    }
}*/

void Graphics::draw_ship(int16_t x, int16_t y, uint16_t bg_color) {
    draw_ship_palette(x, y, COLOR565(100,220,255), COLOR565(200,240,255), COLOR565(255,120,0), COLOR565(0,80,200), COLOR565(160,200,220), bg_color, false);
}

void Graphics::erase_ship(int16_t x, int16_t y, uint16_t bg_color) {
    fill_rect(x, y, SHIP_W, SHIP_H, bg_color);
}
