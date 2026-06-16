#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void SPI_Init(void);
uint8_t SPI_Transfer(uint8_t data);
uint8_t SPI_ReadShortDebug(uint8_t addr);  // para pruebas

#endif
