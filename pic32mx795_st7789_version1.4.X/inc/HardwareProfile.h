#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include <xc.h>
#include <stdint.h>
#include <stdlib.h>

// ==================== CONFIGURACIÓN DEL SISTEMA ====================
// Estas líneas deben ir en main.cpp, pero las dejamos como referencia
// #pragma config FPLLMUL  = MUL_20
// #pragma config FPLLIDIV = DIV_2
// #pragma config FPLLODIV = DIV_1
// #pragma config FWDTEN   = OFF
// #pragma config POSCMOD  = XT
// #pragma config FNOSC    = PRIPLL
// #pragma config FPBDIV   = DIV_1

#define SYS_FREQ 80000000UL

// ==================== PINES DE PANTALLA ====================
#define PIN_BL      0   // RB0
#define PIN_DC      1   // RB1
#define PIN_RST     2   // RB2
#define BL_LAT      LATBbits.LATB0
#define DC_LAT      LATBbits.LATB1
#define RST_LAT     LATBbits.LATB2
#define BL_TRIS     TRISBbits.TRISB0
#define DC_TRIS     TRISBbits.TRISB1
#define RST_TRIS    TRISBbits.TRISB2

// ==================== PINES DE BOTONES ====================
// Si no usas botones, comenta USE_BUTTONS
// #define USE_BUTTONS
#ifdef USE_BUTTONS
#define BTN_LEFT_PIN   7   // RB7
#define BTN_RIGHT_PIN  8   // RB8
#define BTN_UP_PIN     9   // RB9
#define BTN_DOWN_PIN   10  // RB10
#define BTN_LEFT       (!(PORTB >> BTN_LEFT_PIN) & 1)
#define BTN_RIGHT      (!(PORTB >> BTN_RIGHT_PIN) & 1)
#define BTN_UP         (!(PORTB >> BTN_UP_PIN) & 1)
#define BTN_DOWN       (!(PORTB >> BTN_DOWN_PIN) & 1)
#endif

// ==================== PIN DE SONIDO ====================
#define SOUND_PIN      11   // RB11
#define SOUND_TRIS     TRISBbits.TRISB12
#define SOUND_LAT      LATBbits.LATB12
#define SOUND_ON()     SOUND_LAT = 1
#define SOUND_OFF()    SOUND_LAT = 0

// ==================== PARÁMETROS DE PANTALLA ====================
#define TFT_W       240
#define TFT_H       240
#define CELL_W      24      // Cada celda del laberinto: 24x24 píxeles
#define CELL_H      24
#define MAZE_W      10      // 10 celdas de ancho (240/24 = 10)
#define MAZE_H      10      // 10 celdas de alto
#define PACMAN_SIZE 20      // Tamaño del sprite de Pac-Man (20x20)

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
#define DARK_BLUE   0x0010
#define DARK_GREEN  0x0400
#define PINK        0xF81F
#define LIME        0xAFE0
#define COLOR565(r,g,b) ((uint16_t)((((r)&0xF8)<<8) | (((g)&0xFC)<<3) | ((b)>>3)))

// ==================== PROTOTIPOS DE FUNCIONES HARDWARE ====================
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
