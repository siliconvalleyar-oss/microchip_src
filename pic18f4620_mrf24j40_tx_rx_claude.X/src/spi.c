/*
 * spi.c  - SPI hardware MSSP PIC18F4620
 *
 * Registros MSSP SPI maestro:
 *   SSPCON1: SSPEN=1, CKP=0, SSPM=0000 (Fosc/4)
 *   SSPSTAT: CKE=1 (dato valido en flanco activo), SMP=0
 *
 * MRF24J40 usa SPI Modo 0: CPOL=0, CPHA=0
 *   => CKP=0, CKE=1  en PIC18
 */
#include "spi.h"

/* ---- Inicializar SPI maestro ---------------------------- */
void SPI_Init(void)
{
    /* Pines de direccion */
    TRISCbits.TRISC3 = 0;   /* SCK  = salida */
    TRISCbits.TRISC4 = 1;   /* SDI  = entrada (MISO) */
    TRISCbits.TRISC5 = 0;   /* SDO  = salida  (MOSI) */
    MRF_CS_TRIS      = 0;   /* CS   = salida */
    MRF_CS_LAT       = 1;   /* CS desactivado al inicio */

    /* SSPSTAT: CKE=1, SMP=0 */
    SSPSTAT = 0x40;         /* b01000000 */

    /*
     * SSPCON1:
     *   SSPEN = 1  (habilitar MSSP)
     *   CKP   = 0  (clock idle LOW)
     *   SSPM  = 0000 (SPI maestro, Fosc/4 = 1 MHz)
     */
    SSPCON1 = 0x20;         /* b00100000 */

    /* Limpiar flag de interrupcion SPI */
    PIR1bits.SSPIF = 0;
}

/* ---- Transferir un byte --------------------------------- */
uint8_t SPI_Transfer(uint8_t data)
{
    PIR1bits.SSPIF = 0;
    SSPBUF = data;                  /* Iniciar transferencia */
    while (!PIR1bits.SSPIF);        /* Esperar completar */
    PIR1bits.SSPIF = 0;
    return SSPBUF;                  /* Leer dato recibido */
}
