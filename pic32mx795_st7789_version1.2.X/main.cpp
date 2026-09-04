/**
 * @file main.cpp
 * @brief ST7789 Space Shooter v1.2 — Enemigos coloreados con paleta
 *
 * Usa el driver base de ST7789_common/.
 */

#include "../../ST7789_common/st7789_driver.h"
#include <cstdio>

// ======================== SPRITE NAVE 16x24 ========================
#define SHIP_W  16
#define SHIP_H  24

// Paleta de colores para la nave (se reemplaza en draw_ship_palette)
static const uint16_t ship_sprite[24][16] = {
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0CFF,0x0CFF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0CFF,0xE0FF,0xE0FF,0x0CFF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0CFF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0x0CFF,0x0000,0x0000,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0000,0x0CFF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0x0CFF,0x0000,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0CFF,0xE0FF,0xE0FF,0xACEF,0xE0FF,0xE0FF,0xACEF,0xE0FF,0xE0FF,0x0CFF,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x04AA,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x04AA,0x0000,0x0000},
    {0x0000,0x04AA,0x04AA,0x0CFF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0x0CFF,0x04AA,0x04AA,0x0000},
    {0x04AA,0x04AA,0x04AA,0x0CFF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0x0CFF,0x04AA,0x04AA,0x04AA},
    {0x04AA,0x04AA,0x04AA,0x0CFF,0xE0FF,0xACEF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xACEF,0xE0FF,0x0CFF,0x04AA,0x04AA,0x04AA},
    {0x04AA,0x04AA,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x04AA,0x04AA},
    {0x0000,0x04AA,0x0CFF,0x0CFF,0xACEF,0x0CFF,0xFC60,0xFC60,0xFC60,0xFC60,0x0CFF,0xACEF,0x0CFF,0x0CFF,0x04AA,0x0000},
    {0x0000,0x04AA,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0xFC60,0xFC60,0xFC60,0xFC60,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x04AA,0x0000},
    {0x0000,0x0000,0x04AA,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x04AA,0x0000,0x0000},
    {0x0000,0x0000,0x04AA,0x0CFF,0xFC60,0xFC60,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0xFC60,0xFC60,0x0CFF,0x04AA,0x0000,0x0000},
    {0x0000,0x0000,0x04AA,0x0CFF,0xFC60,0xFC60,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0xFC60,0xFC60,0x0CFF,0x04AA,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0CFF,0xFC60,0xFC60,0x0CFF,0xFC60,0xFC60,0x0CFF,0xFC60,0xFC60,0x0CFF,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0CFF,0xFC60,0xFC60,0x0CFF,0xFC60,0xFC60,0x0CFF,0xFC60,0xFC60,0x0CFF,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0000,0xFFE0,0xFFE0,0x0000,0xFFE0,0xFFE0,0x0000,0xFFE0,0xFFE0,0x0000,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0000,0xFFE0,0xFC00,0x0000,0xFFE0,0xFC00,0x0000,0xFFE0,0xFC00,0x0000,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0000,0xFC00,0xFFE0,0x0000,0xFC00,0xFFE0,0x0000,0xFC00,0xFFE0,0x0000,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0000,0x0000,0xFFE0,0x0000,0x0000,0xFFE0,0x0000,0x0000,0xFFE0,0x0000,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0000,0x0000,0xFC00,0x0000,0x0000,0xFC00,0x0000,0x0000,0xFC00,0x0000,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0000,0x0000,0xFFE0,0x0000,0x0000,0xFFE0,0x0000,0x0000,0xFFE0,0x0000,0x0000,0x0000,0x0000},
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
};

// Colores de referencia del sprite original (v1.0)
#define REF_BODY  COLOR565(100,220,255)
#define REF_COCK  COLOR565(200,240,255)
#define REF_ENG   COLOR565(255,120,0)
#define REF_WING  COLOR565(0,80,200)
#define REF_DET   COLOR565(160,200,220)
#define REF_FIRE  COLOR565(255,240,0)

