#include "GameEngine.h"
#include "Sound.h"
#include <cstdio>
#include <stdlib.h>

namespace GameEngine {
    // Laberinto 10x10 (0=pasillo, 1=pared, 2=punto)
    uint8_t maze[MAZE_W][MAZE_H] = {
        {1,1,1,1,1,1,1,1,1,1},
        {1,2,2,2,1,2,2,2,2,1},
        {1,2,1,2,1,2,1,2,2,1},
        {1,2,1,2,2,2,1,2,2,1},
        {1,2,2,2,1,2,2,2,2,1},
        {1,2,1,2,1,2,1,2,2,1},
        {1,2,1,2,2,2,1,2,2,1},
        {1,2,2,2,1,2,2,2,2,1},
        {1,2,2,2,1,2,2,2,2,1},
        {1,1,1,1,1,1,1,1,1,1}
    };

    int16_t pacX = 1*CELL_W, pacY = 1*CELL_H;
    int16_t prevPacX = 1*CELL_W, prevPacY = 1*CELL_H;
    int8_t  pacDir = 3;
    int8_t  requestedDir = 3;
    bool    mouthOpen = true;
    uint8_t mouthCounter = 0;

    int16_t ghostX[4] = {8*CELL_W, 8*CELL_W, 7*CELL_W, 7*CELL_W};
    int16_t ghostY[4] = {8*CELL_H, 9*CELL_H, 8*CELL_H, 9*CELL_H};
    int16_t prevGhostX[4] = {8*CELL_W, 8*CELL_W, 7*CELL_W, 7*CELL_W};
    int16_t prevGhostY[4] = {8*CELL_H, 9*CELL_H, 8*CELL_H, 9*CELL_H};
    uint8_t ghostDir[4] = {2,2,1,1};
    uint16_t score = 0;
    uint8_t  lives = 3;
    bool     gameOver = false;

