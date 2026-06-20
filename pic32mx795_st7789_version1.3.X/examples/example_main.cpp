/**
 * @file main.cpp
 * @brief Punto de entrada del juego.
 */

#include "HardwareProfile.h"
#include "Graphics.h"
#include "GameEngine.h"

// Configuración de bits (debe estar aquí para que el linker las tome)
#pragma config FPLLMUL  = MUL_20
#pragma config FPLLIDIV = DIV_2
#pragma config FPLLODIV = DIV_1
#pragma config FWDTEN   = OFF
#pragma config POSCMOD  = XT
#pragma config FNOSC    = PRIPLL
#pragma config FPBDIV   = DIV_1

// ==================== IMPLEMENTACIÓN DE FUNCIONES DE HARDWARE ====================
void delay_cycles(uint32_t c) {
    uint32_t s = _CP0_GET_COUNT();
    while((_CP0_GET_COUNT() - s) < c);
}
void delay_ms(uint32_t ms) { while(ms--) delay_cycles(40000); }
void delay_us(uint32_t us) { delay_cycles(us * 40); }

void spi4_init(void) {
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

void spi4_write(uint8_t d) {
    while(SPI4STATbits.SPITBF);
    SPI4BUF = d;
    while(!SPI4STATbits.SPIRBF);
    (void)SPI4BUF;
}

void write_cmd(uint8_t c)  { DC_LAT=0; spi4_write(c); }
void write_data(uint8_t d) { DC_LAT=1; spi4_write(d); }
void push_color(uint16_t color) { spi4_write(color>>8); spi4_write(color&0xFF); }

void reset_display(void) {
    RST_LAT=1; delay_ms(10);
    RST_LAT=0; delay_ms(20);
    RST_LAT=1; delay_ms(150);
}

void init_display(void) {
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

void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    write_cmd(0x2A);
    write_data(x0>>8); write_data(x0&0xFF);
    write_data(x1>>8); write_data(x1&0xFF);
    write_cmd(0x2B);
    write_data(y0>>8); write_data(y0&0xFF);
    write_data(y1>>8); write_data(y1&0xFF);
    write_cmd(0x2C);
    DC_LAT = 1;
}

// ==================== IMPLEMENTACIÓN DE GRÁFICOS (Sprite y dibujo) ====================
// Sprite de nave (sin invertir)
static const uint16_t ship_sprite[24][16] = {
    // (Aquí debe ir el sprite completo. Por brevedad se omite pero se incluirá en el script final)
    // En el código generado final se incluirá la matriz completa (ver script final).
};


// ==================== MAIN ====================
int main(void) {
    BL_TRIS=0; DC_TRIS=0; RST_TRIS=0;
    DC_LAT=0; RST_LAT=1;
    BL_LAT=0;

    spi4_init();
    init_display();
    BL_LAT=1;

    srand(_CP0_GET_COUNT());

    GameEngine::init();
    GameEngine::gameLoop();

    return 0;
}
