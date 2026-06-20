/*
 * usart.h  - USART por hardware PIC18F4620
 * 9600 baud, 8N1, TX=RC6, RX=RC7
 */
#ifndef USART_H
#define USART_H

#include "config.h"

/* Inicializar USART */
void USART_Init(void);

/* Transmitir un byte */
void USART_PutChar(uint8_t c);

/* Transmitir string terminado en NULL */
void USART_PutString(const char *str);

/* Transmitir numero entero decimal */
void USART_PutUInt(uint16_t val);

/* Transmitir byte en hexadecimal (formato "0x??") */
void USART_PutHex(uint8_t val);

/* Recibir un byte (bloqueante con timeout) */
/* Retorna 1 si recibio, 0 si timeout */
uint8_t USART_GetChar(uint8_t *c, uint16_t timeout_ms);

/* Verificar si hay dato disponible (no bloqueante) */
uint8_t USART_DataReady(void);

/* Limpiar buffer de recepcion */
void USART_FlushRx(void);

#endif /* USART_H */
