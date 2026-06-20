/*
 * ST7789 Driver + Space Shooter Avanzado con enemigos tipo nave y colores
 * PIC32MX795F512H (64 pines) | SPI4 - Modo 3
 * Display: GMT130 V1.0 240x240, sin CS
 *
 * Mejoras:
 *  - Enemigos con forma de nave (misma silueta que jugador) y colores distintos
 *  - Movimiento automático de la nave (puedes cambiar a botones)
 *  - Optimización de redibujado (solo lo que cambia)
 *  - Múltiples tipos de enemigos
 */

#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <cstdio>

// ======================== CONFIGURACION BITS ========================
#pragma config FPLLMUL  = MUL_20
#pragma config FPLLIDIV = DIV_2
#pragma config FPLLODIV = DIV_1
#pragma config FWDTEN   = OFF
#pragma config POSCMOD  = XT
#pragma config FNOSC    = PRIPLL
#pragma config FPBDIV   = DIV_1

// ======================== OPCIONES ========================
#define PRESENTACION           // Comentar para omitir presentación
// #define USE_BUTTONS          // Descomentar si tienes botones en RB7,RB8,RB9

// ======================== PINES PANTALLA Y BOTONES ========================
#define BL_LAT      LATBbits.LATB0
#define DC_LAT      LATBbits.LATB1
#define RST_LAT     LATBbits.LATB2
#define BL_TRIS     TRISBbits.TRISB0
#define DC_TRIS     TRISBbits.TRISB1
#define RST_TRIS    TRISBbits.TRISB2

#ifdef USE_BUTTONS
#define BTN_LEFT_PIN  7
#define BTN_RIGHT_PIN 8
#define BTN_FIRE_PIN  9
#define BTN_LEFT      (!(PORTB >> BTN_LEFT_PIN) & 1)
#define BTN_RIGHT     (!(PORTB >> BTN_RIGHT_PIN) & 1)
#define BTN_FIRE      (!(PORTB >> BTN_FIRE_PIN) & 1)
#endif

// ======================== PARAMETROS PANTALLA ========================
#define TFT_W       240
#define TFT_H       240
#define SHIP_W      16
#define SHIP_H      24
#define COLOR565(r,g,b) ((uint16_t)((((r)&0xF8)<<8) | (((g)&0xFC)<<3) | ((b)>>3)))

// Colores predefinidos
#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define YELLOW      0xFFE0
#define ORANGE      0xFC00
#define GRAY        0x8410

// ======================== RETARDOS ========================
static inline void delay_cycles(uint32_t c) {
    uint32_t s = _CP0_GET_COUNT();
    while((_CP0_GET_COUNT() - s) < c);
}
void delay_ms(uint32_t ms) { while(ms--) delay_cycles(40000); }
void delay_us(uint32_t us) { delay_cycles(us * 40); }

