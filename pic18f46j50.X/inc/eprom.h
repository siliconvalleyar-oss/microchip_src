#ifndef EPROM_H
#define EPROM_H

#include <xc.h>
#include <stdint.h>

void EEPROM_Write(uint8_t address, uint8_t data);
uint8_t EEPROM_Read(uint8_t address);



void EEPROM_WriteString(uint8_t startAddress, const char* str);

void EEPROM_ReadString(uint8_t startAddress, char* buffer, uint8_t maxLen);
   
    
#endif
