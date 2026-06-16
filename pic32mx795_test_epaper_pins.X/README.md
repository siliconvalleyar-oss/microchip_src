//solo prueba de RB13

#include <xc.h>

// Bits de configuración
#pragma config FNOSC = PRIPLL, POSCMOD = HS, FPLLIDIV = DIV_2, FPLLMUL = MUL_20
#pragma config FPLLODIV = DIV_1, FPBDIV = DIV_1, FWDTEN = OFF
#pragma config ICESEL = ICS_PGx1, CP = OFF

#define SYS_FREQ 80000000UL

void delayMs(uint32_t ms) {
    uint32_t start = __builtin_mfc0(9, 0);
    uint32_t ticks = (SYS_FREQ / 2 / 1000) * ms;
    while ((__builtin_mfc0(9, 0) - start) < ticks);
}

int main(void) {
    // Deshabilitar JTAG para liberar los pines RB13 (TDI)
    DDPCON = 0x00;  // Este es el método correcto y más directo

    // Configurar todos los pines como digitales (deshabilitar ADC)
    AD1PCFG = 0xFFFF;

    // Configurar RB13 como salida
    TRISBCLR = (1 << 13);

    while (1) {
        LATBSET = (1 << 13); // RB13 = 1
        delayMs(5);
        LATBCLR = (1 << 13); // RB13 = 0
        delayMs(5);
    }
    return 0;
}