// ======================== SPRITE DE NAVE (16x24) ========================
static const uint16_t ship_sprite[24][16] = {
    // fila 0
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0CFF,0x0CFF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
    // fila 1
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0CFF,0xE0FF,0xE0FF,0x0CFF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
    // fila 2
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0CFF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0x0CFF,0x0000,0x0000,0x0000,0x0000,0x0000},
    // fila 3
    {0x0000,0x0000,0x0000,0x0000,0x0CFF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0x0CFF,0x0000,0x0000,0x0000,0x0000},
    // fila 4
    {0x0000,0x0000,0x0000,0x0CFF,0xE0FF,0xE0FF,0xACEF,0xE0FF,0xE0FF,0xACEF,0xE0FF,0xE0FF,0x0CFF,0x0000,0x0000,0x0000},
    // fila 5
    {0x0000,0x0000,0x04AA,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x04AA,0x0000,0x0000},
    // fila 6
    {0x0000,0x04AA,0x04AA,0x0CFF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0x0CFF,0x04AA,0x04AA,0x0000},
    // fila 7
    {0x04AA,0x04AA,0x04AA,0x0CFF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0x0CFF,0x04AA,0x04AA,0x04AA},
    // fila 8
    {0x04AA,0x04AA,0x04AA,0x0CFF,0xE0FF,0xACEF,0xE0FF,0xE0FF,0xE0FF,0xE0FF,0xACEF,0xE0FF,0x0CFF,0x04AA,0x04AA,0x04AA},
    // fila 9
    {0x04AA,0x04AA,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x04AA,0x04AA},
    // fila 10
    {0x0000,0x04AA,0x0CFF,0x0CFF,0xACEF,0x0CFF,0xFC60,0xFC60,0xFC60,0xFC60,0x0CFF,0xACEF,0x0CFF,0x0CFF,0x04AA,0x0000},
    // fila 11
    {0x0000,0x04AA,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0xFC60,0xFC60,0xFC60,0xFC60,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x04AA,0x0000},
    // fila 12
    {0x0000,0x0000,0x04AA,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0x04AA,0x0000,0x0000},
    // fila 13
    {0x0000,0x0000,0x04AA,0x0CFF,0xFC60,0xFC60,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0xFC60,0xFC60,0x0CFF,0x04AA,0x0000,0x0000},
    // fila 14
    {0x0000,0x0000,0x04AA,0x0CFF,0xFC60,0xFC60,0x0CFF,0x0CFF,0x0CFF,0x0CFF,0xFC60,0xFC60,0x0CFF,0x04AA,0x0000,0x0000},
    // fila 15
    {0x0000,0x0000,0x0000,0x0CFF,0xFC60,0xFC60,0x0CFF,0xFC60,0xFC60,0x0CFF,0xFC60,0xFC60,0x0CFF,0x0000,0x0000,0x0000},
    // fila 16
    {0x0000,0x0000,0x0000,0x0CFF,0xFC60,0xFC60,0x0CFF,0xFC60,0xFC60,0x0CFF,0xFC60,0xFC60,0x0CFF,0x0000,0x0000,0x0000},
    // fila 17
    {0x0000,0x0000,0x0000,0x0000,0xFFE0,0xFFE0,0x0000,0xFFE0,0xFFE0,0x0000,0xFFE0,0xFFE0,0x0000,0x0000,0x0000,0x0000},
    // fila 18
    {0x0000,0x0000,0x0000,0x0000,0xFFE0,0xFC00,0x0000,0xFFE0,0xFC00,0x0000,0xFFE0,0xFC00,0x0000,0x0000,0x0000,0x0000},
    // fila 19
    {0x0000,0x0000,0x0000,0x0000,0xFC00,0xFFE0,0x0000,0xFC00,0xFFE0,0x0000,0xFC00,0xFFE0,0x0000,0x0000,0x0000,0x0000},
    // fila 20
    {0x0000,0x0000,0x0000,0x0000,0x0000,0xFFE0,0x0000,0x0000,0xFFE0,0x0000,0x0000,0xFFE0,0x0000,0x0000,0x0000,0x0000},
    // fila 21
    {0x0000,0x0000,0x0000,0x0000,0x0000,0xFC00,0x0000,0x0000,0xFC00,0x0000,0x0000,0xFC00,0x0000,0x0000,0x0000,0x0000},
    // fila 22
    {0x0000,0x0000,0x0000,0x0000,0x0000,0xFFE0,0x0000,0x0000,0xFFE0,0x0000,0x0000,0xFFE0,0x0000,0x0000,0x0000,0x0000},
    // fila 23
    {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
};

