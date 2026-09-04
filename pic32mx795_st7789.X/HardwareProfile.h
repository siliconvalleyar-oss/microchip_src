#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include "../../ST7789_common/st7789_driver.h"

// ==================== PINES DE BOTONES ====================
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
#define MAZE_COLS   9
#define MAZE_ROWS   9
#define MAZE_W      MAZE_COLS
#define MAZE_H      MAZE_ROWS

#define CELL_W      24
#define CELL_H      24
#define MAZE_OX     12
#define MAZE_OY     12
#define WALL_THICK  4
#define PACMAN_R    9
#define PACMAN_SIZE 18
#define GHOST_SIZE  18
#define HUD_Y       (MAZE_OY + MAZE_ROWS*CELL_H + 4)
#define HUD_H       (TFT_H - HUD_Y)

// Colores del laberinto
#define WALL_COLOR      COLOR565(0,0,140)
#define WALL_EDGE       COLOR565(30,80,255)
#define FLOOR_COLOR     BLACK
#define DOT_COLOR       COLOR565(255,220,100)
#define POWER_COLOR     WHITE

#endif
