/**
 * @file main.cpp
 * @brief ST7789 Space Shooter v1.1 — Enemigos, proyectiles, colisiones
 *
 * Usa el driver base de ST7789_common/.
 */

#include "../../ST7789_common/st7789_driver.h"
#include <cstdio>

// ======================== PINES DE BOTONES ========================
#define BTN_LEFT_PIN  7
#define BTN_RIGHT_PIN 8
#define BTN_FIRE_PIN  9
#define BTN_LEFT      (!(PORTB >> BTN_LEFT_PIN) & 1)
#define BTN_RIGHT     (!(PORTB >> BTN_RIGHT_PIN) & 1)
#define BTN_FIRE      (!(PORTB >> BTN_FIRE_PIN) & 1)

// ======================== SPRITE NAVE 16x24 ========================
#define SHIP_W  16
#define SHIP_H  24

#define T 0x0000
#define B COLOR565(100,220,255)
#define C COLOR565(200,240,255)
#define E COLOR565(255,120,0)
#define W COLOR565(0,80,200)
#define G COLOR565(160,200,220)
#define F COLOR565(255,240,0)
#define D COLOR565(255,160,0)

static const uint16_t ship_sprite[24][16] = {
    {T,T,T,T,T,T,T,B,B,T,T,T,T,T,T,T},
    {T,T,T,T,T,T,B,C,C,B,T,T,T,T,T,T},
    {T,T,T,T,T,B,C,C,C,C,B,T,T,T,T,T},
    {T,T,T,T,B,C,C,C,C,C,C,B,T,T,T,T},
    {T,T,T,B,C,C,G,C,C,G,C,C,B,T,T,T},
    {T,T,W,B,B,B,B,B,B,B,B,B,B,W,T,T},
    {T,W,W,B,C,C,C,C,C,C,C,C,B,W,W,T},
    {W,W,W,B,C,C,C,C,C,C,C,C,B,W,W,W},
    {W,W,W,B,C,G,C,C,C,C,G,C,B,W,W,W},
    {W,W,B,B,B,B,B,B,B,B,B,B,B,B,W,W},
    {T,W,B,B,G,B,E,E,E,E,B,G,B,B,W,T},
    {T,W,B,B,B,B,E,E,E,E,B,B,B,B,W,T},
    {T,T,W,B,B,B,B,B,B,B,B,B,B,W,T,T},
    {T,T,W,B,E,E,B,B,B,B,E,E,B,W,T,T},
    {T,T,W,B,E,E,B,B,B,B,E,E,B,W,T,T},
    {T,T,T,B,E,E,B,E,E,B,E,E,B,T,T,T},
    {T,T,T,B,E,E,B,E,E,B,E,E,B,T,T,T},
    {T,T,T,T,F,F,T,F,F,T,F,F,T,T,T,T},
    {T,T,T,T,F,D,T,F,D,T,F,D,T,T,T,T},
    {T,T,T,T,D,F,T,D,F,T,D,F,T,T,T,T},
    {T,T,T,T,T,F,T,T,F,T,T,F,T,T,T,T},
    {T,T,T,T,T,D,T,T,D,T,T,D,T,T,T,T},
    {T,T,T,T,T,F,T,T,F,T,T,F,T,T,T,T},
    {T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T},
};
#undef T B C E W G F D

void draw_ship(int16_t x, int16_t y, uint16_t bg) {
    for(int16_t r=0;r<SHIP_H;r++) for(int16_t c=0;c<SHIP_W;c++)
        draw_pixel(x+c,y+r, ship_sprite[r][c] ? ship_sprite[r][c] : bg);
}
void erase_ship(int16_t x, int16_t y) { fill_rect(x,y,SHIP_W,SHIP_H,BLACK); }

// ======================== NAMESPACE DEL JUEGO ========================
namespace Game {
    const uint8_t MAX_ENEMIES = 8;
    const uint8_t MAX_BULLETS = 5;
    const uint8_t INVINCIBLE_FRAMES = 60;

    struct Enemy { int16_t x,y; uint8_t w,h; int8_t speed; bool active; uint16_t color; };
    struct Bullet { int16_t x,y; int8_t speed; bool active; };

    Enemy enemies[MAX_ENEMIES];
    Bullet bullets[MAX_BULLETS];
    uint8_t lives;
    uint32_t score;
    int16_t shipX, shipY;
    int8_t invincibleCounter;
    bool shipVisible;
    uint8_t lastFireState;

    void init(void) {
        for(int i=0;i<MAX_ENEMIES;i++) enemies[i].active=false;
        for(int i=0;i<MAX_BULLETS;i++) bullets[i].active=false;
        lives=3; score=0;
        shipX=(TFT_W-SHIP_W)/2; shipY=TFT_H-SHIP_H-10;
        invincibleCounter=0; shipVisible=true; lastFireState=1;
        fill_screen(BLACK);
    }

    void spawnEnemy(void) {
        for(int i=0;i<MAX_ENEMIES;i++) if(!enemies[i].active){
            enemies[i].w=12; enemies[i].h=12;
            enemies[i].x=lcg_rand()%(TFT_W-enemies[i].w);
            enemies[i].y=-enemies[i].h;
            enemies[i].speed=1+(lcg_rand()%3);
            enemies[i].color=(lcg_rand()%2)?RED:COLOR565(255,100,100);
            enemies[i].active=true; break;
        }
    }