// ======================== NAMESPACE GRÁFICO ========================
namespace Graphics {
    // Fuente 5x7 completa
    static const uint8_t font5x7[95][5] = {
        {0x00,0x00,0x00,0x00,0x00}, // ' '
        {0x00,0x00,0x5F,0x00,0x00}, // '!'
        {0x00,0x07,0x00,0x07,0x00}, // '"'
        {0x14,0x7F,0x14,0x7F,0x14}, // '#'
        {0x24,0x2A,0x7F,0x2A,0x12}, // '$'
        {0x23,0x13,0x08,0x64,0x62}, // '%'
        {0x36,0x49,0x55,0x22,0x50}, // '&'
        {0x00,0x05,0x03,0x00,0x00}, // '''
        {0x00,0x1C,0x22,0x41,0x00}, // '('
        {0x00,0x41,0x22,0x1C,0x00}, // ')'
        {0x14,0x08,0x3E,0x08,0x14}, // '*'
        {0x08,0x08,0x3E,0x08,0x08}, // '+'
        {0x00,0x50,0x30,0x00,0x00}, // ','
        {0x08,0x08,0x08,0x08,0x08}, // '-'
        {0x00,0x60,0x60,0x00,0x00}, // '.'
        {0x20,0x10,0x08,0x04,0x02}, // '/'
        {0x3E,0x51,0x49,0x45,0x3E}, // '0'
        {0x00,0x42,0x7F,0x40,0x00}, // '1'
        {0x42,0x61,0x51,0x49,0x46}, // '2'
        {0x21,0x41,0x45,0x4B,0x31}, // '3'
        {0x18,0x14,0x12,0x7F,0x10}, // '4'
        {0x27,0x45,0x45,0x45,0x39}, // '5'
        {0x3C,0x4A,0x49,0x49,0x30}, // '6'
        {0x01,0x71,0x09,0x05,0x03}, // '7'
        {0x36,0x49,0x49,0x49,0x36}, // '8'
        {0x06,0x49,0x49,0x29,0x1E}, // '9'
        {0x00,0x36,0x36,0x00,0x00}, // ':'
        {0x00,0x56,0x36,0x00,0x00}, // ';'
        {0x08,0x14,0x22,0x41,0x00}, // '<'
        {0x14,0x14,0x14,0x14,0x14}, // '='
        {0x00,0x41,0x22,0x14,0x08}, // '>'
        {0x02,0x01,0x51,0x09,0x06}, // '?'
        {0x32,0x49,0x79,0x41,0x3E}, // '@'
        {0x7E,0x11,0x11,0x11,0x7E}, // 'A'
        {0x7F,0x49,0x49,0x49,0x36}, // 'B'
        {0x3E,0x41,0x41,0x41,0x22}, // 'C'
        {0x7F,0x41,0x41,0x22,0x1C}, // 'D'
        {0x7F,0x49,0x49,0x49,0x41}, // 'E'
        {0x7F,0x09,0x09,0x09,0x01}, // 'F'
        {0x3E,0x41,0x49,0x49,0x7A}, // 'G'
        {0x7F,0x08,0x08,0x08,0x7F}, // 'H'
        {0x00,0x41,0x7F,0x41,0x00}, // 'I'
        {0x20,0x40,0x41,0x3F,0x01}, // 'J'
        {0x7F,0x08,0x14,0x22,0x41}, // 'K'
        {0x7F,0x40,0x40,0x40,0x40}, // 'L'
        {0x7F,0x02,0x0C,0x02,0x7F}, // 'M'
        {0x7F,0x04,0x08,0x10,0x7F}, // 'N'
        {0x3E,0x41,0x41,0x41,0x3E}, // 'O'
        {0x7F,0x09,0x09,0x09,0x06}, // 'P'
        {0x3E,0x41,0x51,0x21,0x5E}, // 'Q'
        {0x7F,0x09,0x19,0x29,0x46}, // 'R'
        {0x46,0x49,0x49,0x49,0x31}, // 'S'
        {0x01,0x01,0x7F,0x01,0x01}, // 'T'
        {0x3F,0x40,0x40,0x40,0x3F}, // 'U'
        {0x1F,0x20,0x40,0x20,0x1F}, // 'V'
        {0x3F,0x40,0x38,0x40,0x3F}, // 'W'
        {0x63,0x14,0x08,0x14,0x63}, // 'X'
        {0x07,0x08,0x70,0x08,0x07}, // 'Y'
        {0x61,0x51,0x49,0x45,0x43}, // 'Z'
        {0x00,0x7F,0x41,0x41,0x00}, // '['
        {0x02,0x04,0x08,0x10,0x20}, // '\'
        {0x00,0x41,0x41,0x7F,0x00}, // ']'
        {0x04,0x02,0x01,0x02,0x04}, // '^'
        {0x40,0x40,0x40,0x40,0x40}, // '_'
        {0x00,0x01,0x02,0x04,0x00}, // '`'
        {0x20,0x54,0x54,0x54,0x78}, // 'a'
        {0x7F,0x48,0x44,0x44,0x38}, // 'b'
        {0x38,0x44,0x44,0x44,0x20}, // 'c'
        {0x38,0x44,0x44,0x48,0x7F}, // 'd'
        {0x38,0x54,0x54,0x54,0x18}, // 'e'
        {0x08,0x7E,0x09,0x01,0x02}, // 'f'
        {0x0C,0x52,0x52,0x52,0x3E}, // 'g'
        {0x7F,0x08,0x04,0x04,0x78}, // 'h'
        {0x00,0x44,0x7D,0x40,0x00}, // 'i'
        {0x20,0x40,0x44,0x3D,0x00}, // 'j'
        {0x7F,0x10,0x28,0x44,0x00}, // 'k'
        {0x00,0x41,0x7F,0x40,0x00}, // 'l'
        {0x7C,0x04,0x18,0x04,0x78}, // 'm'
        {0x7C,0x08,0x04,0x04,0x78}, // 'n'
        {0x38,0x44,0x44,0x44,0x38}, // 'o'
        {0x7C,0x14,0x14,0x14,0x08}, // 'p'
        {0x08,0x14,0x14,0x18,0x7C}, // 'q'
        {0x7C,0x08,0x04,0x04,0x08}, // 'r'
        {0x48,0x54,0x54,0x54,0x20}, // 's'
        {0x04,0x3F,0x44,0x40,0x20}, // 't'
        {0x3C,0x40,0x40,0x40,0x7C}, // 'u'
        {0x1C,0x20,0x40,0x20,0x1C}, // 'v'
        {0x3C,0x40,0x30,0x40,0x3C}, // 'w'
        {0x44,0x28,0x10,0x28,0x44}, // 'x'
        {0x0C,0x50,0x50,0x50,0x3C}, // 'y'
        {0x44,0x64,0x54,0x4C,0x44}  // 'z'
    };

