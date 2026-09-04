// ============================================================
//  GameEngine.cpp ? Motor de Pac-Man para PIC32MX795 + ST7789
//  Versión sin dependencias problemáticas de C++
// ============================================================

#include "GameEngine.h"
#include "Sound.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace GameEngine {

// ============================================================
//  LABERINTO 9x9
//  0=pasillo  1=pared  2=punto  3=power pellet
// ============================================================
uint8_t maze[MAZE_W][MAZE_H] = {
    {1,1,1,1,1,1,1,1,1},
    {1,3,2,2,1,2,2,3,1},
    {1,2,1,2,2,2,1,2,1},
    {1,2,1,2,1,2,1,2,1},
    {1,2,2,2,1,2,2,2,1},
    {1,2,1,2,1,2,1,2,1},
    {1,2,1,2,2,2,1,2,1},
    {1,3,2,2,1,2,2,3,1},
    {1,1,1,1,1,1,1,1,1}
};

static uint8_t mazeTemplate[MAZE_W][MAZE_H];

// ---- Pac-Man ----
int16_t pacX, pacY;
static int16_t prevPacX, prevPacY;
int8_t  pacDir       = 3;
int8_t  requestedDir = 3;
static bool    mouthOpen    = true;
static uint8_t mouthCounter = 0;

// ---- Fantasmas ----
int16_t ghostX[4], ghostY[4];
static int16_t prevGhostX[4], prevGhostY[4];
uint8_t ghostDir[4];
bool    ghostFrightened[4];
uint8_t frightenTimer = 0;

static const uint16_t GHOST_COLORS[4] = { RED, PINK, CYAN, ORANGE };

// ---- Estado ----
uint16_t score     = 0;
uint16_t highScore = 0;
uint8_t  lives     = 3;
uint8_t  level     = 1;
bool     gameOver  = false;

// ---- Velocidad ----
static uint8_t pacSpeed   = 4;
static uint8_t ghostSpeed = 2;

// ============================================================
//  COORDENADAS
// ============================================================
static inline int16_t cellPX(uint8_t cx) { return MAZE_OX + (int16_t)cx * CELL_W; }
static inline int16_t cellPY(uint8_t cy) { return MAZE_OY + (int16_t)cy * CELL_H; }

static inline uint8_t pixToCellX(int16_t px) {
    int16_t v = (px - MAZE_OX) / CELL_W;
    if(v < 0) v = 0;
    if(v >= MAZE_W) v = MAZE_W-1;
    return (uint8_t)v;
}
static inline uint8_t pixToCellY(int16_t py) {
    int16_t v = (py - MAZE_OY) / CELL_H;
    if(v < 0) v = 0;
    if(v >= MAZE_H) v = MAZE_H-1;
    return (uint8_t)v;
}

// ============================================================
//  REDIBUJA todas las celdas solapadas con (rx,ry,rw,rh)
// ============================================================
static void redraw_cells_in_rect(int16_t rx, int16_t ry,
                                  int16_t rw, int16_t rh)
{
    int16_t cx0 = (rx - MAZE_OX) / CELL_W;
    int16_t cy0 = (ry - MAZE_OY) / CELL_H;
    int16_t cx1 = (rx + rw - 1 - MAZE_OX) / CELL_W;
    int16_t cy1 = (ry + rh - 1 - MAZE_OY) / CELL_H;

    if(cx0 < 0)        cx0 = 0;
    if(cy0 < 0)        cy0 = 0;
    if(cx1 >= MAZE_W)  cx1 = MAZE_W-1;
    if(cy1 >= MAZE_H)  cy1 = MAZE_H-1;

    for(int16_t cx = cx0; cx <= cx1; cx++) {
        for(int16_t cy = cy0; cy <= cy1; cy++) {
            int16_t px = cellPX((uint8_t)cx);
            int16_t py = cellPY((uint8_t)cy);
            uint8_t v  = maze[cx][cy];
            if(v == 1) {
                bool wT = (cy > 0)        && (maze[cx][cy-1]==1);
                bool wB = (cy < MAZE_H-1) && (maze[cx][cy+1]==1);
                bool wL = (cx > 0)        && (maze[cx-1][cy]==1);
                bool wR = (cx < MAZE_W-1) && (maze[cx+1][cy]==1);
                Graphics::draw_wall_cell(px, py, wT, wB, wL, wR);
            } else {
                Graphics::fill_rect(px, py, CELL_W, CELL_H, FLOOR_COLOR);
                if(v == 2) Graphics::draw_dot  (px, py);
                if(v == 3) Graphics::draw_power(px, py);
            }
        }
    }
}

