#include "hal/hal_config.h"
#include "hal/spi_sd.h"

void SPI_SD_Init(void)
{
    TRISBbits.TRISB0 = 1;
    TRISBbits.TRISB1 = 0;
    TRISBbits.TRISB3 = 0;
    TRISCbits.TRISC7 = 0;

    LATBbits.LATB1 = 0;
    LATBbits.LATB3 = 1;
    LATCbits.LATC7 = 0;

    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 0;

    volatile unsigned char *rpor1 = (volatile unsigned char *)0xEF1;
    *rpor1 = 0x0A;

    volatile unsigned char *rpor9 = (volatile unsigned char *)0xEF9;
    *rpor9 = 0x09;

    volatile unsigned char *rpinr21 = (volatile unsigned char *)0xEFD;
    *rpinr21 = 0x03;

    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 1;

    SSP2CON1 = 0b00100010;
    SSP2CON2 = 0;
    SSP2STAT = 0;
    SSP2CON1bits.SSPEN = 1;

    __delay_ms(10);
}
