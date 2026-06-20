#ifndef SDCARD_H
#define SDCARD_H

#include <xc.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void SD_Select(void);
void SD_Deselect(void);
uint8_t SD_Init(void);
void SD_ReadLogo(uint8_t *buffer);

uint8_t SD_ReadByte(void); 


#ifdef __cplusplus
}
#endif

#endif