// ============================================================
//  INIT
// ============================================================
static void save_template(void) {
    for(uint8_t i=0;i<MAZE_W;i++)
        for(uint8_t j=0;j<MAZE_H;j++)
            mazeTemplate[i][j] = maze[i][j];
}

void init_game(void) {
    // Restaurar laberinto desde plantilla
    for(uint8_t i=0;i<MAZE_W;i++)
        for(uint8_t j=0;j<MAZE_H;j++)
            maze[i][j] = mazeTemplate[i][j];

    pacX = cellPX(1); pacY = cellPY(1);
    prevPacX = pacX;  prevPacY = pacY;
    pacDir = 3; requestedDir = 3;
    mouthOpen = true; mouthCounter = 0;

    ghostX[0]=cellPX(7); ghostY[0]=cellPY(7);
    ghostX[1]=cellPX(7); ghostY[1]=cellPY(1);
    ghostX[2]=cellPX(1); ghostY[2]=cellPY(7);
    ghostX[3]=cellPX(4); ghostY[3]=cellPY(4);
    for(int i=0;i<4;i++){
        prevGhostX[i]=ghostX[i]; prevGhostY[i]=ghostY[i];
        ghostDir[i]=(i<2)?2:3;
        ghostFrightened[i]=false;
    }
    frightenTimer = 0;
    gameOver = false;

    pacSpeed   = (level<=3)?4:(level<=6)?5:6;
    ghostSpeed = (level<=2)?2:(level<=5)?3:4;
}

// ============================================================
//  DRAW MAZE
// ============================================================
void draw_maze(void) {
    Graphics::fill_screen(BLACK);
    for(uint8_t cx=0;cx<MAZE_W;cx++) {
        for(uint8_t cy=0;cy<MAZE_H;cy++) {
            int16_t px = cellPX(cx);
            int16_t py = cellPY(cy);
            uint8_t v  = maze[cx][cy];
            if(v==1) {
                bool wT=(cy>0)       &&(maze[cx][cy-1]==1);
                bool wB=(cy<MAZE_H-1)&&(maze[cx][cy+1]==1);
                bool wL=(cx>0)       &&(maze[cx-1][cy]==1);
                bool wR=(cx<MAZE_W-1)&&(maze[cx+1][cy]==1);
                Graphics::draw_wall_cell(px,py,wT,wB,wL,wR);
            } else if(v==2) {
                Graphics::draw_dot  (px,py);
            } else if(v==3) {
                Graphics::draw_power(px,py);
            }
        }
    }
}

// ============================================================
//  COLISION CON PAREDES (verifica las 4 esquinas del sprite)
// ============================================================
static bool canMovePac(int16_t px, int16_t py, int8_t dir) {
    int16_t nx=px, ny=py;
    switch(dir){
        case 0: ny-=pacSpeed; break;
        case 1: ny+=pacSpeed; break;
        case 2: nx-=pacSpeed; break;
        case 3: nx+=pacSpeed; break;
    }
    const int16_t M=2, S=PACMAN_SIZE-2*M-1;
    for(uint8_t c=0;c<4;c++){
        int16_t tx = nx + M + ((c&1)?S:0);
        int16_t ty = ny + M + ((c>>1)?S:0);
        uint8_t cx2 = pixToCellX(tx);
        uint8_t cy2 = pixToCellY(ty);
        if(maze[cx2][cy2]==1) return false;
    }
    return true;
}

// ============================================================
//  UPDATE PAC-MAN
// ============================================================
void update_pacman(void) {
    static uint8_t lastCX=0xFF, lastCY=0xFF;

    prevPacX = pacX;
    prevPacY = pacY;

    if(++mouthCounter >= 5){ mouthOpen=!mouthOpen; mouthCounter=0; }

    if(canMovePac(pacX,pacY,requestedDir)) pacDir=requestedDir;
    if(canMovePac(pacX,pacY,pacDir)){
        switch(pacDir){
            case 0: pacY-=pacSpeed; break;
            case 1: pacY+=pacSpeed; break;
            case 2: pacX-=pacSpeed; break;
            case 3: pacX+=pacSpeed; break;
        }
    }

    // Comer puntos
    uint8_t cx=pixToCellX(pacX+PACMAN_SIZE/2);
    uint8_t cy=pixToCellY(pacY+PACMAN_SIZE/2);
    if(maze[cx][cy]==2){
        maze[cx][cy]=0; score+=10; sound_eat();
    } else if(maze[cx][cy]==3){
        maze[cx][cy]=0; score+=50;
        frightenTimer=150;
        for(int i=0;i<4;i++) ghostFrightened[i]=true;
        sound_power_pellet();
    }

    // Anti-flicker: borrar posicion anterior
    redraw_cells_in_rect(prevPacX, prevPacY, PACMAN_SIZE, PACMAN_SIZE);

    // Dibujar Pac-Man
    Graphics::draw_pacman(pacX, pacY, mouthOpen, pacDir);

    uint8_t ncx=pixToCellX(pacX+PACMAN_SIZE/2);
    uint8_t ncy=pixToCellY(pacY+PACMAN_SIZE/2);
    if(ncx!=lastCX||ncy!=lastCY){ lastCX=ncx; lastCY=ncy; draw_status(); }
}

