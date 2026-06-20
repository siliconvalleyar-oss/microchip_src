#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "HardwareProfile.h"
#include "fonts.h"



    // En Graphics.h, después de los includes
extern uint8_t maze[MAZE_W][MAZE_H];

namespace Graphics {
    


    void fill_screen(uint16_t color);
    //void draw_pixel(int16_t x, int16_t y, uint16_t color);

    
#ifdef USE_DOUBLE_BUFFER
    uint16_t framebuffer[TFT_W * TFT_H];

    void flush_buffer(void) ;

    void draw_pixel(int16_t x, int16_t y, uint16_t color) ;
#else

void draw_pixel(int16_t x, int16_t y, uint16_t color) ;

#endif
    
    
    
    
    void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void draw_hline(int16_t x, int16_t y, int16_t len, uint16_t color);
    void draw_vline(int16_t x, int16_t y, int16_t len, uint16_t color);
    void draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
    void draw_char(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale);
    void draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale);
    
    // Dibujo de Pac-Man (con boca abierta/cerrada)
    void draw_pacman(int16_t x, int16_t y, bool mouth_open);
    //void erase_pacman(int16_t x, int16_t y);
    //void erase_pacman(int16_t x, int16_t y, uint8_t (*maze)[MAZE_H]);
        // Dibujo de fantasmas
    void draw_ghost(int16_t x, int16_t y, uint16_t color);
    //void erase_ghost(int16_t x, int16_t y);
   // void erase_ghost(int16_t x, int16_t y, uint8_t (*maze)[MAZE_H]);
    void erase_pacman(int16_t x, int16_t y, uint8_t (*maze)[MAZE_H]);
    void erase_ghost(int16_t x, int16_t y, uint8_t (*maze)[MAZE_H]);
    // Dibujo de punto
    void draw_dot(int16_t x, int16_t y);
    void erase_dot(int16_t x, int16_t y);
    
    void flush_buffer(void);
}

#endif