    void spi4_init(void);
    void spi4_write(uint8_t d);
    void write_cmd(uint8_t c);
    void write_data(uint8_t d);
    void push_color(uint16_t color);
    void reset_display(void);
    void init_display(void);
    void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void fill_screen(uint16_t color);
    void draw_pixel(int16_t x, int16_t y, uint16_t color);
    void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void draw_hline(int16_t x, int16_t y, int16_t len, uint16_t color);
    void draw_vline(int16_t x, int16_t y, int16_t len, uint16_t color);
    void draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
    void draw_char(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale);
    void draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale);
    void draw_ship_palette(int16_t x, int16_t y, uint16_t body, uint16_t cockpit, uint16_t engine, uint16_t wing, uint16_t detail, uint16_t bg);
    void draw_ship(int16_t x, int16_t y, uint16_t bg_color);
    void erase_ship(int16_t x, int16_t y, uint16_t bg_color);
}

// ======================== IMPLEMENTACIÓN GRÁFICA ========================
void Graphics::spi4_init(void) {
    DDPCON  = 0x00;
    AD1PCFG = 0xFFFF;

    TRISBCLR = (1<<6);   // RB6 = SCK4 salida
    TRISFCLR = (1<<5);   // RF5 = SDO4 salida
    TRISFSET = (1<<4);   // RF4 = SDI4 entrada

#ifdef USE_BUTTONS
    TRISBSET = (1<<BTN_LEFT_PIN) | (1<<BTN_RIGHT_PIN) | (1<<BTN_FIRE_PIN);
    CNPUE |= (1<<BTN_LEFT_PIN) | (1<<BTN_RIGHT_PIN) | (1<<BTN_FIRE_PIN);
#endif

    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    OSCCONCLR = (1<<6);

    volatile uint32_t *RPOR3  = (volatile uint32_t*)0xBF80B33C;
    volatile uint32_t *RPOR2  = (volatile uint32_t*)0xBF80B338;
    volatile uint32_t *RPINR20= (volatile uint32_t*)0xBF80B3A0;

    *RPOR3  = (*RPOR3  & 0xFFFFFF00) | 3;       // SCK4 -> RB6(RP6)
    *RPOR2  = (*RPOR2  & 0x0000FFFF) | (4<<16); // SDO4 -> RF5(RP5)
    *RPINR20= (*RPINR20& 0xFFFFFF00) | 4;       // SDI4 <- RF4(RP4)

    SYSKEY = 0x00000000;

    SPI4CON = 0;
    SPI4BRG  = 0;
    SPI4CONbits.MSTEN = 1;
    SPI4CONbits.MODE16= 0;
    SPI4CONbits.CKP   = 1;
    SPI4CONbits.CKE   = 0;
    SPI4CONbits.SMP   = 0;
    SPI4STATbits.SPIROV = 0;
    SPI4CONbits.ON = 1;
}

void Graphics::spi4_write(uint8_t d) {
    while(SPI4STATbits.SPITBF);
    SPI4BUF = d;
    while(!SPI4STATbits.SPIRBF);
    (void)SPI4BUF;
}

