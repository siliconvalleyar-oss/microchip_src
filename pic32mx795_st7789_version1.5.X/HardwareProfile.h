#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include <xc.h>
#include <stdint.h>
#include <stdlib.h>

#define SYS_FREQ 80000000UL

// ==================== PINES DE PANTALLA ====================
#define PIN_BL      0
#define PIN_DC      1
#define PIN_RST     2
#define BL_LAT      LATBbits.LATB0
#define DC_LAT      LATBbits.LATB1
#define RST_LAT     LATBbits.LATB2
#define BL_TRIS     TRISBbits.TRISB0
#define DC_TRIS     TRISBbits.TRISB1
#define RST_TRIS    TRISBbits.TRISB2

// ==================== PINES DE BOTONES ====================
// Descomenta para usar botones físicos
// #define USE_BUTTONS
#ifdef USE_BUTTONS
#define BTN_LEFT_PIN   7
#define BTN_RIGHT_PIN  8
#define BTN_UP_PIN     9
#define BTN_DOWN_PIN   10
#define BTN_LEFT       (!(PORTB >> BTN_LEFT_PIN) & 1)
#define BTN_RIGHT      (!(PORTB >> BTN_RIGHT_PIN) & 1)
#define BTN_UP         (!(PORTB >> BTN_UP_PIN) & 1)
#define BTN_DOWN       (!(PORTB >> BTN_DOWN_PIN) & 1)
#endif

// ==================== PIN DE SONIDO ====================
#define SOUND_PIN      11
#define SOUND_TRIS     TRISBbits.TRISB12
#define SOUND_LAT      LATBbits.LATB12
#define SOUND_ON()     SOUND_LAT = 1
#define SOUND_OFF()    SOUND_LAT = 0

// ==================== PANTALLA ====================
#define TFT_W       240
#define TFT_H       240

// ---- Laberinto ----
// 9x9 celdas de 24x24 px → 216x216 px zona de juego
// Deja margen de 12 px a cada lado  (12 + 216 + 12 = 240)
#define MAZE_COLS   9          // celdas en X
#define MAZE_ROWS   9          // celdas en Y
// Alias de compatibilidad
#define MAZE_W      MAZE_COLS
#define MAZE_H      MAZE_ROWS

#define CELL_W      24         // px por celda (ancho)
#define CELL_H      24         // px por celda (alto)

// Offset de pantalla para centrar el laberinto
#define MAZE_OX     12         // (240 - 9*24) / 2
#define MAZE_OY     12

// ---- Sprites ----
// El pasillo interior mide CELL_W - 2*WALL_THICK = 24-8 = 16 px
// Pac-Man cabe perfectamente con 16 px de diámetro
#define WALL_THICK  4          // grosor visual del borde de pared
#define PACMAN_R    9          // radio del círculo de Pac-Man (sprite 18x18)
#define PACMAN_SIZE 18         // ancho/alto del bounding-box

#define GHOST_SIZE  18         // igual que Pac-Man para consistencia

// ---- Zona HUD ----
#define HUD_Y       (MAZE_OY + MAZE_ROWS*CELL_H + 4)   // ~220
#define HUD_H       (TFT_H - HUD_Y)                    // ~20

// ==================== COLORES ====================
#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define YELLOW      0xFFE0
#define ORANGE      0xFC00
#define MAGENTA     0xF81F
#define GRAY        0x8410
#define DARK_BLUE   0x000C
#define PINK        0xF81F
#define COLOR565(r,g,b) ((uint16_t)((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|((b)>>3)))

// Colores del laberinto
#define WALL_COLOR      COLOR565(0,0,140)
#define WALL_EDGE       COLOR565(30,80,255)
#define FLOOR_COLOR     BLACK
#define DOT_COLOR       COLOR565(255,220,100)
#define POWER_COLOR     WHITE

// ==================== PROTOTIPOS HARDWARE ====================
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
void spi4_init(void);
void spi4_write(uint8_t d);
void write_cmd(uint8_t c);
void write_data(uint8_t d);
void push_color(uint16_t color);
void reset_display(void);
void init_display(void);
void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

#endif
