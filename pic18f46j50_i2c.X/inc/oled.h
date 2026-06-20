
// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef OLED_H
#define	OLED_H

#include <xc.h> // include processor files - each processor file is guarded.  
//----------------------------------------
// DECLARACIÓN DE FUNCIONES
//----------------------------------------

void OLED_Init(void);
void OLED_WriteCmd(uint8_t cmd);
void OLED_WriteData(uint8_t data);
void OLED_Clear(void);
void OLED_SetCursor(uint8_t x, uint8_t y);
void OLED_PrintChar(char c);
void OLED_PrintString(const char* str);
void OLED_PrintNumber(uint8_t num);

#endif	/* XC_HEADER_TEMPLATE_H */

