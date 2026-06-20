/*
 * ST7789 Driver + Nave Espacial para PIC32MX795F512H (64 pines)
 * Display: GMT130 V1.0 240x240, sin CS
 * SPI4 - Modo 3
 *
 * Incluye:
 *   - Driver completo ST7789
 *   - Fuente 5x7 para texto
 *   - Sprite de nave (16x24 px, 3 colores)
 *   - Draw pixel, linea, rectangulo, circulo
 *   - Animacion basica de nave moviéndose
 */

#include <xc.h>
#include <stdint.h>
#include <string.h>

// ======================== CONFIGURACION BITS ========================
#pragma config FPLLMUL  = MUL_20
#pragma config FPLLIDIV = DIV_2
#pragma config FPLLODIV = DIV_1
#pragma config FWDTEN   = OFF
#pragma config POSCMOD  = XT
#pragma config FNOSC    = PRIPLL
#pragma config FPBDIV   = DIV_1

// ======================== PINES PANTALLA ========================
#define BL_LAT      LATBbits.LATB0
#define DC_LAT      LATBbits.LATB1
#define RST_LAT     LATBbits.LATB2
#define BL_TRIS     TRISBbits.TRISB0
#define DC_TRIS     TRISBbits.TRISB1
#define RST_TRIS    TRISBbits.TRISB2

// ======================== PARAMETROS PANTALLA ========================
#define TFT_W       240
#define TFT_H       240

// Colores RGB565
#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define ORANGE      0xFC00
#define DARK_BLUE   0x000F
#define GRAY        0x8410
#define DARK_GRAY   0x4208
#define LIGHT_CYAN  0xAFFF
#define PURPLE      0x780F

#define COLOR565(r,g,b) ((uint16_t)((((r)&0xF8)<<8) | (((g)&0xFC)<<3) | ((b)>>3)))

// ======================== RETARDOS (80 MHz) ========================
static inline void delay_cycles(uint32_t c) {
    uint32_t s = _CP0_GET_COUNT();
    while((_CP0_GET_COUNT() - s) < c);
}
void delay_ms(uint32_t ms) { while(ms--) delay_cycles(40000); }
void delay_us(uint32_t us) { delay_cycles(us * 40); }

// ======================== INICIALIZACION SPI4 ========================
void spi4_init(void) {
    DDPCON  = 0x00;
    AD1PCFG = 0xFFFF;

    TRISBCLR = (1<<6);   // RB6 = SCK4 salida
    TRISFCLR = (1<<5);   // RF5 = SDO4 salida
    TRISFSET = (1<<4);   // RF4 = SDI4 entrada

    // Desbloquear PPS
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    OSCCONCLR = (1<<6);

    volatile uint32_t *RPOR3  = (volatile uint32_t*)0xBF80B33C;
    volatile uint32_t *RPOR2  = (volatile uint32_t*)0xBF80B338;
    volatile uint32_t *RPINR20= (volatile uint32_t*)0xBF80B3A0;

    *RPOR3  = (*RPOR3  & 0xFFFFFF00) | 3;       // SCK4 -> RB6(RP6)
    *RPOR2  = (*RPOR2  & 0x0000FFFF) | (4<<16); // SDO4 -> RF5(RP5)  <-- corregido shift
    *RPINR20= (*RPINR20& 0xFFFFFF00) | 4;        // SDI4 <- RF4(RP4)

    SYSKEY = 0x00000000;

    SPI4CON = 0;
    SPI4BRG  = 0;          // Maximo: PBCLK/2 = 40 MHz
    SPI4CONbits.MSTEN = 1;
    SPI4CONbits.MODE16= 0; // 8 bits
    SPI4CONbits.CKP   = 1; // CPOL=1
    SPI4CONbits.CKE   = 0; // CPHA=1  => Modo 3
    SPI4CONbits.SMP   = 0;
    SPI4STATbits.SPIROV = 0;
    SPI4CONbits.ON = 1;
}

// ======================== SPI LOW LEVEL ========================
static inline void spi4_write(uint8_t d) {
    while(SPI4STATbits.SPITBF);
    SPI4BUF = d;
    while(!SPI4STATbits.SPIRBF);
    (void)SPI4BUF;
}
static inline void write_cmd(uint8_t c)  { DC_LAT=0; spi4_write(c); }
static inline void write_data(uint8_t d) { DC_LAT=1; spi4_write(d); }

// Envio rapido de color (DC ya en 1)
static inline void push_color(uint16_t color) {
    spi4_write(color >> 8);
    spi4_write(color & 0xFF);
}