void Graphics::write_cmd(uint8_t c)  { DC_LAT=0; spi4_write(c); }
void Graphics::write_data(uint8_t d) { DC_LAT=1; spi4_write(d); }
void Graphics::push_color(uint16_t color) { spi4_write(color>>8); spi4_write(color&0xFF); }

void Graphics::reset_display(void) {
    RST_LAT=1; delay_ms(10);
    RST_LAT=0; delay_ms(20);
    RST_LAT=1; delay_ms(150);
}

void Graphics::init_display(void) {
    reset_display();
    write_cmd(0x11); delay_ms(120);
    write_cmd(0x36); write_data(0x00);
    write_cmd(0x3A); write_data(0x05);
    write_cmd(0x21);
    write_cmd(0x13);
    write_cmd(0xB2); write_data(0x0C); write_data(0x0C); write_data(0x00); write_data(0x33); write_data(0x33);
    write_cmd(0xB7); write_data(0x35);
    write_cmd(0xBB); write_data(0x37);
    write_cmd(0xC0); write_data(0x2C);
    write_cmd(0xC2); write_data(0x01);
    write_cmd(0xC3); write_data(0x12);
    write_cmd(0xC4); write_data(0x20);
    write_cmd(0xC6); write_data(0x0F);
    write_cmd(0xD0); write_data(0xA4); write_data(0xA1);
    write_cmd(0xE0);
    const uint8_t gp[]={0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
    for(int i=0;i<14;i++) write_data(gp[i]);
    write_cmd(0xE1);
    const uint8_t gn[]={0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
    for(int i=0;i<14;i++) write_data(gn[i]);
    write_cmd(0x29); delay_ms(120);
}

void Graphics::set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    write_cmd(0x2A);
    write_data(x0>>8); write_data(x0&0xFF);
    write_data(x1>>8); write_data(x1&0xFF);
    write_cmd(0x2B);
    write_data(y0>>8); write_data(y0&0xFF);
    write_data(y1>>8); write_data(y1&0xFF);
    write_cmd(0x2C);
    DC_LAT = 1;
}

void Graphics::fill_screen(uint16_t color) {
    set_window(0,0,TFT_W-1,TFT_H-1);
    uint32_t n = (uint32_t)TFT_W * TFT_H;
    uint8_t hi = color>>8, lo = color&0xFF;
    for(uint32_t i=0;i<n;i++){ spi4_write(hi); spi4_write(lo); }
}

void Graphics::draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if((uint16_t)x >= TFT_W || (uint16_t)y >= TFT_H) return;
    set_window(x,y,x,y);
    push_color(color);
}

void Graphics::fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if(x>=TFT_W || y>=TFT_H || w<=0 || h<=0) return;
    if(x<0){w+=x; x=0;}
    if(y<0){h+=y; y=0;}
    if(x+w>TFT_W) w=TFT_W-x;
    if(y+h>TFT_H) h=TFT_H-y;
    set_window(x,y,x+w-1,y+h-1);
    uint32_t n=(uint32_t)w*h;
    uint8_t hi=color>>8, lo=color&0xFF;
    for(uint32_t i=0;i<n;i++){spi4_write(hi); spi4_write(lo);}
}

void Graphics::draw_hline(int16_t x, int16_t y, int16_t len, uint16_t color) { fill_rect(x,y,len,1,color); }
void Graphics::draw_vline(int16_t x, int16_t y, int16_t len, uint16_t color) { fill_rect(x,y,1,len,color); }

void Graphics::draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    int16_t x=0,y=r,d=3-2*r;
    while(x<=y){
        draw_pixel(cx+x,cy+y,color); draw_pixel(cx-x,cy+y,color);
        draw_pixel(cx+x,cy-y,color); draw_pixel(cx-x,cy-y,color);
        draw_pixel(cx+y,cy+x,color); draw_pixel(cx-y,cy+x,color);
        draw_pixel(cx+y,cy-x,color); draw_pixel(cx-y,cy-x,color);
        if(d<0) d+=4*x+6; else {d+=4*(x-y)+10; y--;}
        x++;
    }
}

