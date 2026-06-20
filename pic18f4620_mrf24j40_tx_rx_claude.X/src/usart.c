/*
 * usart.c  - USART hardware PIC18F4620
 *
 * Pines: TX = RC6 (salida automatica por TXEN)
 *        RX = RC7 (entrada automatica por SPEN)
 *
 * Configuracion:
 *   TXSTA: BRGH=1, TXEN=1, SYNC=0, TX9=0
 *   RCSTA: SPEN=1, RX9=0,  CREN=1
 *   SPBRG: 25  (9600 baud @ 4MHz con BRGH=1)
 *   BRG16: 0
 */
#include "usart.h"
#include <stdio.h>

/* ---- Inicializar USART ---------------------------------- */
void USART_Init(void)
{
    /* RC6 = TX: salida (se controla por modulo USART) */
    TRISCbits.TRISC6 = 1;   /* XC8: poner como input primero,
                               el modulo USART toma control */
    TRISCbits.TRISC7 = 1;   /* RC7 = RX: entrada */

    /* SPBRG: valor para 9600 baud @ 4MHz, BRGH=1 */
    SPBRG  = SPBRG_VAL;     /* 25 */
    SPBRGH = 0;

    /* TXSTA: TXEN=1, BRGH=1, SYNC=0, TX9=0 */
    TXSTA  = 0x24;          /* b00100100 */

    /* RCSTA: SPEN=1, RX9=0, CREN=1 */
    RCSTA  = 0x90;          /* b10010000 */

    /* BAUDCON: BRG16=0 */
    BAUDCONbits.BRG16 = 0;

    /* Limpiar flags */
    (void)RCREG;
    (void)RCREG;
    PIR1bits.RCIF = 0;
    PIR1bits.TXIF = 0;
}

/* ---- Transmitir un byte --------------------------------- */
void USART_PutChar(uint8_t c)
{
    /* Esperar que el buffer de transmision este vacio */
    while (!TXSTAbits.TRMT);
    TXREG = c;
}

/* ---- Transmitir string ---------------------------------- */
void USART_PutString(const char *str)
{
    while (*str) {
        USART_PutChar((uint8_t)*str);
        str++;
    }
}

/* ---- Transmitir entero decimal -------------------------- */
void USART_PutUInt(uint16_t val)
{
    char buf[6];
    uint8_t i = 0, j;
    char tmp;

    if (val == 0) {
        USART_PutChar('0');
        return;
    }
    while (val > 0) {
        buf[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    /* Invertir */
    for (j = 0; j < i / 2; j++) {
        tmp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = tmp;
    }
    buf[i] = '\0';
    USART_PutString(buf);
}

/* ---- Transmitir byte en hex ----------------------------- */
void USART_PutHex(uint8_t val)
{
    const char hex[] = "0123456789ABCDEF";
    USART_PutChar('0');
    USART_PutChar('x');
    USART_PutChar(hex[val >> 4]);
    USART_PutChar(hex[val & 0x0F]);
}

/* ---- Recibir un byte con timeout (timeout en ms) -------- */
uint8_t USART_GetChar(uint8_t *c, uint16_t timeout_ms)
{
    uint16_t t;
    for (t = 0; t < timeout_ms; t++) {
        if (PIR1bits.RCIF) {
            /* Verificar errores de framing/overrun */
            if (RCSTAbits.FERR || RCSTAbits.OERR) {
                RCSTAbits.CREN = 0;     /* Reset receptor */
                (void)RCREG;
                RCSTAbits.CREN = 1;
                continue;
            }
            *c = RCREG;
            return 1;
        }
        __delay_ms(1);
    }
    return 0;
}

/* ---- Verificar dato disponible -------------------------- */
uint8_t USART_DataReady(void)
{
    return PIR1bits.RCIF ? 1 : 0;
}

/* ---- Limpiar buffer RX ---------------------------------- */
void USART_FlushRx(void)
{
    (void)RCREG;
    (void)RCREG;
    PIR1bits.RCIF = 0;
}
