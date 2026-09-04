#include <xc.h>
#include "spi.h"
#include "mrf24j40.h"
#include "config.h"

void SPI_Init(void) {
    TRISCbits.TRISC3 = 0;   // SCK salida
    TRISCbits.TRISC4 = 1;   // MISO entrada (importante)
    TRISCbits.TRISC5 = 0;   // MOSI salida
    SSPSTAT = 0x00;          // SMP=0, CKE=0
    SSPCON1 = 0x20;          // SSPEN=1, CKP=0, Fosc/64 = 62.5 kHz
    SSPCON2 = 0x00;
}

uint8_t SPI_Transfer(uint8_t data) {
    SSPBUF = data;
    while (!SSPSTATbits.BF);
    return SSPBUF;
}

// Función de prueba: lee un registro short
uint8_t SPI_ReadShortDebug(uint8_t addr) {
    uint8_t cmd = (addr & 0x3F) << 1;
    uint8_t result;
    MRF_CS_L();
    __delay_us(1);
    SPI_Transfer(cmd);
    result = SPI_Transfer(0x00);
    __delay_us(1);
    MRF_CS_H();
    return result;
}
