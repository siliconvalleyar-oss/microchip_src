#include "hal/hal_config.h"
#include "usb/usb_cdc.h"
#include <string.h>

// Esta es una implementación simplificada de USB CDC.
// En un proyecto real, se necesita la pila USB completa de Microchip.
// Para este ejemplo, usaremos una simulación básica.

static uint8_t usb_connected = 0;
static uint8_t rx_buffer[64];
static uint8_t rx_head = 0;
static uint8_t rx_tail = 0;

void USB_CDC_Init(void)
{
    // Inicializar buffers
    memset(rx_buffer, 0, sizeof(rx_buffer));
    rx_head = 0;
    rx_tail = 0;

    // Configurar pines USB
    TRISCbits.TRISC4 = 1;  // D- como entrada
    TRISCbits.TRISC5 = 1;  // D+ como entrada

    // Habilitar módulo USB
    UCFGbits.UPUEN = 1;    // Pull-up en D+ habilitado
    UCFGbits.UTEYE = 0;    // Modo normal
    UCFGbits.UOEMON = 0;   // Modo normal
    UCFGbits.UTRDIS = 0;   // Transceiver habilitado
    UCFGbits.FSEN = 1;     // Full-speed enabled

    // Para simplificar, simulamos conexión después de 500ms
    __delay_ms(500);
    usb_connected = 1;
}

void USB_CDC_Tasks(void)
{
    // En una implementación real, aquí se procesarían eventos USB
    // Por ahora, simulamos que siempre está conectado
}

//uint8_t USB_CDC_IsConnected(void)
//{
//    return usb_connected;
//}

void USB_CDC_WriteByte(uint8_t data)
{
    // En una implementación real, enviar por USB
    // Aquí simulamos enviando por UART para depuración
    // TXSTA, RCSTA, etc.

    // Simulación: usar UART1 a 9600 (TX en RC6)
    static uint8_t uart_initialized = 0;
    if(!uart_initialized) {
        TRISCbits.TRISC6 = 0;  // TX como salida
        TXSTA = 0x24;          // High speed, enable TX
        RCSTA = 0x80;          // Enable serial port
        SPBRG = 77;            // 12MHz / (16*77) = 9740 baud (cerca)
        uart_initialized = 1;
    }

    while(!TXSTAbits.TRMT);
    TXREG = data;
}

void USB_CDC_WriteString(const char* str)
{
    while(*str) {
        USB_CDC_WriteByte(*str++);
    }
}

uint8_t USB_CDC_ReadData(char* buffer, uint8_t max_len)
{
    // Simular lectura desde UART1 (RX en RC7)
    if(RCSTAbits.OERR) {
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
    }

    uint8_t len = 0;
    while(PIR1bits.RCIF && len < max_len-1) {
        buffer[len++] = RCREG;
    }
    buffer[len] = '\0';
    return len;
}