// ============================================================
//  UPDATE FANTASMAS
// ============================================================
void update_ghosts(void) {
    if(frightenTimer>0){
        frightenTimer--;
        if(frightenTimer==0)
            for(int i=0;i<4;i++) ghostFrightened[i]=false;
    }

    for(int i=0;i<4;i++){
        prevGhostX[i]=ghostX[i];
        prevGhostY[i]=ghostY[i];

        int16_t dx=pacX-ghostX[i];
        int16_t dy=pacY-ghostY[i];
        if(dx<0) dx = -dx;
        if(dy<0) dy = -dy;

        // Elegir direccion
        static uint8_t rnd[4]={0,8,16,24};
        rnd[i]++;
        bool randomTurn = (rnd[i] > 20+i*4);
        if(randomTurn) rnd[i]=0;

        if(ghostFrightened[i]){
            if(dx>dy) ghostDir[i]=(pacX>ghostX[i])?2:3;
            else      ghostDir[i]=(pacY>ghostY[i])?0:1;
        } else if(randomTurn){
            if((rand()%10)<3) ghostDir[i]=rand()%4;
            else { if(dx>dy) ghostDir[i]=(pacX>ghostX[i])?3:2;
                   else      ghostDir[i]=(pacY>ghostY[i])?1:0; }
        }

        int16_t spd=ghostFrightened[i]?(ghostSpeed>1?ghostSpeed-1:1):ghostSpeed;

        for(int tries=0;tries<4;tries++){
            int16_t nx=ghostX[i], ny=ghostY[i];
            switch(ghostDir[i]){
                case 0: ny-=spd; break;
                case 1: ny+=spd; break;
                case 2: nx-=spd; break;
                case 3: nx+=spd; break;
            }
            uint8_t cx2=pixToCellX(nx+GHOST_SIZE/2);
            uint8_t cy2=pixToCellY(ny+GHOST_SIZE/2);
            if(maze[cx2][cy2]!=1){
                ghostX[i]=nx; ghostY[i]=ny; break;
            }
            ghostDir[i]=(ghostDir[i]+1)%4;
        }

        // Anti-flicker
        redraw_cells_in_rect(prevGhostX[i], prevGhostY[i],
                              GHOST_SIZE, GHOST_SIZE);

        Graphics::draw_ghost(ghostX[i], ghostY[i],
                              GHOST_COLORS[i], ghostFrightened[i]);
    }
}

// ============================================================
//  COLISIONES PAC-MAN / FANTASMA
// ============================================================
void check_collisions(void) {
    for(int i=0;i<4;i++){
        int16_t dx = pacX-ghostX[i];
        int16_t dy = pacY-ghostY[i];
        if(dx<0) dx = -dx;
        if(dy<0) dy = -dy;
        if(dx<PACMAN_SIZE-4 && dy<PACMAN_SIZE-4){
            if(ghostFrightened[i]){
                ghostFrightened[i]=false;
                score+=200;
                sound_ghost();
                ghostX[i]=cellPX(4); ghostY[i]=cellPY(4);
                prevGhostX[i]=ghostX[i]; prevGhostY[i]=ghostY[i];
            } else {
                if(lives>0) lives--;
                sound_death();
                draw_maze();
                pacX=cellPX(1); pacY=cellPY(1);
                prevPacX=pacX; prevPacY=pacY;
                ghostX[0]=cellPX(7); ghostY[0]=cellPY(7);
                ghostX[1]=cellPX(7); ghostY[1]=cellPY(1);
                ghostX[2]=cellPX(1); ghostY[2]=cellPY(7);
                ghostX[3]=cellPX(4); ghostY[3]=cellPY(4);
                for(int j=0;j<4;j++){
                    prevGhostX[j]=ghostX[j]; prevGhostY[j]=ghostY[j];
                    ghostFrightened[j]=false;
                }
                frightenTimer=0;
                Graphics::draw_pacman(pacX,pacY,true,pacDir);
                draw_status();
                delay_ms(1200);
                if(lives==0){ gameOver=true; }
                return;
            }
        }
    }
}

