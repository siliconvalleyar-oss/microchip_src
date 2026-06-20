#ifndef UART_H  // Evita inclusión múltiple del header
#define UART_H  // Define el guardián de inclusión

#include "config.h"  // Configuración general y definiciones del sistema

// Prototipos de funciones UART
void UART_Init(void);                // Inicializa el módulo UART
void UART_SendChar(char c);          // Envía un carácter por UART
char UART_ReceiveChar(void);         // Recibe un carácter por UART
void UART_SendString(const char *str); // Envía una cadena terminada en nulo por UART

#endif  // Fin del guardián de inclusión
