/**
 * @file HardwareProfile.h
 * @brief Configuración de hardware: pines, SPI4, retardos, sistema.
 */

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include <xc.h>
#include <stdint.h>
#include <stdlib.h>

// ==================== CONFIGURACIÓN DEL SISTEMA ====================
// (Estos pragmas deben estar en main.cpp, pero los dejamos aquí como referencia)
// #pragma config FPLLMUL  = MUL_20
// #pragma config FPLLIDIV = DIV_2
// #pragma config FPLLODIV = DIV_1
// #pragma config FWDTEN   = OFF
// #pragma config POSCMOD  = XT
// #pragma config FNOSC    = PRIPLL
// #pragma config FPBDIV   = DIV_1

#define SYS_FREQ 80000000UL

// ==================== PINES DE LA PANTALLA ====================
#define PIN_BL      0   // RB0
#define PIN_DC      1   // RB1
#define PIN_RST     2   // RB2
#define BL_LAT      LATBbits.LATB0
#define DC_LAT      LATBbits.LATB1
#define RST_LAT     LATBbits.LATB2
#define BL_TRIS     TRISBbits.TRISB0
#define DC_TRIS     TRISBbits.TRISB1
#define RST_TRIS    TRISBbits.TRISB2

// ==================== PINES DE BOTONES (opcionales) ====================
// #define USE_BUTTONS
#ifdef USE_BUTTONS
#define BTN_LEFT_PIN  7
#define BTN_RIGHT_PIN 8
#define BTN_FIRE_PIN  9
#define BTN_LEFT      (!(PORTB >> BTN_LEFT_PIN) & 1)
#define BTN_RIGHT     (!(PORTB >> BTN_RIGHT_PIN) & 1)
#define BTN_FIRE      (!(PORTB >> BTN_FIRE_PIN) & 1)
#endif

// ==================== PARÁMETROS DE PANTALLA ====================
#define TFT_W       240
#define TFT_H       240
#define SHIP_W      16
#define SHIP_H      24

// ==================== COLORES ÚTILES ====================
#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define YELLOW      0xFFE0
#define ORANGE      0xFC00
#define GRAY        0x8410
#define COLOR565(r,g,b) ((uint16_t)((((r)&0xF8)<<8) | (((g)&0xFC)<<3) | ((b)>>3)))

// ==================== PROTOTIPOS DE FUNCIONES DE HARDWARE ====================
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
#endif // HARDWARE_PROFILE_H
