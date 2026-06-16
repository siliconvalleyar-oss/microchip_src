#include "Graphics.h"
#include "HardwareProfile.h"

void Graphics::fill_screen(uint16_t color) {
    set_window(0, 0, TFT_W-1, TFT_H-1);
    uint32_t n = (uint32_t)TFT_W * TFT_H;
    uint8_t hi = color>>8, lo = color&0xFF;
    for(uint32_t i=0; i<n; i++) { spi4_write(hi); spi4_write(lo); }
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

// ========== PAC-MAN ==========
void Graphics::draw_pacman(int16_t x, int16_t y, bool mouth_open) {
    // Cuerpo amarillo (cuadrado redondeado)
    fill_rect(x+2, y, PACMAN_SIZE-4, PACMAN_SIZE, 0xFFE0);
    fill_rect(x, y+2, PACMAN_SIZE, PACMAN_SIZE-4, 0xFFE0);
    
    // Boca
    if(mouth_open) {
        for(int i = 0; i < PACMAN_SIZE/2; i++) {
            draw_pixel(x + PACMAN_SIZE/2 + i, y + i, BLACK);
            draw_pixel(x + PACMAN_SIZE/2 + i, y + PACMAN_SIZE - i, BLACK);
        }
    }
    
    // Ojo
    fill_rect(x + PACMAN_SIZE - 7, y + PACMAN_SIZE/3, 3, 3, WHITE);
    draw_pixel(x + PACMAN_SIZE - 6, y + PACMAN_SIZE/3 + 1, BLACK);
}


void Graphics::erase_pacman(int16_t x, int16_t y, uint8_t (*maze)[MAZE_H]) {
    // Obtener la celda donde está Pac-Man
    uint8_t cellX = (x + PACMAN_SIZE/2) / CELL_W;
    uint8_t cellY = (y + PACMAN_SIZE/2) / CELL_H;
    
    if(cellX >= MAZE_W || cellY >= MAZE_H) return;
    
    // Redibujar toda la celda completa según el laberinto
    int16_t cellXpos = cellX * CELL_W;
    int16_t cellYpos = cellY * CELL_H;
    
    if(maze[cellX][cellY] == 1) {
        // Pared azul
        fill_rect(cellXpos, cellYpos, CELL_W, CELL_H, COLOR565(0,0,80));
        draw_hline(cellXpos, cellYpos, CELL_W, COLOR565(0,0,150));
        draw_vline(cellXpos, cellYpos, CELL_H, COLOR565(0,0,150));
    } else if(maze[cellX][cellY] == 2) {
        // Punto blanco
        fill_rect(cellXpos, cellYpos, CELL_W, CELL_H, BLACK);
        draw_dot(cellXpos, cellYpos);
    } else {
        // Pasillo vacío (negro)
        fill_rect(cellXpos, cellYpos, CELL_W, CELL_H, BLACK);
    }
}

void Graphics::erase_ghost(int16_t x, int16_t y, uint8_t (*maze)[MAZE_H]) {
    // Obtener la celda donde está el fantasma
    uint8_t cellX = (x + 10) / CELL_W;
    uint8_t cellY = (y + 10) / CELL_H;
    
    if(cellX >= MAZE_W || cellY >= MAZE_H) return;
    
    // Redibujar toda la celda completa según el laberinto
    int16_t cellXpos = cellX * CELL_W;
    int16_t cellYpos = cellY * CELL_H;
    
    if(maze[cellX][cellY] == 1) {
        fill_rect(cellXpos, cellYpos, CELL_W, CELL_H, COLOR565(0,0,80));
        draw_hline(cellXpos, cellYpos, CELL_W, COLOR565(0,0,150));
        draw_vline(cellXpos, cellYpos, CELL_H, COLOR565(0,0,150));
    } else if(maze[cellX][cellY] == 2) {
        fill_rect(cellXpos, cellYpos, CELL_W, CELL_H, BLACK);
        draw_dot(cellXpos, cellYpos);
    } else {
        fill_rect(cellXpos, cellYpos, CELL_W, CELL_H, BLACK);
    }
}


// ========== FANTASMAS ==========
void Graphics::draw_ghost(int16_t x, int16_t y, uint16_t color) {
    // Cuerpo
    draw_circle(x + 10, y + 8, 8, color);
    fill_rect(x + 2, y + 8, 16, 8, color);
    
    // Volantes
    fill_rect(x + 2, y + 16, 4, 4, color);
    fill_rect(x + 8, y + 16, 4, 4, color);
    fill_rect(x + 14, y + 16, 4, 4, color);
    
    // Ojos blancos
    fill_rect(x + 5, y + 5, 4, 4, WHITE);
    fill_rect(x + 11, y + 5, 4, 4, WHITE);
    
    // Pupilas azules
    draw_pixel(x + 6, y + 6, BLUE);
    draw_pixel(x + 7, y + 6, BLUE);
    draw_pixel(x + 12, y + 6, BLUE);
    draw_pixel(x + 13, y + 6, BLUE);
}





void Graphics::draw_dot(int16_t x, int16_t y) {
    fill_rect(x + CELL_W/2 - 2, y + CELL_H/2 - 2, 4, 4, WHITE);
}

void Graphics::erase_dot(int16_t x, int16_t y) {
    fill_rect(x + CELL_W/2 - 2, y + CELL_H/2 - 2, 4, 4, BLACK);
}

// ========== PIXEL ==========
void Graphics::draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if((uint16_t)x >= TFT_W || (uint16_t)y >= TFT_H) return;
    set_window(x, y, x, y);
    push_color(color);
}