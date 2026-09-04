#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "HardwareProfile.h"
#include "Graphics.h"

namespace GameEngine {
    extern uint8_t maze[MAZE_W][MAZE_H];   // 0=pasillo, 1=pared, 2=punto
    extern int16_t pacX, pacY;             // coordenadas en píxeles
    extern int8_t pacDir;                  // 0=arriba,1=abajo,2=izq,3=der
    extern int16_t ghostX[4], ghostY[4];
    extern uint8_t ghostDir[4];
    extern uint16_t score;
    extern uint8_t lives;
    extern bool gameOver;
    
    void init_maze(void);
    void draw_maze(void);
    void update_pacman(void);
    void update_ghosts(void);
    void check_collisions(void);
    void draw_status(void);
    void game_loop(void);
}

#endif