// ============================================================
//  HUD
// ============================================================
void draw_status(void) {
    static uint16_t ls=0xFFFF;
    static uint8_t  ll=0xFF, llv=0xFF;
    if(score==ls&&lives==ll&&level==llv) return;
    ls=score; ll=lives; llv=level;

    if(score>highScore) highScore=score;

    Graphics::fill_rect(0, HUD_Y, TFT_W, HUD_H, BLACK);

    // Mini pac-mans por vida
    for(uint8_t i=0;i<lives&&i<5;i++)
        Graphics::draw_pacman_scaled(8+i*14, HUD_Y+HUD_H/2, true, 3, 1);

    char buf[28];
    sprintf(buf,"S:%u H:%u L%u", score, highScore, level);
    Graphics::draw_string(52, HUD_Y+2, buf, YELLOW, BLACK, 1);
    Graphics::draw_hline(0, HUD_Y-1, TFT_W, WALL_EDGE);
}

// ============================================================
//  PANTALLA DE TÍTULO PROFESIONAL
// ============================================================
void show_title_professional(void) {
    Graphics::fill_screen(BLACK);
    
    // Título directo sin animaciones
    Graphics::draw_string(26, 6,  "PAC-MAN",    YELLOW, BLACK, 3);
    Graphics::draw_string(24, 4,  "PAC-MAN",    COLOR565(200,100,0), BLACK, 3);
    Graphics::draw_string(25, 5,  "PAC-MAN",    YELLOW, BLACK, 3);
    Graphics::draw_string(38, 40, "PIC32MX795", CYAN, BLACK, 2);
    
    // Pac-Man (solo una vez, sin animación)
    Graphics::draw_pacman_scaled(120, 105, true, 3, 3);
    
    // Fantasmas
    const uint16_t gc[4] = {RED, PINK, CYAN, ORANGE};
    for(int i = 0; i < 4; i++) {
        Graphics::draw_ghost_scaled(28 + i*48, 158, gc[i], false, 3);
    }
    
    // Instrucciones
    Graphics::draw_string(12, 215, "POWER PELLET MAKES GHOSTS SCARED!", YELLOW, BLACK, 1);
    Graphics::draw_string(28, 225, "EAT FRIGHTENED GHOSTS: 200 PTS!", CYAN, BLACK, 1);
    
    // Barra de progreso RÁPIDA (reducida de 100% a 20%)
    for(uint8_t progress = 0; progress <= 20; progress += 5) {
        Graphics::fill_rect(20, 235, (uint16_t)(200 * progress / 20), 4, GREEN);
        delay_ms(20);  // Mantiene el delay pero solo 4 iteraciones
    }
    
    sound_start();
    delay_ms(500);  // REDUCIDO de 1000ms a 500ms
    
    // Transición rápida
    for(uint8_t fade = 0; fade < 8; fade++) {  // REDUCIDO de 15 a 8
        Graphics::fill_rect(0, 0, TFT_W, TFT_H, COLOR565(0, 0, fade * 8));
        delay_ms(10);  // REDUCIDO de 20ms a 10ms
    }
}
// ============================================================
//  PANTALLA LEVEL CLEAR MEJORADA
// ============================================================
void show_level_clear_enhanced(void) {
    sound_level_up();
    
    // Animación más rápida (reducida de 8 a 4 parpadeos)
    for(uint8_t f = 0; f < 4; f++) {
        uint16_t color = (f & 1) ? WALL_EDGE : BLACK;
        Graphics::fill_rect(MAZE_OX, MAZE_OY, MAZE_W*CELL_W, MAZE_H*CELL_H, color);
        delay_ms(40);  // REDUCIDO de 80ms a 40ms
    }
    
    // Menos estrellas (reducido de 30 a 10)
    Graphics::fill_screen(BLACK);
    for(int star = 0; star < 10; star++) {
        int16_t sx = rand() % TFT_W;
        int16_t sy = rand() % TFT_H;
        Graphics::draw_pixel(sx, sy, WHITE);
    }
    
    char buf[32];
    sprintf(buf, "LEVEL %u COMPLETE!", level);
    int len = 0;
    while(buf[len]) len++;
    int16_t text_x = (TFT_W - len * 12) / 2;
    
    // Rebote más rápido (reducido de 10 a 4)
    for(int bounce = 0; bounce < 4; bounce++) {
        int16_t offset = (3 - bounce);
        if(offset < 0) offset = -offset;
        offset *= 2;
        Graphics::fill_rect(0, TFT_H/2 - 20, TFT_W, 60, BLACK);
        Graphics::draw_string(text_x, TFT_H/2 - 10 + offset, buf, YELLOW, BLACK, 2);
        delay_ms(30);  // REDUCIDO de 50ms a 30ms
    }
    
    // Mostrar estadísticas
    Graphics::fill_rect(0, TFT_H/2 - 15, TFT_W, 40, BLACK);
    sprintf(buf, "SCORE: %u", score);
    Graphics::draw_string((TFT_W - 120)/2, TFT_H/2 - 5, buf, WHITE, BLACK, 1);
    sprintf(buf, "BONUS: %u", 100 * level);
    Graphics::draw_string((TFT_W - 100)/2, TFT_H/2 + 10, buf, CYAN, BLACK, 1);
    
    delay_ms(800);  // REDUCIDO de 2000ms a 800ms
    
    // Transición más rápida (reducida de 20 a 10)
    for(uint8_t trans = 0; trans < 10; trans++) {
        Graphics::fill_rect(0, 0, TFT_W, TFT_H, COLOR565(0, trans * 10, 0));
        delay_ms(8);  // REDUCIDO de 15ms a 8ms
    }
    
    level++;
}

