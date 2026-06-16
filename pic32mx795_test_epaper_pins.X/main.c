/**
 * @file    main.c
 * @brief   Prueba SPI4 Master para PIC32MX795F512H (64 pines)
 * @version 8.0 (Configuración Corregida y Verificada)
 * 
 * @details Genera una señal de reloj (SCK4) y datos (SDO4) para verificar con osciloscopio.
 *          Pines utilizados:
 *          - SCK4  -> RB14 (Pin lógico 30, Pin físico 29 del TQFP-64)
 *          - SDO4  -> RF5  (Pin lógico 85, Pin físico 32 del TQFP-64)
 *          - SDI4  -> RF4  (Pin lógico 84, Pin físico 31 del TQFP-64)
 */

#include <xc.h>
#include <stdint.h>

// ============================================================================
// CONFIGURACIÓN DEL SISTEMA (Cristal HS de 8 MHz)
// ============================================================================
#pragma config FNOSC = PRIPLL, POSCMOD = HS, FPLLIDIV = DIV_2, FPLLMUL = MUL_20
#pragma config FPLLODIV = DIV_1, FPBDIV = DIV_1, FWDTEN = OFF
#pragma config ICESEL = ICS_PGx1, CP = OFF

#define SYS_FREQ 80000000UL

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================
static inline uint32_t ReadCoreTimer(void) {
    uint32_t count;
    __asm__ volatile("mfc0 %0, $9" : "=r"(count));
    return count;
}

void delayMs(uint32_t ms) {
    uint32_t start = ReadCoreTimer();
    uint32_t ticks = (SYS_FREQ / 2 / 1000) * ms;
    while ((ReadCoreTimer() - start) < ticks);
}

// ============================================================================
// CONFIGURACIÓN SPI4
// ============================================================================
void spi4_init(void) {
    // --- 1. Liberar Pines JTAG (RB13, RB14, etc.) ---
    // El bit JTAGEN en DDPCON está activado por defecto.
    // Al desactivarlo, los pines TDI (RB13), TDO, TMS y TCK (RB14) se liberan
    // y pueden ser usados como GPIO o por otros periféricos.
    DDPCON = 0x00;   // JTAGEN = 0

    // --- 2. Deshabilitar el ADC en los pines que se van a usar ---
    AD1PCFG = 0xFFFF; // Configura todos los pines como digitales

    // --- 3. Configurar la dirección de los pines (TRISx) ---
    // RB14 (SCK4) como salida
    TRISBCLR = (1 << 14);
    // RF5 (SDO4) como salida
    TRISFCLR = (1 << 5);
    // RF4 (SDI4) como entrada
    TRISFSET = (1 << 4);

    // --- 4. Configurar el Mapeo de Pines (PPS) ---
    // Desbloquear la configuración PPS
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    OSCCONCLR = (1 << 6); // Limpiar IOLOCK

    // Mapeo de SALIDAS usando punteros a direcciones absolutas
    // RPOR7 (0xBF80B34C) controla los pines RP14 y RP15
    volatile uint32_t *RPOR7 = (volatile uint32_t*)0xBF80B34C;
    // RPOR2 (0xBF80B338) controla los pines RP4 y RP5
    volatile uint32_t *RPOR2 = (volatile uint32_t*)0xBF80B338;

    // SCK4 a RB14 (RP14). RP14 está en RPOR7, bits 8-15. Valor de función = 3.
    *RPOR7 = (*RPOR7 & 0x00FF) | (3 << 8);
    // SDO4 a RF5 (RP5). RP5 está en RPOR2, bits 8-15. Valor de función = 4.
    *RPOR2 = (*RPOR2 & 0x00FF) | (4 << 8);

    // Mapeo de ENTRADA usando puntero a dirección absoluta
    // RPINR20 (0xBF80B3A0) controla la entrada SDI4
    volatile uint32_t *RPINR20 = (volatile uint32_t*)0xBF80B3A0;
    // SDI4 desde RF4 (RP4). Valor de entrada = 4.
    *RPINR20 = (*RPINR20 & 0xFFFFFF00) | 4;

    // Bloquear la configuración PPS
    SYSKEY = 0x00000000;

    // --- 5. Configurar el Periférico SPI4 como Master ---
    SPI4CON = 0;                // Reset del módulo SPI4
    SPI4BRG = 399;               // 80 MHz / (2 * (4+1)) = 8 MHz
    SPI4CONbits.MSTEN = 1;     // Habilitar modo Master
    SPI4CONbits.MODE16 = 0;    // Modo de 8 bits
    SPI4CONbits.CKP = 0;       // Clock Idle Low
    SPI4CONbits.CKE = 0;       // Datos cambian en flanco de subida
    SPI4CONbits.SMP = 0;       // Muestrear datos en el medio
    SPI4STATbits.SPIROV = 0;   // Limpiar bandera de overflow
    SPI4CONbits.ON = 1;        // Encender el módulo SPI4
}

// ============================================================================
// ENVÍO DE DATOS POR SPI4
// ============================================================================
void spi4_send(uint8_t data) {
    while (!SPI4STATbits.SPITBE); // Esperar a que el buffer de TX esté vacío
    SPI4BUF = data;               // Cargar el dato a enviar
    while (!SPI4STATbits.SPIRBF); // Esperar a que se reciba un dato
    volatile uint8_t dummy = SPI4BUF; // Leer para limpiar la bandera de RX
    (void)dummy;
}

// ============================================================================
// PROGRAMA PRINCIPAL
// ============================================================================
int main(void) {
    // Inicializar SPI4
    spi4_init();

    // Bucle infinito: enviar el byte 0xAA continuamente
    // Esto generará una señal continua en SCK4 y SDO4
    while (1) {
        spi4_send(0xAA);
        // Una pequeña pausa entre envíos ayuda a visualizar las ráfagas
        // delayMs(1);
    }
    return 0;
}