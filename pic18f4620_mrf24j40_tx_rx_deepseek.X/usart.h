#ifndef USART_H
#define USART_H

#include <stdint.h>
#include <stdbool.h>

void USART_Init(uint32_t baudrate);
void USART_PrintChar(char c);
void USART_PrintString(const char* s);
void USART_PrintHex8(uint8_t val);
void USART_PrintHex16(uint16_t val);
void USART_PrintDec8(uint8_t val);
void USART_PrintDec16(uint16_t val);
void USART_PrintDec32(uint32_t val);
bool USART_DataAvailable(void);
char USART_ReadChar(void);

#endif
