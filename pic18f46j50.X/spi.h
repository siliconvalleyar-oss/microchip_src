#ifndef SPI_H
#define SPI_H

#include <xc.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void SPI_Init(void);
uint8_t SPI_Write(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif