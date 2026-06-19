#include "Graphics.h"
#include "HardwareProfile.h"

// ========== PAC-MAN ==========
void Graphics::draw_pacman(int16_t x, int16_t y, bool mouth_open) {
    fill_rect(x+2, y, PACMAN_SIZE-4, PACMAN_SIZE, 0xFFE0);
    fill_rect(x, y+2, PACMAN_SIZE, PACMAN_SIZE-4, 0xFFE0);

    if(mouth_open) {
        for(int i = 0; i < PACMAN_SIZE/2; i++) {
            draw_pixel(x + PACMAN_SIZE/2 + i, y + i, BLACK);
            draw_pixel(x + PACMAN_SIZE/2 + i, y + PACMAN_SIZE - i, BLACK);
        }
    }
    fill_rect(x + PACMAN_SIZE - 7, y + PACMAN_SIZE/3, 3, 3, WHITE);
    draw_pixel(x + PACMAN_SIZE - 6, y + PACMAN_SIZE/3 + 1, BLACK);
}

void Graphics::erase_pacman(int16_t x, int16_t y, uint8_t (*maze)[MAZE_H]) {
    uint8_t cellX = (x + PACMAN_SIZE/2) / CELL_W;
    uint8_t cellY = (y + PACMAN_SIZE/2) / CELL_H;
    if(cellX >= MAZE_W || cellY >= MAZE_H) return;
    int16_t px = cellX * CELL_W, py = cellY * CELL_H;
    if(maze[cellX][cellY] == 1) {
        fill_rect(px, py, CELL_W, CELL_H, COLOR565(0,0,80));
        draw_hline(px, py, CELL_W, COLOR565(0,0,150));
        draw_vline(px, py, CELL_H, COLOR565(0,0,150));
    } else if(maze[cellX][cellY] == 2) {
        fill_rect(px, py, CELL_W, CELL_H, BLACK);
        draw_dot(px, py);
    } else {
        fill_rect(px, py, CELL_W, CELL_H, BLACK);
    }
}

// ========== FANTASMAS ==========
void Graphics::draw_ghost(int16_t x, int16_t y, uint16_t color) {
    draw_circle(x + 10, y + 8, 8, color);
    fill_rect(x + 2, y + 8, 16, 8, color);
    fill_rect(x + 2, y + 16, 4, 4, color);
    fill_rect(x + 8, y + 16, 4, 4, color);
    fill_rect(x + 14, y + 16, 4, 4, color);
    fill_rect(x + 5, y + 5, 4, 4, WHITE);
    fill_rect(x + 11, y + 5, 4, 4, WHITE);
    draw_pixel(x + 6, y + 6, BLUE);
    draw_pixel(x + 7, y + 6, BLUE);
    draw_pixel(x + 12, y + 6, BLUE);
    draw_pixel(x + 13, y + 6, BLUE);
}

void Graphics::erase_ghost(int16_t x, int16_t y, uint8_t (*maze)[MAZE_H]) {
    uint8_t cellX = (x + 10) / CELL_W;
    uint8_t cellY = (y + 10) / CELL_H;
    if(cellX >= MAZE_W || cellY >= MAZE_H) return;
    int16_t px = cellX * CELL_W, py = cellY * CELL_H;
    if(maze[cellX][cellY] == 1) {
        fill_rect(px, py, CELL_W, CELL_H, COLOR565(0,0,80));
        draw_hline(px, py, CELL_W, COLOR565(0,0,150));
        draw_vline(px, py, CELL_H, COLOR565(0,0,150));
    } else if(maze[cellX][cellY] == 2) {
        fill_rect(px, py, CELL_W, CELL_H, BLACK);
        draw_dot(px, py);
    } else {
        fill_rect(px, py, CELL_W, CELL_H, BLACK);
    }
}

// ========== PUNTOS ==========
void Graphics::draw_dot(int16_t x, int16_t y) {
    fill_rect(x + CELL_W/2 - 2, y + CELL_H/2 - 2, 4, 4, WHITE);
}

void Graphics::erase_dot(int16_t x, int16_t y) {
    fill_rect(x + CELL_W/2 - 2, y + CELL_H/2 - 2, 4, 4, BLACK);
}
