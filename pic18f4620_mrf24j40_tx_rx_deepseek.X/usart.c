#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "usart.h"
#include "config.h"

#define RX_BUFFER_SIZE 64

static volatile char rx_buffer[RX_BUFFER_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;
static volatile uint8_t rx_has_data = 0;

void __interrupt(high_priority) isr_high(void) {
    if (RCIF && RCIE) {
        char c = RCREG;
        uint8_t next = (rx_head + 1) % RX_BUFFER_SIZE;
        if (next != rx_tail) {
            rx_buffer[rx_head] = c;
            rx_head = next;
            rx_has_data = 1;
        }
        RCIF = 0;
    }
}

void USART_Init(uint32_t baudrate) {
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 1;
    uint16_t spbrg = (4000000 / (16 * baudrate)) - 1;
    TXSTA = 0x24;
    RCSTA = 0x90;
    BAUDCON = 0x00;
    SPBRG = spbrg;
    RCIE = 1;
    PEIE = 1;
    GIE = 1;
}

void USART_PrintChar(char c) {
    while (!TXSTAbits.TRMT);
    TXREG = c;
}

void USART_PrintString(const char* s) {
    while (*s) USART_PrintChar(*s++);
}

void USART_PrintHex8(uint8_t val) {
    const char hex[] = "0123456789ABCDEF";
    USART_PrintChar(hex[val >> 4]);
    USART_PrintChar(hex[val & 0x0F]);
}

void USART_PrintHex16(uint16_t val) {
    USART_PrintHex8(val >> 8);
    USART_PrintHex8(val & 0xFF);
}

void USART_PrintDec8(uint8_t val) {
    char buf[4];
    uint8_t i = 2;
    buf[3] = 0;
    do {
        buf[i--] = '0' + (val % 10);
        val /= 10;
    } while (val && i);
    USART_PrintString(&buf[i+1]);
}

void USART_PrintDec16(uint16_t val) {
    char buf[6];
    uint8_t i = 4;
    buf[5] = 0;
    do {
        buf[i--] = '0' + (val % 10);
        val /= 10;
    } while (val && i);
    USART_PrintString(&buf[i+1]);
}

void USART_PrintDec32(uint32_t val) {
    char buf[12];
    uint8_t i = 10;
    buf[11] = 0;
    do {
        buf[i--] = '0' + (val % 10);
        val /= 10;
    } while (val && i);
    USART_PrintString(&buf[i+1]);
}

bool USART_DataAvailable(void) {
    return rx_has_data;
}

char USART_ReadChar(void) {
    char c = 0;
    if (rx_head != rx_tail) {
        c = rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
        if (rx_head == rx_tail) rx_has_data = 0;
    }
    return c;
}
