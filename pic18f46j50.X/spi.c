
#include <xc.h>
#include <stdint.h>
#include "spi.h"

void SPI_Init(void)
{
    TRISC &= ~(1 << 3);   // SCK salida
    TRISC &= ~(1 << 5);   // MOSI salida
    TRISC |=  (1 << 4);   // MISO entrada

    TRISB &= ~(1 << 2);   // CS salida
    LATB  |=  (1 << 2);   // CS alto

    SSPSTAT = 0x40;
    SSPCON1 = 0x21;
}

uint8_t SPI_Write(uint8_t data)
{
    SSPBUF = data;

    while(!PIR1bits.SSPIF);

    PIR1bits.SSPIF = 0;

    return SSPBUF;
}