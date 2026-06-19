#include "uart.h"  // Header de funciones UART

#pragma warning disable 520  // Desactiva warning 520 (función sin prototipo)

void UART_Init(void)  // Inicializa el módulo UART para comunicación serie
{
    TRISCbits.TRISC6 = 1;  // Pin RC6 como entrada (TX - salida del módulo USART)
    TRISCbits.TRISC7 = 1;  // Pin RC7 como entrada (RX - entrada del módulo USART)
    SPBRG = 25;            // Configura el baudrate (25 para 9600 baudios a 4MHz)
    TXSTA = 0x24;          // Configura transmisión: BRGH=1, TXEN=1, modo asíncrono
    RCSTA = 0x90;          // Configura recepción: SPEN=1, CREN=1
}

void UART_SendChar(char c)  // Envía un carácter por el puerto UART
{
    while (!PIR1bits.TXIF);  // Espera a que el buffer de transmisión esté vacío
    TXREG = c;               // Carga el carácter en el registro de transmisión
}

char UART_ReceiveChar(void)  // Recibe un carácter por el puerto UART
{
    while (!PIR1bits.RCIF);  // Espera a que haya un dato disponible en recepción
    return RCREG;            // Retorna el carácter recibido
}

void UART_SendString(const char *str)  // Envía una cadena terminada en nulo por UART
{
    while (*str) {          // Mientras no se llegue al final de la cadena
        UART_SendChar(*str++);  // Envía el carácter actual y avanza al siguiente
    }
}
