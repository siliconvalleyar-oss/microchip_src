/**
 * @file main.cpp
 * @brief ST7789 Space Shooter v1.0
 *
 * Demostración de animación con nave espacial, estrellas y disparos.
 * Usa el driver base de ST7789_common/.
 */

#include "../../ST7789_common/st7789_driver.h"

// ======================== SPRITE NAVE 16x24 ========================
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

#undef T
#undef B
#undef C
#undef E
#undef W
#undef G
#undef F
#undef D

#define SHIP_W  16
#define SHIP_H  24

void draw_ship(int16_t x, int16_t y, uint16_t bg_color) {
    for(int16_t row=0; row<SHIP_H; row++)
        for(int16_t col=0; col<SHIP_W; col++) {
            uint16_t px = ship_sprite[row][col];
            draw_pixel(x+col, y+row, px ? px : bg_color);
        }
}

void erase_ship(int16_t x, int16_t y, uint16_t bg_color) {
    fill_rect(x, y, SHIP_W, SHIP_H, bg_color);
}

// ======================== ESTRELLAS DE FONDO ========================
#define MAX_STARS 60
typedef struct { int16_t x,y; uint8_t speed; } Star;
static Star stars[MAX_STARS];

void init_stars(void) {
    for(int i=0;i<MAX_STARS;i++){
        stars[i].x     = lcg_rand() % TFT_W;
        stars[i].y     = lcg_rand() % TFT_H;
        stars[i].speed = (lcg_rand() % 3) + 1;
    }
}

void update_stars(void) {
    for(int i=0;i<MAX_STARS;i++){
        draw_pixel(stars[i].x, stars[i].y, BLACK);
        stars[i].y += stars[i].speed;
        if(stars[i].y >= TFT_H){ stars[i].y=0; stars[i].x=lcg_rand()%TFT_W; }
        uint8_t b = 80 + (lcg_rand() % 176);
        draw_pixel(stars[i].x, stars[i].y, COLOR565(b,b,b));
    }
}

// ======================== MAIN ========================
int main(void) {
    BL_TRIS=0; DC_TRIS=0; RST_TRIS=0;
    DC_LAT=0; RST_LAT=1; BL_LAT=0;

    spi4_init();
    init_display();
    BL_LAT=1;

    lcg_srand(_CP0_GET_COUNT());

    fill_screen(BLACK);
    init_stars();

    int16_t ship_x = (TFT_W - SHIP_W) / 2;
    int16_t ship_y = TFT_H - SHIP_H - 20;
    int8_t dir_x = 1;
    uint32_t score = 0;
    uint8_t tick = 0;

    draw_ship(ship_x, ship_y, BLACK);

    // Título
    draw_string(55, 100, "SPACE", CYAN, BLACK, 3);
    draw_string(40, 130, "SHOOTER", CYAN, BLACK, 3);
    draw_string(30, 165, "PIC32MX795", COLOR565(100,100,255), BLACK, 2);
    delay_ms(2000);
    fill_rect(0, 90, TFT_W, 100, BLACK);
    draw_ship(ship_x, ship_y, BLACK);

    while(1) {
        update_stars();
        erase_ship(ship_x, ship_y, BLACK);

        ship_x += dir_x;
        if(ship_x <= 5)              { ship_x=5; dir_x=1; }
        if(ship_x >= TFT_W-SHIP_W-5) { ship_x=TFT_W-SHIP_W-5; dir_x=-1; }

        draw_ship(ship_x, ship_y, BLACK);

        if((tick % 20) == 0) { score += 10; draw_string(90, 3, "SCORE:", WHITE, BLACK, 1); }
        tick++;
        delay_ms(16);
    }
}
