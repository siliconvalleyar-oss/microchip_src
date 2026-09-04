#ifndef SPI_SD_H
#define SPI_SD_H

#include "hal_config.h"

void SPI_SD_Init(void);
uint8_t SPI_SD_TransferByte(uint8_t data);
void SPI_SD_Select(void);
void SPI_SD_Deselect(void);

#endif