void draw_ship_palette(int16_t x, int16_t y, uint16_t body, uint16_t cockpit,
                       uint16_t engine, uint16_t wing, uint16_t detail, uint16_t bg) {
    for(int16_t r=0;r<SHIP_H;r++) for(int16_t c=0;c<SHIP_W;c++){
        uint16_t px = ship_sprite[r][c];
        uint16_t color = bg;
        if(px == REF_BODY) color=body; else if(px == REF_COCK) color=cockpit;
        else if(px == REF_ENG) color=engine; else if(px == REF_WING) color=wing;
        else if(px == REF_DET) color=detail; else if(px == REF_FIRE) color=engine;
        else if(px != 0x0000) color=px;
        draw_pixel(x+c, y+r, color);
    }
}

void draw_ship(int16_t x, int16_t y, uint16_t bg) {
    draw_ship_palette(x, y, REF_BODY, REF_COCK, REF_ENG, REF_WING, REF_DET, bg);
}

void erase_ship(int16_t x, int16_t y) { fill_rect(x, y, SHIP_W, SHIP_H, BLACK); }

// ======================== JUEGO ========================
namespace Game {
    enum EnemyType { ENEMY_RED, ENEMY_GREEN, ENEMY_BLUE, ENEMY_ORANGE, ENEMY_PURPLE, NUM_TYPES };
    const uint8_t MAX_E=8, MAX_B=5, INV_FRAMES=60;

    struct Enemy { int16_t x,y; uint8_t w,h; int8_t speed; bool active; EnemyType type; };
    struct Bullet { int16_t x,y; int8_t speed; bool active; };

    Enemy enemies[MAX_E]; Bullet bullets[MAX_B];
    uint8_t lives; uint32_t score;
    int16_t shipX, shipY;
    int8_t invincibleCounter;
    bool shipVisible;

    void drawEnemyShip(int16_t x, int16_t y, EnemyType t) {
        uint16_t bd,ck,eng,wn,dt;
        switch(t) {
            case ENEMY_RED:    bd=COLOR565(200,0,0); ck=COLOR565(255,100,100); eng=COLOR565(255,80,0); wn=COLOR565(150,0,0); dt=COLOR565(180,50,50); break;
            case ENEMY_GREEN:  bd=COLOR565(0,180,0); ck=COLOR565(100,255,100); eng=COLOR565(0,200,0); wn=COLOR565(0,130,0); dt=COLOR565(80,200,80); break;
            case ENEMY_BLUE:   bd=COLOR565(0,0,200); ck=COLOR565(100,100,255); eng=COLOR565(0,100,200); wn=COLOR565(0,0,150); dt=COLOR565(80,80,200); break;
            case ENEMY_ORANGE: bd=COLOR565(255,100,0); ck=COLOR565(255,180,100); eng=COLOR565(255,140,0); wn=COLOR565(200,80,0); dt=COLOR565(255,150,80); break;
            default:           bd=COLOR565(255,0,255); ck=COLOR565(255,150,255); eng=COLOR565(200,0,200); wn=COLOR565(180,0,180); dt=COLOR565(220,100,220);
        }
        draw_ship_palette(x,y,bd,ck,eng,wn,dt,BLACK);
    }

    void init(void) {
        for(int i=0;i<MAX_E;i++) enemies[i].active=false;
        for(int i=0;i<MAX_B;i++) bullets[i].active=false;
        lives=3; score=0;
        shipX=(TFT_W-SHIP_W)/2; shipY=TFT_H-SHIP_H-10;
        invincibleCounter=0; shipVisible=true;
    }

    void spawnEnemy(void) {
        for(int i=0;i<MAX_E;i++) if(!enemies[i].active){
            enemies[i].w=SHIP_W; enemies[i].h=SHIP_H;
            enemies[i].x=lcg_rand()%(TFT_W-enemies[i].w);
            enemies[i].y=-enemies[i].h;
            enemies[i].speed=1+(lcg_rand()%3);
            enemies[i].type=(EnemyType)(lcg_rand()%NUM_TYPES);
            enemies[i].active=true; break;
        }
    }