    // -----------------------------------------------------------------------
    // redraw_cells_in_rect
    //   Reconstruye todas las celdas del laberinto que se solapan con el
    //   rectángulo (rx, ry, rw, rh) en coordenadas de píxel.
    //   Esto garantiza que cualquier sprite ?sin importar cuántas celdas
    //   cruce? deja el fondo perfectamente limpio al borrarse.
    // -----------------------------------------------------------------------
    static void redraw_cells_in_rect(int16_t rx, int16_t ry,
                                     int16_t rw, int16_t rh)
    {
        // Celdas que cubre el rectángulo (con clip a los límites del mapa)
        int16_t cx0 = rx / CELL_W;
        int16_t cy0 = ry / CELL_H;
        int16_t cx1 = (rx + rw - 1) / CELL_W;
        int16_t cy1 = (ry + rh - 1) / CELL_H;

        if(cx0 < 0)      cx0 = 0;
        if(cy0 < 0)      cy0 = 0;
        if(cx1 >= MAZE_W) cx1 = MAZE_W - 1;
        if(cy1 >= MAZE_H) cy1 = MAZE_H - 1;

        for(int16_t cx = cx0; cx <= cx1; cx++) {
            for(int16_t cy = cy0; cy <= cy1; cy++) {
                int16_t px = cx * CELL_W;
                int16_t py = cy * CELL_H;
                if(maze[cx][cy] == 1) {
                    Graphics::fill_rect(px, py, CELL_W, CELL_H, COLOR565(0,0,80));
                    Graphics::draw_hline(px, py, CELL_W, COLOR565(0,0,150));
                    Graphics::draw_vline(px, py, CELL_H, COLOR565(0,0,150));
                } else if(maze[cx][cy] == 2) {
                    Graphics::fill_rect(px, py, CELL_W, CELL_H, BLACK);
                    Graphics::draw_dot(px, py);
                } else {
                    Graphics::fill_rect(px, py, CELL_W, CELL_H, BLACK);
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    void init_maze(void) {
        for(uint8_t i = 0; i < MAZE_W; i++)
            for(uint8_t j = 0; j < MAZE_H; j++)
                if(maze[i][j] == 0 &&
                   !(i==0 || i==MAZE_W-1 || j==0 || j==MAZE_H-1))
                    maze[i][j] = 2;

        score = 0;  lives = 3;  gameOver = false;
        pacX = 1*CELL_W;  pacY = 1*CELL_H;
        pacDir = 3;  requestedDir = 3;

        ghostX[0]=8*CELL_W; ghostY[0]=8*CELL_H;
        ghostX[1]=8*CELL_W; ghostY[1]=9*CELL_H;
        ghostX[2]=7*CELL_W; ghostY[2]=8*CELL_H;
        ghostX[3]=7*CELL_W; ghostY[3]=9*CELL_H;
        for(int i=0;i<4;i++){
            prevGhostX[i]=ghostX[i];
            prevGhostY[i]=ghostY[i];
        }
        prevPacX = pacX;  prevPacY = pacY;
    }

    // -----------------------------------------------------------------------
    void draw_maze(void) {
        Graphics::fill_screen(BLACK);
        for(uint8_t i = 0; i < MAZE_W; i++) {
            for(uint8_t j = 0; j < MAZE_H; j++) {
                int16_t x = i*CELL_W, y = j*CELL_H;
                if(maze[i][j] == 1) {
                    Graphics::fill_rect(x, y, CELL_W, CELL_H, COLOR565(0,0,80));
                    Graphics::draw_hline(x, y, CELL_W, COLOR565(0,0,150));
                    Graphics::draw_vline(x, y, CELL_H, COLOR565(0,0,150));
                } else if(maze[i][j] == 2) {
                    Graphics::fill_rect(x+CELL_W/2-2, y+CELL_H/2-2, 4, 4, WHITE);
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    // canMove: devuelve true si el centro del sprite puede avanzar 'dir'
    // -----------------------------------------------------------------------
    static bool canMove(int16_t x, int16_t y, int8_t dir) {
        int16_t nx = x, ny = y;
        switch(dir) {
            case 0: ny -= 4; break;
            case 1: ny += 4; break;
            case 2: nx -= 4; break;
            case 3: nx += 4; break;
        }
        uint8_t cx = (nx + PACMAN_SIZE/2) / CELL_W;
        uint8_t cy = (ny + PACMAN_SIZE/2) / CELL_H;
        if(cx < MAZE_W && cy < MAZE_H)
            return maze[cx][cy] != 1;
        return false;
    }

    // -----------------------------------------------------------------------
    void update_pacman(void) {
        static uint8_t lastCellX = 0xFF, lastCellY = 0xFF;

        prevPacX = pacX;
        prevPacY = pacY;

        // Animación boca
        if(++mouthCounter >= 6) { mouthOpen = !mouthOpen; mouthCounter = 0; }

        // Dirección solicitada
        if(canMove(pacX, pacY, requestedDir)) pacDir = requestedDir;

        // Mover
        int16_t nx = pacX, ny = pacY;
        switch(pacDir) {
            case 0: ny -= 4; break;
            case 1: ny += 4; break;
            case 2: nx -= 4; break;
            case 3: nx += 4; break;
        }

        uint8_t cx = (nx + PACMAN_SIZE/2) / CELL_W;
        uint8_t cy = (ny + PACMAN_SIZE/2) / CELL_H;

        if(cx < MAZE_W && cy < MAZE_H && maze[cx][cy] != 1) {
            pacX = nx;  pacY = ny;
            if(maze[cx][cy] == 2) {
                maze[cx][cy] = 0;
                score += 10;
                sound_eat();
            }
        }

        // Clip a zona de juego
        if(pacX < 0) pacX = 0;
        if(pacY < 0) pacY = 0;
        if(pacX + PACMAN_SIZE > TFT_W)       pacX = TFT_W - PACMAN_SIZE;
        if(pacY + PACMAN_SIZE > TFT_H - 30)  pacY = TFT_H - 30 - PACMAN_SIZE;

        // -------------------------------------------------------------------
        // BORRAR: reconstruir todas las celdas que cubría la posición anterior
        // -------------------------------------------------------------------
        redraw_cells_in_rect(prevPacX, prevPacY, PACMAN_SIZE, PACMAN_SIZE);

        // DIBUJAR en nueva posición
        Graphics::draw_pacman(pacX, pacY, mouthOpen);

        // HUD solo cuando cambia de celda
        uint8_t newCX = (pacX + PACMAN_SIZE/2) / CELL_W;
        uint8_t newCY = (pacY + PACMAN_SIZE/2) / CELL_H;
        if(newCX != lastCellX || newCY != lastCellY) {
            lastCellX = newCX;  lastCellY = newCY;
            draw_status();
        }
    }

    // -----------------------------------------------------------------------
    void update_ghosts(void) {
        static const uint16_t GHOST_COLORS[4] = {
            RED, 0xF81F /*rosa*/, 0x07FF /*cyan*/, 0xFFA0 /*naranja*/
        };

        for(int i = 0; i < 4; i++) {
            prevGhostX[i] = ghostX[i];
            prevGhostY[i] = ghostY[i];

            // IA: perseguir a Pac-Man
            int16_t dx = pacX - ghostX[i];
            int16_t dy = pacY - ghostY[i];
            if(abs(dx) > abs(dy))
                ghostDir[i] = (dx > 0) ? 3 : 2;
            else
                ghostDir[i] = (dy > 0) ? 1 : 0;

            int16_t nx = ghostX[i], ny = ghostY[i];
            switch(ghostDir[i]) {
                case 0: ny -= 2; break;
                case 1: ny += 2; break;
                case 2: nx -= 2; break;
                case 3: nx += 2; break;
            }

            uint8_t cx = (nx + 10) / CELL_W;
            uint8_t cy = (ny + 10) / CELL_H;

            if(cx < MAZE_W && cy < MAZE_H && maze[cx][cy] != 1) {
                ghostX[i] = nx;
                ghostY[i] = ny;
            }

            // ---------------------------------------------------------------
            // BORRAR: reconstruir celdas de la posición anterior del fantasma
            // El sprite de fantasma también es PACMAN_SIZE x PACMAN_SIZE (20x20)
            // ---------------------------------------------------------------
            redraw_cells_in_rect(prevGhostX[i], prevGhostY[i],
                                  PACMAN_SIZE, PACMAN_SIZE);

            // DIBUJAR en nueva posición
            Graphics::draw_ghost(ghostX[i], ghostY[i], GHOST_COLORS[i]);
        }
    }

    // -----------------------------------------------------------------------
    void check_collisions(void) {
        for(int i = 0; i < 4; i++) {
            if(abs(pacX - ghostX[i]) < 20 && abs(pacY - ghostY[i]) < 20) {
                if(lives > 0) {
                    lives--;
                    sound_death();
                    draw_maze();

                    pacX  = 1*CELL_W;  pacY  = 1*CELL_H;
                    prevPacX = pacX;   prevPacY = pacY;

                    ghostX[0]=8*CELL_W; ghostY[0]=8*CELL_H;
                    ghostX[1]=8*CELL_W; ghostY[1]=9*CELL_H;
                    ghostX[2]=7*CELL_W; ghostY[2]=8*CELL_H;
                    ghostX[3]=7*CELL_W; ghostY[3]=9*CELL_H;
                    for(int j=0;j<4;j++){
                        prevGhostX[j]=ghostX[j];
                        prevGhostY[j]=ghostY[j];
                    }

                    Graphics::draw_pacman(pacX, pacY, true);
                    draw_status();
                    delay_ms(1000);
                } else {
                    gameOver = true;
                }
                break;
            }
        }
    }

    // -----------------------------------------------------------------------
    void draw_status(void) {
        static uint16_t lastScore = 0xFFFF;
        static uint8_t  lastLives = 0xFF;

        if(score != lastScore || lives != lastLives) {
            Graphics::fill_rect(0, TFT_H-25, TFT_W, 25, BLACK);

            for(uint8_t i = 0; i < lives && i < 5; i++)
                Graphics::draw_pacman(10 + i*25, TFT_H-20, true);

            char buf[20];
            sprintf(buf, "SCORE: %u", score);
            Graphics::draw_string(TFT_W-120, TFT_H-20, buf, YELLOW, BLACK, 1);
            Graphics::draw_hline(0, TFT_H-26, TFT_W, COLOR565(50,50,50));

            lastScore = score;
            lastLives = lives;
        }
    }

    // -----------------------------------------------------------------------
    void game_loop(void) {
        uint32_t lastTime = _CP0_GET_COUNT();
        const uint32_t frameTime = 60000; // ~25 FPS a 80 MHz

        draw_maze();
        draw_status();

        while(!gameOver) {
            #ifdef USE_BUTTONS
            if(BTN_UP)         requestedDir = 0;
            else if(BTN_DOWN)  requestedDir = 1;
            else if(BTN_LEFT)  requestedDir = 2;
            else if(BTN_RIGHT) requestedDir = 3;
            #else
            // Movimiento automático para pruebas
            static uint8_t changeCounter = 0;
            if(++changeCounter > 30) {
                changeCounter = 0;
                requestedDir = rand() % 4;
            }
            #endif

            update_pacman();
            update_ghosts();
            check_collisions();

            while((_CP0_GET_COUNT() - lastTime) < frameTime);
            lastTime = _CP0_GET_COUNT();
        }

        Graphics::fill_screen(BLACK);
        Graphics::draw_string(TFT_W/2-70, TFT_H/2-20, "GAME OVER", RED,   BLACK, 2);
        Graphics::draw_string(TFT_W/2-80, TFT_H/2+10, "PRESS RESET", WHITE, BLACK, 1);
        while(1);
    }

} // namespace GameEngine