void Graphics::draw_char(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale) {
    if(c<0x20 || c>0x7E) c=' ';
    uint8_t idx = c - 0x20;
    for(uint8_t col=0; col<5; col++){
        uint8_t line = font5x7[idx][col];
        for(uint8_t row=0; row<7; row++){
            uint16_t color = (line & (1<<row)) ? fg : bg;
            fill_rect(x + col*scale, y + row*scale, scale, scale, color);
        }
    }
}

void Graphics::draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale) {
    while(*s){
        draw_char(x,y,*s,fg,bg,scale);
        x += 6*scale;
        s++;
    }
}

void Graphics::draw_ship_palette(int16_t x, int16_t y, uint16_t body, uint16_t cockpit, uint16_t engine, uint16_t wing, uint16_t detail, uint16_t bg) {
    for(int16_t row=0; row<SHIP_H; row++){
        for(int16_t col=0; col<SHIP_W; col++){
            uint16_t px = ship_sprite[row][col];
            uint16_t color = bg;
            if(px == COLOR565(100,220,255)) color = body;
            else if(px == COLOR565(200,240,255)) color = cockpit;
            else if(px == COLOR565(255,120,0)) color = engine;
            else if(px == COLOR565(0,80,200)) color = wing;
            else if(px == COLOR565(160,200,220)) color = detail;
            else if(px == COLOR565(255,240,0)) color = engine;
            else if(px == COLOR565(255,160,0)) color = engine;
            else if(px == 0x0000) color = bg;
            else color = px;
            draw_pixel(x+col, y+row, color);
        }
    }
}

void Graphics::draw_ship(int16_t x, int16_t y, uint16_t bg_color) {
    draw_ship_palette(x, y, COLOR565(100,220,255), COLOR565(200,240,255), COLOR565(255,120,0), COLOR565(0,80,200), COLOR565(160,200,220), bg_color);
}

void Graphics::erase_ship(int16_t x, int16_t y, uint16_t bg_color) {
    fill_rect(x, y, SHIP_W, SHIP_H, bg_color);
}

// ======================== NAMESPACE DEL JUEGO ========================
namespace Game {
    const uint8_t MAX_ENEMIES = 8;
    const uint8_t MAX_BULLETS = 5;
    const uint8_t INVINCIBLE_FRAMES = 60;

    enum EnemyType { ENEMY_RED, ENEMY_GREEN, ENEMY_BLUE, ENEMY_ORANGE, ENEMY_PURPLE, NUM_TYPES };

    struct Enemy {
        int16_t x, y;
        uint8_t w, h;
        int8_t speed;
        bool active;
        EnemyType type;
    };
    struct Bullet {
        int16_t x, y;
        int8_t speed;
        bool active;
    };

    Enemy enemies[MAX_ENEMIES];
    Bullet bullets[MAX_BULLETS];
    uint8_t lives;
    uint32_t score;
    int16_t shipX, shipY;
    int8_t invincibleCounter;
    bool shipVisible;
    uint8_t lastFireState;

    void init(void);
    void spawnEnemy(void);
    void updateEnemies(void);
    void updateBullets(void);
    void updateShip(void);
    void checkCollisions(void);
    void drawHUD(void);
    void drawExplosion(int16_t x, int16_t y);
    void drawEnemyShip(int16_t x, int16_t y, EnemyType type);
    void gameLoop(void);
}

void Game::drawEnemyShip(int16_t x, int16_t y, EnemyType type) {
    uint16_t body, cockpit, engine, wing, detail;
    switch(type) {
        case ENEMY_RED:    body = COLOR565(200,0,0); cockpit = COLOR565(255,100,100); engine = COLOR565(255,80,0); wing = COLOR565(150,0,0); detail = COLOR565(180,50,50); break;
        case ENEMY_GREEN:  body = COLOR565(0,180,0); cockpit = COLOR565(100,255,100); engine = COLOR565(0,200,0); wing = COLOR565(0,130,0); detail = COLOR565(80,200,80); break;
        case ENEMY_BLUE:   body = COLOR565(0,0,200); cockpit = COLOR565(100,100,255); engine = COLOR565(0,100,200); wing = COLOR565(0,0,150); detail = COLOR565(80,80,200); break;
        case ENEMY_ORANGE: body = COLOR565(255,100,0); cockpit = COLOR565(255,180,100); engine = COLOR565(255,140,0); wing = COLOR565(200,80,0); detail = COLOR565(255,150,80); break;
        default:           body = COLOR565(255,0,255); cockpit = COLOR565(255,150,255); engine = COLOR565(200,0,200); wing = COLOR565(180,0,180); detail = COLOR565(220,100,220); break;
    }
    Graphics::draw_ship_palette(x, y, body, cockpit, engine, wing, detail, BLACK);
}