// ======================== INICIALIZACION ST7789 ========================
void reset_display(void) {
    RST_LAT=1; delay_ms(10);
    RST_LAT=0; delay_ms(20);
    RST_LAT=1; delay_ms(150);
}

void init_display(void) {
    reset_display();

    write_cmd(0x11); delay_ms(120);    // Sleep Out
    write_cmd(0x36); write_data(0x00); // MADCTL: orientacion normal
    write_cmd(0x3A); write_data(0x05); // COLMOD: 16bpp RGB565
    write_cmd(0x21);                   // Display Inversion ON (necesario en GMT130)
    write_cmd(0x13);                   // Normal Display Mode ON

    // Porch control
    write_cmd(0xB2);
    write_data(0x0C); write_data(0x0C); write_data(0x00);
    write_data(0x33); write_data(0x33);

    write_cmd(0xB7); write_data(0x35); // Gate control
    write_cmd(0xBB); write_data(0x37); // VCOM Setting
    write_cmd(0xC0); write_data(0x2C); // LCM Control
    write_cmd(0xC2); write_data(0x01); // VDV and VRH Command Enable
    write_cmd(0xC3); write_data(0x12); // VRH Set
    write_cmd(0xC4); write_data(0x20); // VDV Set
    write_cmd(0xC6); write_data(0x0F); // Frame Rate Control: 60Hz

    write_cmd(0xD0); write_data(0xA4); write_data(0xA1); // Power Control 1

    // Gamma positiva
    write_cmd(0xE0);
    const uint8_t gp[]={0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,
                         0x4C,0x18,0x0D,0x0B,0x1F,0x23};
    for(int i=0;i<14;i++) write_data(gp[i]);

    // Gamma negativa
    write_cmd(0xE1);
    const uint8_t gn[]={0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,
                         0x51,0x2F,0x1F,0x1F,0x20,0x23};
    for(int i=0;i<14;i++) write_data(gn[i]);

    write_cmd(0x29); delay_ms(120);    // Display ON
}

// ======================== VENTANA Y DIBUJO BASE ========================
void set_window(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1) {
    write_cmd(0x2A);
    write_data(x0>>8); write_data(x0&0xFF);
    write_data(x1>>8); write_data(x1&0xFF);
    write_cmd(0x2B);
    write_data(y0>>8); write_data(y0&0xFF);
    write_data(y1>>8); write_data(y1&0xFF);
    write_cmd(0x2C);
    DC_LAT = 1;
}

void fill_screen(uint16_t color) {
    set_window(0,0,TFT_W-1,TFT_H-1);
    uint32_t n = (uint32_t)TFT_W * TFT_H;
    uint8_t hi = color>>8, lo = color&0xFF;
    for(uint32_t i=0;i<n;i++){ spi4_write(hi); spi4_write(lo); }
}

void draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if((uint16_t)x >= TFT_W || (uint16_t)y >= TFT_H) return;
    set_window(x,y,x,y);
    push_color(color);
}

void fill_rect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t color) {
    if(x>=TFT_W || y>=TFT_H || w<=0 || h<=0) return;
    if(x<0){w+=x; x=0;}
    if(y<0){h+=y; y=0;}
    if(x+w>TFT_W) w=TFT_W-x;
    if(y+h>TFT_H) h=TFT_H-y;
    set_window(x,y,x+w-1,y+h-1);
    uint32_t n=(uint32_t)w*h;
    uint8_t hi=color>>8, lo=color&0xFF;
    for(uint32_t i=0;i<n;i++){spi4_write(hi);spi4_write(lo);}
}

void draw_rect_outline(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t color){
    fill_rect(x,y,w,1,color);
    fill_rect(x,y+h-1,w,1,color);
    fill_rect(x,y,1,h,color);
    fill_rect(x+w-1,y,1,h,color);
}

// Linea horizontal rapida
void draw_hline(int16_t x,int16_t y,int16_t len,uint16_t color){
    fill_rect(x,y,len,1,color);
}
// Linea vertical rapida
void draw_vline(int16_t x,int16_t y,int16_t len,uint16_t color){
    fill_rect(x,y,1,len,color);
}

// Bresenham
void draw_line(int16_t x0,int16_t y0,int16_t x1,int16_t y1,uint16_t color){
    int16_t dx=abs(x1-x0), dy=-abs(y1-y0);
    int16_t sx=(x0<x1)?1:-1, sy=(y0<y1)?1:-1;
    int16_t err=dx+dy, e2;
    while(1){
        draw_pixel(x0,y0,color);
        if(x0==x1&&y0==y1) break;
        e2=2*err;
        if(e2>=dy){err+=dy; x0+=sx;}
        if(e2<=dx){err+=dx; y0+=sy;}
    }
}

