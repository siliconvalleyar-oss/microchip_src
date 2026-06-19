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

// ==================== PARÁMETROS DE PANTALLA ====================
#define CELL_W      24
#define CELL_H      24
#define MAZE_W      10
#define MAZE_H      10
#define PACMAN_SIZE 20

#endif