// ============================================================
//  PANTALLA GAME OVER PROFESIONAL
// ============================================================
void show_game_over_professional(void) {
    sound_death();
    
    // Descenso más rápido (pasos más grandes)
    for(int y = -30; y <= (int)(TFT_H/2 - 30); y += 15) {  // Cambiado de -50 a -30, paso 15
        Graphics::fill_rect(0, 0, TFT_W, TFT_H, BLACK);
        Graphics::draw_string((TFT_W - 108)/2, y, "GAME OVER", RED, BLACK, 2);
        delay_ms(20);  // REDUCIDO de 30ms a 20ms
    }
    
    delay_ms(200);  // REDUCIDO de 500ms a 200ms
    
    char buf[30];
    
    // Menos parpadeos (reducido de 6 a 3)
    for(int flash = 0; flash < 3; flash++) {
        uint16_t color = (flash & 1) ? YELLOW : WHITE;
        sprintf(buf, "SCORE: %u", score);
        Graphics::fill_rect((TFT_W - 100)/2, TFT_H/2 - 10, 110, 20, BLACK);
        Graphics::draw_string((TFT_W - 100)/2, TFT_H/2 - 10, buf, color, BLACK, 1);
        delay_ms(100);  // REDUCIDO de 200ms a 100ms
    }
    
    // Menos parpadeos para récord
    if(score >= highScore) {
        for(int i = 0; i < 3; i++) {  // REDUCIDO de 5 a 3
            Graphics::fill_rect((TFT_W - 100)/2, TFT_H/2 + 5, 110, 20, BLACK);
            Graphics::draw_string((TFT_W - 140)/2, TFT_H/2 + 5, "? NEW RECORD! ?", YELLOW, BLACK, 1);
            delay_ms(80);  // REDUCIDO de 150ms a 80ms
            Graphics::fill_rect((TFT_W - 100)/2, TFT_H/2 + 5, 110, 20, BLACK);
            delay_ms(50);  // REDUCIDO de 100ms a 50ms
        }
    } else {
        sprintf(buf, "BEST: %u", highScore);
        Graphics::draw_string((TFT_W - 100)/2, TFT_H/2 + 5, buf, CYAN, BLACK, 1);
    }
    
    // Mensaje de reinicio (reducido de 8 a 4 pulsos)
    for(int pulse = 0; pulse < 4; pulse++) {
        uint16_t color = (pulse % 2) ? WHITE : GRAY;
        Graphics::draw_string((TFT_W - 120)/2, TFT_H - 30, "PRESS RESET", color, BLACK, 1);
        delay_ms(150);  // REDUCIDO de 300ms a 150ms
    }
}

// ============================================================
//  GAME LOOP
// ============================================================
void game_loop(void) {
    save_template();
    show_title_professional();
    
    score = 0; 
    lives = 3; 
    level = 1; 
    highScore = 0;
    
restart_game:
    init_game();
    draw_maze();
    draw_status();
    Graphics::draw_pacman(pacX, pacY, true, pacDir);
    for(int i = 0; i < 4; i++)
        Graphics::draw_ghost(ghostX[i], ghostY[i], GHOST_COLORS[i], false);
    
    sound_start();
    Graphics::draw_string((TFT_W - 140)/2, HUD_Y - 15, "READY?", YELLOW, BLACK, 1);
    delay_ms(300);  // REDUCIDO de 800ms a 300ms
    Graphics::fill_rect(0, HUD_Y - 20, TFT_W, 20, BLACK);
    
    // Resto del código...
}

} // namespace GameEngine