    void updateEnemies(void) {
        for(int i=0;i<MAX_E;i++) if(enemies[i].active){
            erase_ship(enemies[i].x,enemies[i].y);
            enemies[i].y+=enemies[i].speed;
            if(enemies[i].y>TFT_H){ enemies[i].active=false; if(lives>0){lives--; invincibleCounter=INV_FRAMES;} }
            else drawEnemyShip(enemies[i].x,enemies[i].y,enemies[i].type);
        }
        static uint8_t sc=0;
        if(++sc>25){sc=0; if(lcg_rand()%3==0) spawnEnemy();}
    }

    void updateBullets(void) {
        for(int i=0;i<MAX_B;i++) if(bullets[i].active){
            draw_pixel(bullets[i].x,bullets[i].y,BLACK);
            bullets[i].y+=bullets[i].speed;
            if(bullets[i].y<0) bullets[i].active=false;
            else draw_pixel(bullets[i].x,bullets[i].y,YELLOW);
        }
    }

    void updateShip(void) {
        static int16_t px=-1,py=-1;
        static uint8_t ac=0; static int8_t ad=1;
        if(++ac>=4){shipX+=ad*4; ac=0;}
        if(shipX<=5) ad=1; if(shipX>=TFT_W-SHIP_W-5) ad=-1;
        if(shipX<2)shipX=2; if(shipX>TFT_W-SHIP_W-2)shipX=TFT_W-SHIP_W-2;

        static uint8_t fc=0;
        if(++fc>=20){fc=0;
            for(int i=0;i<MAX_B;i++) if(!bullets[i].active)
                {bullets[i].x=shipX+SHIP_W/2; bullets[i].y=shipY-2; bullets[i].speed=-5; bullets[i].active=true; break;}
        }
        if(invincibleCounter>0){invincibleCounter--; shipVisible=(invincibleCounter&4)?true:false;}
        else shipVisible=true;

        if(shipX!=px||shipY!=py||(invincibleCounter>0&&(invincibleCounter&4)!=((invincibleCounter+1)&4))){
            if(px!=-1) erase_ship(px,py);
            if(shipVisible) draw_ship(shipX,shipY,BLACK);
            px=shipX; py=shipY;
        }
    }

    void checkCollisions(void) {
        for(int b=0;b<MAX_B;b++) if(bullets[b].active) for(int e=0;e<MAX_E;e++) if(enemies[e].active)
            if(bullets[b].x>=enemies[e].x&&bullets[b].x<=enemies[e].x+enemies[e].w&&
               bullets[b].y>=enemies[e].y&&bullets[b].y<=enemies[e].y+enemies[e].h){
                bullets[b].active=false; enemies[e].active=false; score+=10;
                char buf[12]; std::sprintf(buf,"%lu",score);
                fill_rect(0,0,TFT_W,14,BLACK); draw_string(TFT_W/2-30,3,buf,WHITE,BLACK,1);
                break;
            }
        if(invincibleCounter==0) for(int e=0;e<MAX_E;e++) if(enemies[e].active)
            if(shipX<enemies[e].x+enemies[e].w&&shipX+SHIP_W>enemies[e].x&&
               shipY<enemies[e].y+enemies[e].h&&shipY+SHIP_H>enemies[e].y){
                lives--; invincibleCounter=INV_FRAMES; enemies[e].active=false;
                if(lives==0){draw_string(TFT_W/2-60,TFT_H/2-10,"GAME OVER",RED,BLACK,2); while(1);}
                break;
            }
    }

    void gameLoop(void) {
        fill_screen(BLACK);
        init(); for(int i=0;i<3;i++) spawnEnemy();
        while(1){ updateShip(); updateBullets(); updateEnemies(); checkCollisions(); delay_ms(16); }
    }
}

// ======================== MAIN ========================
int main(void) {
    BL_TRIS=0; DC_TRIS=0; RST_TRIS=0;
    DC_LAT=0; RST_LAT=1; BL_LAT=0;
    spi4_init(); init_display(); BL_LAT=1;
    lcg_srand(_CP0_GET_COUNT());
    Game::gameLoop(); return 0;
}