    void updateEnemies(void) {
        for(int i=0;i<MAX_ENEMIES;i++) if(enemies[i].active){
            fill_rect(enemies[i].x,enemies[i].y,enemies[i].w,enemies[i].h,BLACK);
            enemies[i].y+=enemies[i].speed;
            if(enemies[i].y>TFT_H){ enemies[i].active=false; if(lives>0){lives--; invincibleCounter=INVINCIBLE_FRAMES;} }
            else fill_rect(enemies[i].x,enemies[i].y,enemies[i].w,enemies[i].h,enemies[i].color);
        }
        static uint8_t sc=0;
        if(++sc>20){sc=0; if(lcg_rand()%3==0) spawnEnemy();}
    }

    void updateBullets(void) {
        for(int i=0;i<MAX_BULLETS;i++) if(bullets[i].active){
            draw_pixel(bullets[i].x,bullets[i].y,BLACK);
            bullets[i].y+=bullets[i].speed;
            if(bullets[i].y<0) bullets[i].active=false;
            else draw_pixel(bullets[i].x,bullets[i].y,YELLOW);
        }
    }

    void updateShip(void) {
        static uint8_t lp=0,rp=0;
        static int16_t px=-1,py=-1;
        if(BTN_LEFT){if(!lp){shipX-=8;lp=1;}}else lp=0;
        if(BTN_RIGHT){if(!rp){shipX+=8;rp=1;}}else rp=0;
        if(shipX<2)shipX=2; if(shipX>TFT_W-SHIP_W-2)shipX=TFT_W-SHIP_W-2;
        if(!BTN_FIRE&&lastFireState) for(int i=0;i<MAX_BULLETS;i++) if(!bullets[i].active){
            bullets[i].x=shipX+SHIP_W/2; bullets[i].y=shipY-2; bullets[i].speed=-5; bullets[i].active=true; break;
        }
        lastFireState=BTN_FIRE;
        if(invincibleCounter>0){invincibleCounter--; shipVisible=(invincibleCounter&4)?true:false;}
        else shipVisible=true;
        if(shipX!=px||shipY!=py){
            if(px!=-1) erase_ship(px,py);
            if(shipVisible) draw_ship(shipX,shipY,BLACK);
            px=shipX; py=shipY;
        }
    }

    void checkCollisions(void) {
        for(int b=0;b<MAX_BULLETS;b++) if(bullets[b].active)
            for(int e=0;e<MAX_ENEMIES;e++) if(enemies[e].active)
                if(bullets[b].x>=enemies[e].x&&bullets[b].x<=enemies[e].x+enemies[e].w&&
                   bullets[b].y>=enemies[e].y&&bullets[b].y<=enemies[e].y+enemies[e].h){
                    bullets[b].active=false; enemies[e].active=false; score+=10;
                    fill_rect(0,0,TFT_W,14,BLACK);
                    char buf[12]; std::sprintf(buf,"%lu",score);
                    draw_string(TFT_W/2-30,3,buf,WHITE,BLACK,1);
                    draw_hline(0,13,TFT_W,GRAY);
                    break;
                }
        if(invincibleCounter==0) for(int e=0;e<MAX_ENEMIES;e++) if(enemies[e].active)
            if(shipX<enemies[e].x+enemies[e].w&&shipX+SHIP_W>enemies[e].x&&
               shipY<enemies[e].y+enemies[e].h&&shipY+SHIP_H>enemies[e].y){
                lives--; invincibleCounter=INVINCIBLE_FRAMES; enemies[e].active=false;
                fill_rect(0,0,TFT_W,14,BLACK);
                if(lives==0){draw_string(TFT_W/2-60,TFT_H/2-10,"GAME OVER",RED,BLACK,2); while(1);}
                break;
            }
    }

    void gameLoop(void) {
        fill_screen(BLACK);
        draw_string(55,100,"SPACE",CYAN,BLACK,3);
        draw_string(40,130,"SHOOTER",CYAN,BLACK,3);
        delay_ms(2000);
        fill_rect(0,90,TFT_W,100,BLACK);
        init();
        for(int i=0;i<3;i++) spawnEnemy();
        while(1){
            updateShip(); updateBullets(); updateEnemies(); checkCollisions();
            delay_ms(16);
        }
    }
}

// ======================== MAIN ========================
int main(void) {
    BL_TRIS=0; DC_TRIS=0; RST_TRIS=0;
    DC_LAT=0; RST_LAT=1; BL_LAT=0;

    // Botones como entrada con pull-up
    TRISBSET=(1<<BTN_LEFT_PIN)|(1<<BTN_RIGHT_PIN)|(1<<BTN_FIRE_PIN);
    CNPUE|=(1<<BTN_LEFT_PIN)|(1<<BTN_RIGHT_PIN)|(1<<BTN_FIRE_PIN);

    spi4_init();
    init_display();
    BL_LAT=1;

    lcg_srand(_CP0_GET_COUNT());

    Game::gameLoop();
    return 0;
}