void Game::init(void) {
    for(int i=0; i<MAX_ENEMIES; i++) enemies[i].active = false;
    for(int i=0; i<MAX_BULLETS; i++) bullets[i].active = false;

    lives = 3;
    score = 0;
    shipX = (TFT_W - SHIP_W) / 2;
    shipY = TFT_H - SHIP_H - 10;
    invincibleCounter = 0;
    shipVisible = true;
    lastFireState = 1;

    Graphics::fill_screen(BLACK);
    drawHUD();

#ifdef PRESENTACION
    Graphics::draw_string(55, 100, "SPACE", CYAN, BLACK, 3);
    Graphics::draw_string(40, 130, "SHOOTER", CYAN, BLACK, 3);
    Graphics::draw_string(30, 165, "PIC32MX795", COLOR565(100,100,255), BLACK, 2);
    delay_ms(2000);
    Graphics::fill_rect(0, 90, TFT_W, 100, BLACK);
#endif

    for(int i=0; i<3; i++) spawnEnemy();
}

void Game::spawnEnemy(void) {
    for(int i=0; i<MAX_ENEMIES; i++) {
        if(!enemies[i].active) {
            enemies[i].w = SHIP_W;
            enemies[i].h = SHIP_H;
            enemies[i].x = rand() % (TFT_W - enemies[i].w);
            enemies[i].y = -enemies[i].h;
            enemies[i].speed = 1 + (rand() % 3);
            enemies[i].type = (EnemyType)(rand() % NUM_TYPES);
            enemies[i].active = true;
            break;
        }
    }
}

void Game::updateEnemies(void) {
    for(int i=0; i<MAX_ENEMIES; i++) {
        if(enemies[i].active) {
            Graphics::erase_ship(enemies[i].x, enemies[i].y, BLACK);
            enemies[i].y += enemies[i].speed;
            if(enemies[i].y > TFT_H) {
                enemies[i].active = false;
                if(lives > 0) {
                    lives--;
                    drawHUD();
                    invincibleCounter = INVINCIBLE_FRAMES;
                }
            } else {
                drawEnemyShip(enemies[i].x, enemies[i].y, enemies[i].type);
            }
        }
    }
    static uint8_t spawnCounter = 0;
    if(++spawnCounter > 25) {
        spawnCounter = 0;
        if(rand() % 3 == 0) spawnEnemy();
    }
}

void Game::updateBullets(void) {
    for(int i=0; i<MAX_BULLETS; i++) {
        if(bullets[i].active) {
            Graphics::draw_pixel(bullets[i].x, bullets[i].y, BLACK);
            bullets[i].y += bullets[i].speed;
            if(bullets[i].y < 0) {
                bullets[i].active = false;
            } else {
                Graphics::draw_pixel(bullets[i].x, bullets[i].y, YELLOW);
            }
        }
    }
}

void Game::updateShip(void) {
    static int16_t prevShipX = -1, prevShipY = -1;
    static uint8_t auto_counter = 0;
    static int8_t auto_dir = 1;

#ifdef USE_BUTTONS
    static uint8_t leftPressed=0, rightPressed=0;
    if(BTN_LEFT) { if(!leftPressed) { shipX -= 8; leftPressed=1; } } else leftPressed=0;
    if(BTN_RIGHT) { if(!rightPressed) { shipX += 8; rightPressed=1; } } else rightPressed=0;
#else
    auto_counter++;
    if(auto_counter >= 4) { shipX += auto_dir * 4; auto_counter = 0; }
    if(shipX <= 5) auto_dir = 1;
    if(shipX >= TFT_W - SHIP_W - 5) auto_dir = -1;
#endif

    if(shipX < 2) shipX = 2;
    if(shipX > TFT_W - SHIP_W - 2) shipX = TFT_W - SHIP_W - 2;

#ifdef USE_BUTTONS
    if(!BTN_FIRE && lastFireState) {
        for(int i=0; i<MAX_BULLETS; i++) {
            if(!bullets[i].active) {
                bullets[i].x = shipX + SHIP_W/2;
                bullets[i].y = shipY - 2;
                bullets[i].speed = -5;
                bullets[i].active = true;
                break;
            }
        }
    }
    lastFireState = BTN_FIRE;
#else
    static uint8_t fireCounter = 0;
    if(++fireCounter >= 20) {
        fireCounter = 0;
        for(int i=0; i<MAX_BULLETS; i++) {
            if(!bullets[i].active) {
                bullets[i].x = shipX + SHIP_W/2;
                bullets[i].y = shipY - 2;
                bullets[i].speed = -5;
                bullets[i].active = true;
                break;
            }
        }
    }
#endif

    if(invincibleCounter > 0) {
        invincibleCounter--;
        shipVisible = (invincibleCounter & 4) ? true : false;
    } else {
        shipVisible = true;
    }

    if(shipX != prevShipX || shipY != prevShipY || (invincibleCounter > 0 && (invincibleCounter & 4) != ((invincibleCounter+1) & 4))) {
        if(prevShipX != -1 && prevShipY != -1) Graphics::erase_ship(prevShipX, prevShipY, BLACK);
        if(shipVisible) Graphics::draw_ship(shipX, shipY, BLACK);
        prevShipX = shipX;
        prevShipY = shipY;
    }
}

