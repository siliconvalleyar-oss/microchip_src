/*
 * spi.h  - SPI hardware MSSP PIC18F4620
 *
 * Pines:
 *   RC3 = SCK  (salida, maestro)
 *   RC4 = SDI  (entrada, MISO)
 *   RC5 = SDO  (salida,  MOSI)
 *   RC0 = CS   (salida, activo bajo, manual)
 *
 * Modo SPI: Modo 0 (CPOL=0, CPHA=0) - compatible MRF24J40
 * Velocidad: Fosc/4 = 1 MHz @ 4 MHz
 */
#ifndef SPI_H
#define SPI_H

#include "config.h"

/* Inicializar SPI como maestro */
void SPI_Init(void);

/* Transferir un byte (TX y RX simultaneo) */
uint8_t SPI_Transfer(uint8_t data);

/* CS bajo (seleccionar MRF24J40) */
#define SPI_CS_LOW()    do { MRF_CS_LAT = 0; } while(0)

/* CS alto (deseleccionar) */
#define SPI_CS_HIGH()   do { MRF_CS_LAT = 1; } while(0)

#endif /* SPI_H */