void draw_circle(int16_t cx,int16_t cy,int16_t r,uint16_t color){
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

void fill_circle(int16_t cx,int16_t cy,int16_t r,uint16_t color){
    for(int16_t dy=-r;dy<=r;dy++){
//        int16_t dx=(int16_t)sqrt_int(r*r-dy*dy); // usa sqrt_int abajo
//        fill_rect(cx-dx,cy+dy,dx*2+1,1,color);
    }
}

// Raiz entera simple (sin math.h)
static uint16_t sqrt_int(uint32_t n){
    uint32_t x=n, y=1;
    while(x>y){x=(x+y)/2; y=n/x;}
    return (uint16_t)x;
}

// ======================== FUENTE 5x7 ========================
// Caracteres ASCII 0x20-0x7E, 5 columnas x 7 filas
static const uint8_t font5x7[][5] = {
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
    {0x44,0x64,0x54,0x4C,0x44}, // 'z'
};

void draw_char(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale) {
    if(c<0x20 || c>0x7A) c=' ';
    uint8_t idx = c - 0x20;
    for(uint8_t col=0; col<5; col++){
        uint8_t line = font5x7[idx][col];
        for(uint8_t row=0; row<7; row++){
            uint16_t color = (line & (1<<row)) ? fg : bg;
            fill_rect(x + col*scale, y + row*scale, scale, scale, color);
        }
    }
}

void draw_string(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale) {
    while(*s){
        draw_char(x,y,*s,fg,bg,scale);
        x += 6*scale;
        s++;
    }
}

// ======================== SPRITE NAVE ESPACIAL 16x24 ========================
/*
 * Nave mirando hacia ARRIBA (Y decreciente = frente)
 * Colores:
 *   T = transparente (fondo)
 *   B = cuerpo (cian claro)
 *   C = cabina (cian brillante)
 *   E = motor/propulsor (naranja)
 *   W = alas (azul)
 *   G = borde/detalle gris
 *   F = llama motor (amarillo)
 *   D = llama oscura (naranja oscuro)
 *
 * Grid 16 columnas x 24 filas
 */
#define T 0x0000  // transparente - placeholder, se reemplaza con bg_color
#define B COLOR565(100,220,255)  // cuerpo cian
#define C COLOR565(200,240,255)  // cabina brillante
#define E COLOR565(255,120,0)    // motor naranja
#define W COLOR565(0,80,200)     // alas azul
#define G COLOR565(160,200,220)  // detalles grises
#define F COLOR565(255,240,0)    // llama amarilla
#define D COLOR565(255,160,0)    // llama oscura naranja

static const uint16_t ship_sprite[24][16] = {
    // fila 0: punta nave
    {T,T,T,T,T,T,T,B,B,T,T,T,T,T,T,T},
    // fila 1
    {T,T,T,T,T,T,B,C,C,B,T,T,T,T,T,T},
    // fila 2
    {T,T,T,T,T,B,C,C,C,C,B,T,T,T,T,T},
    // fila 3
    {T,T,T,T,B,C,C,C,C,C,C,B,T,T,T,T},
    // fila 4
    {T,T,T,B,C,C,G,C,C,G,C,C,B,T,T,T},
    // fila 5: cuerpo principal comienza
    {T,T,W,B,B,B,B,B,B,B,B,B,B,W,T,T},
    // fila 6
    {T,W,W,B,C,C,C,C,C,C,C,C,B,W,W,T},
    // fila 7
    {W,W,W,B,C,C,C,C,C,C,C,C,B,W,W,W},
    // fila 8: alas anchas
    {W,W,W,B,C,G,C,C,C,C,G,C,B,W,W,W},
    // fila 9
    {W,W,B,B,B,B,B,B,B,B,B,B,B,B,W,W},
    // fila 10: mitad nave
    {T,W,B,B,G,B,E,E,E,E,B,G,B,B,W,T},
    // fila 11
    {T,W,B,B,B,B,E,E,E,E,B,B,B,B,W,T},
    // fila 12
    {T,T,W,B,B,B,B,B,B,B,B,B,B,W,T,T},
    // fila 13: parte trasera, motores visibles
    {T,T,W,B,E,E,B,B,B,B,E,E,B,W,T,T},
    // fila 14
    {T,T,W,B,E,E,B,B,B,B,E,E,B,W,T,T},
    // fila 15: motores principales
    {T,T,T,B,E,E,B,E,E,B,E,E,B,T,T,T},
    // fila 16
    {T,T,T,B,E,E,B,E,E,B,E,E,B,T,T,T},
    // fila 17: inicio llamas
    {T,T,T,T,F,F,T,F,F,T,F,F,T,T,T,T},
    // fila 18
    {T,T,T,T,F,D,T,F,D,T,F,D,T,T,T,T},
    // fila 19
    {T,T,T,T,D,F,T,D,F,T,D,F,T,T,T,T},
    // fila 20
    {T,T,T,T,T,F,T,T,F,T,T,F,T,T,T,T},
    // fila 21: llamas largas
    {T,T,T,T,T,D,T,T,D,T,T,D,T,T,T,T},
    // fila 22
    {T,T,T,T,T,F,T,T,F,T,T,F,T,T,T,T},
    // fila 23
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

/*
 * Dibuja la nave en (x,y). El pixel negro (0x0000) en el sprite
 * se usa como "transparente" y se reemplaza por bg_color.
 *
 * Para evitar parpadeo se dibuja pixel a pixel solo donde cambia,
 * o simplemente se borra el fondo antes de dibujar.
 */
void draw_ship(int16_t x, int16_t y, uint16_t bg_color) {
    for(int16_t row=0; row<SHIP_H; row++){
        for(int16_t col=0; col<SHIP_W; col++){
            uint16_t px = ship_sprite[row][col];
            if(px == 0x0000) px = bg_color;   // transparente
            draw_pixel(x+col, y+row, px);
        }
    }
}

/*
 * Borra la nave (dibuja bg sobre su area)
 */
void erase_ship(int16_t x, int16_t y, uint16_t bg_color) {
    fill_rect(x, y, SHIP_W, SHIP_H, bg_color);
}

// ======================== ESTRELLAS DE FONDO ========================
#define MAX_STARS 60

typedef struct { int16_t x,y; uint8_t speed; uint16_t color; } Star;
static Star stars[MAX_STARS];

// LCG sencillo para pseudoaleatoriedad sin rand()
static uint32_t lcg_seed = 12345;
static uint16_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1664525u + 1013904223u;
    return (uint16_t)(lcg_seed >> 16);
}

void init_stars(void) {
    for(int i=0;i<MAX_STARS;i++){
        stars[i].x     = lcg_rand() % TFT_W;
        stars[i].y     = lcg_rand() % TFT_H;
        stars[i].speed = (lcg_rand() % 3) + 1;
        uint8_t bright = 80 + (lcg_rand() % 176);
        stars[i].color = COLOR565(bright,bright,bright);
    }
}

void update_stars(uint16_t bg) {
    for(int i=0;i<MAX_STARS;i++){
        // Borrar posicion anterior
        draw_pixel(stars[i].x, stars[i].y, bg);
        // Mover hacia abajo (nave "avanza" hacia arriba)
        stars[i].y += stars[i].speed;
        if(stars[i].y >= TFT_H){
            stars[i].y = 0;
            stars[i].x = lcg_rand() % TFT_W;
        }
        // Dibujar nueva posicion
        draw_pixel(stars[i].x, stars[i].y, stars[i].color);
    }
}

// ======================== BARRA DE VIDA / HUD ========================
void draw_hud(uint8_t lives, uint32_t score, uint16_t bg) {
    // Fondo HUD (franja superior)
    fill_rect(0,0,TFT_W,14,bg);

    // Vidas: pequeños triangulos (nave miniatura simplificada)
    for(uint8_t i=0;i<lives && i<5;i++){
        int16_t lx = 4 + i*14;
        draw_pixel(lx+3,  2, COLOR565(100,220,255));
        fill_rect (lx+1,  4, 6, 2, COLOR565(0,80,200));
        fill_rect (lx+2,  6, 4, 2, COLOR565(100,220,255));
        fill_rect (lx+3,  8, 2, 1, COLOR565(255,120,0));
    }

    // Puntaje
    char buf[12];
    uint32_t s = score;
    int8_t pos = 10;
    buf[11]='\0';
    if(s==0){ buf[10]='0'; pos=10; }
    else {
        while(s>0 && pos>=0){ buf[pos--]='0'+(s%10); s/=10; }
        pos++;
    }
    draw_string(TFT_W/2-30, 3, buf+pos, WHITE, bg, 1);

    // Separador
    draw_hline(0,13,TFT_W, COLOR565(50,50,100));
}

// ======================== FONDO ESPACIAL INICIAL ========================
void draw_space_background(void) {
    // Fondo negro degradado muy sutil: solo llenamos de negro
    fill_screen(BLACK);

    // Nebulosa de fondo: manchas de color muy oscuro
    for(uint8_t i=0;i<8;i++){
        int16_t nx = lcg_rand() % (TFT_W-40);
        int16_t ny = 20 + lcg_rand() % (TFT_H-40);
        uint16_t nc;
        uint8_t t = lcg_rand()%3;
        if(t==0) nc = COLOR565(10,0,25);
        else if(t==1) nc = COLOR565(0,10,20);
        else nc = COLOR565(15,5,0);
        for(int16_t dy=-15;dy<=15;dy++)
            for(int16_t dx=-20;dx<=20;dx++)
                if(dx*dx/400+dy*dy/225 < 1)
                    draw_pixel(nx+20+dx,ny+dy, nc);
    }

    // Estrella brillante (tipo Sol lejano)
    int16_t sx = 30 + lcg_rand()%(TFT_W-60);
    int16_t sy_star = 30 + lcg_rand()%50;
    fill_rect(sx-1,sy_star-4,3,9,COLOR565(80,80,80));
    fill_rect(sx-4,sy_star-1,9,3,COLOR565(80,80,80));
    draw_pixel(sx,sy_star,WHITE);
}

// ======================== DEMO / ANIMACION PRINCIPAL ========================
void demo_animation(void) {
    fill_screen(BLACK);
    init_stars();
    draw_space_background();
    init_stars(); // re-inicializar sobre fondo

    // Posicion inicial de la nave: centrada abajo
    int16_t ship_x = (TFT_W - SHIP_W) / 2;
    int16_t ship_y = TFT_H - SHIP_H - 20;

    // Dibujar nave inicial
    draw_ship(ship_x, ship_y, BLACK);

    uint32_t score = 0;
    uint8_t  lives = 3;
    draw_hud(lives, score, BLACK);

    // Pantalla titulo por 2 segundos
    draw_string(55, 100, "SPACE", CYAN, BLACK, 3);
    draw_string(40, 130, "SHOOTER", CYAN, BLACK, 3);
    draw_string(30, 165, "PIC32MX795", COLOR565(100,100,255), BLACK, 2);
    delay_ms(2000);

    // Borrar texto
    fill_rect(0, 90, TFT_W, 100, BLACK);
    // Re-dibujar nave
    draw_ship(ship_x, ship_y, BLACK);

    // ---- Bucle de animacion ----
    // Trayectoria: la nave oscila de izquierda a derecha
    int8_t  dir_x = 1;
    uint8_t frame  = 0;
    uint32_t tick  = 0;

    while(1) {
        // 1. Actualizar estrellas
        update_stars(BLACK);

        // 2. Borrar nave vieja
        erase_ship(ship_x, ship_y, BLACK);

        // 3. Mover nave
        ship_x += dir_x;
        if(ship_x <= 5)               { ship_x=5;          dir_x= 1; }
        if(ship_x >= TFT_W-SHIP_W-5) { ship_x=TFT_W-SHIP_W-5; dir_x=-1; }

        // 4. Dibujar nave nueva
        draw_ship(ship_x, ship_y, BLACK);

        // 5. HUD periodicamente
        if((tick % 20) == 0){
            score += 10;
            draw_hud(lives, score, BLACK);
        }

        // 6. Simular disparo: cada 80 frames, un proyectil sube
        if((tick % 80) == 0){
            int16_t bx = ship_x + SHIP_W/2;
            for(int16_t by = ship_y-1; by > 15; by -= 3){
                draw_pixel(bx, by,   YELLOW);
                draw_pixel(bx, by+1, ORANGE);
                delay_us(800);
                draw_pixel(bx, by,   BLACK);
                draw_pixel(bx, by+1, BLACK);
                // Re-trazar estrellas borradas
                for(int i=0;i<MAX_STARS;i++)
                    if(stars[i].x==bx &&
                       (stars[i].y==by || stars[i].y==by+1))
                        draw_pixel(stars[i].x, stars[i].y, stars[i].color);
            }
            // Explosion en el "enemigo"
            for(uint8_t r=1; r<8; r++){
                draw_circle(bx, 16, r, COLOR565(255,200,0));
                delay_us(300);
            }
            for(uint8_t r=1; r<8; r++){
                draw_circle(bx, 16, r, BLACK);
            }
            score += 100;
        }

        frame++;
        tick++;
        delay_ms(16);  // ~60 fps target
    }
}

// ======================== MAIN ========================
int main(void) {
    // Configurar pines pantalla
    BL_TRIS=0; DC_TRIS=0; RST_TRIS=0;
    DC_LAT=0; RST_LAT=1;
    BL_LAT=0;  // backlight OFF hasta init

    // Inicializar SPI y pantalla
    spi4_init();
    init_display();

    // Encender backlight
    BL_LAT=1;

    // Demo / animacion
    demo_animation();

    return 0;
}