void Game::checkCollisions(void) {
    for(int b=0; b<MAX_BULLETS; b++) {
        if(!bullets[b].active) continue;
        for(int e=0; e<MAX_ENEMIES; e++) {
            if(!enemies[e].active) continue;
            if(bullets[b].x >= enemies[e].x && bullets[b].x <= enemies[e].x+enemies[e].w &&
               bullets[b].y >= enemies[e].y && bullets[b].y <= enemies[e].y+enemies[e].h) {
                bullets[b].active = false;
                enemies[e].active = false;
                score += 10;
                drawHUD();
                drawExplosion(enemies[e].x + enemies[e].w/2, enemies[e].y + enemies[e].h/2);
                break;
            }
        }
    }

    if(invincibleCounter == 0) {
        for(int e=0; e<MAX_ENEMIES; e++) {
            if(!enemies[e].active) continue;
            if(shipX < enemies[e].x+enemies[e].w && shipX+SHIP_W > enemies[e].x &&
               shipY < enemies[e].y+enemies[e].h && shipY+SHIP_H > enemies[e].y) {
                lives--;
                drawHUD();
                invincibleCounter = INVINCIBLE_FRAMES;
                enemies[e].active = false;
                drawExplosion(shipX + SHIP_W/2, shipY + SHIP_H/2);
                if(lives == 0) {
                    Graphics::draw_string(TFT_W/2-60, TFT_H/2-10, "GAME OVER", RED, BLACK, 2);
                    while(1);
                }
                break;
            }
        }
    }
}

void Game::drawExplosion(int16_t x, int16_t y) {
    for(int r=1; r<8; r++) {
        Graphics::draw_circle(x, y, r, COLOR565(255,200,0));
        delay_us(50000);
    }
    for(int r=1; r<8; r++) {
        Graphics::draw_circle(x, y, r, BLACK);
    }
}

void Game::drawHUD(void) {
    Graphics::fill_rect(0,0,TFT_W,14,BLACK);
    for(uint8_t i=0; i<lives && i<5; i++) {
        int16_t lx = 4 + i*14;
        Graphics::draw_pixel(lx+3, 2, RED);
        Graphics::fill_rect(lx+1, 4, 6, 2, RED);
        Graphics::fill_rect(lx+2, 6, 4, 2, RED);
        Graphics::fill_rect(lx+3, 8, 2, 1, RED);
    }
    char buf[12];
    std::sprintf(buf, "%lu", score);
    Graphics::draw_string(TFT_W/2-30, 3, buf, WHITE, BLACK, 1);
    Graphics::draw_hline(0,13,TFT_W, GRAY);
}

void Game::gameLoop(void) {
    while(1) {
        updateShip();
        updateBullets();
        updateEnemies();
        checkCollisions();
        delay_ms(16);
    }
}

// ======================== MAIN ========================
int main(void) {
    BL_TRIS=0; DC_TRIS=0; RST_TRIS=0;
    DC_LAT=0; RST_LAT=1;
    BL_LAT=0;

    Graphics::spi4_init();
    Graphics::init_display();
    BL_LAT=1;

    srand(_CP0_GET_COUNT());

    Game::init();
    Game::gameLoop();

    return 0;
}