/* =====================================================================
 * PIC18F4620 + MCP2515 (CAN) -> Lector OBD-II
 * -----------------------------------------------------------------------
 * Lee parametros del vehiculo (RPM, velocidad, temperatura de motor,
 * carga del motor, posicion del acelerador) via bus CAN (protocolo
 * ISO 15765-4 / OBD-II sobre CAN 500 kbps, 11 bits) usando un modulo
 * MCP2515 y los muestra por el puerto serie (UART) del PIC.
 *
 * Compilador: Microchip XC8
 * MCU:        PIC18F4620
 * Cristal:    20 MHz (HS)
 * MCP2515:    Cristal de 8 MHz
 * =====================================================================*/

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "mcp2515.h"

/* ---------------------- Configuration bits -------------------------- */
#pragma config OSC     = HS        // Oscilador HS (cristal 20 MHz)
#pragma config WDT     = OFF       // Watchdog apagado
#pragma config LVP     = OFF       // Bajo voltaje de programacion apagado
#pragma config PBADEN  = OFF       // PORTB<4:0> como digital al reset
#pragma config MCLRE   = ON        // MCLR habilitado
#pragma config DEBUG   = OFF
#pragma config PWRT    = ON

#define _XTAL_FREQ 20000000UL      // Frecuencia del cristal del PIC

/* ============================ UART ================================= */
#define UART_BAUD 9600UL

static void UART_Init(void) {
    TRISCbits.TRISC6 = 0;   // TX salida
    TRISCbits.TRISC7 = 1;   // RX entrada

    TXSTAbits.SYNC = 0;     // Modo asincrono
    TXSTAbits.BRGH = 1;     // Alta velocidad
    BAUDCONbits.BRG16 = 1;  // Generador de baudios de 16 bits

    #define SPBRG_VAL ((_XTAL_FREQ / (4UL * UART_BAUD)) - 1)
    SPBRGH = (uint8_t)(SPBRG_VAL >> 8);
    SPBRG  = (uint8_t)(SPBRG_VAL & 0xFF);

    TXSTAbits.TXEN = 1;     // Habilita transmisor
    RCSTAbits.SPEN = 1;     // Habilita puerto serie
    RCSTAbits.CREN = 1;     // Habilita recepcion continua
}

/* Redireccion de printf() de XC8 hacia la UART */
void putch(char c) {
    while (!TXSTAbits.TRMT);
    TXREG = c;
}

/* ============================ OBD-II ================================= */
#define OBD2_REQUEST_ID   0x7DF   // ID de peticion "broadcast" a la ECU
#define OBD2_RESPONSE_ID  0x7E8   // ID tipico de respuesta del motor (ECU #1)

#define PID_ENGINE_LOAD    0x04
#define PID_COOLANT_TEMP   0x05
#define PID_ENGINE_RPM     0x0C
#define PID_VEHICLE_SPEED  0x0D
#define PID_THROTTLE_POS   0x11

/* Envia una peticion de PID de modo 01 (datos en tiempo real) */
static void OBD2_RequestPID(uint8_t pid) {
    uint8_t data[8] = {0x02, 0x01, pid, 0x00, 0x00, 0x00, 0x00, 0x00};
    mcp2515_send_message(OBD2_REQUEST_ID, data, 8);
}

/* Espera una respuesta valida durante 'timeout_ms' aproximados.
 * Devuelve 1 si obtuvo respuesta valida para el pid pedido, 0 si no. */
static uint8_t OBD2_WaitResponse(uint8_t pid, uint8_t *A, uint8_t *B) {
    uint16_t id;
    uint8_t data[8];
    uint8_t len;
    uint16_t timeout = 500;   // ~500 * 2ms = 1s aprox

    while (timeout--) {
        if (mcp2515_receive_message(&id, data, &len)) {
            if (id == OBD2_RESPONSE_ID && len >= 3 &&
                data[1] == 0x41 && data[2] == pid) {
                *A = (len >= 4) ? data[3] : 0;
                *B = (len >= 5) ? data[4] : 0;
                return 1;
            }
        }
        __delay_ms(2);
    }
    return 0;
}

/* ============================== MAIN ================================= */
void main(void) {
    ADCON1 = 0x0F;   // Todos los pines digitales (no ADC)

    UART_Init();
    printf("\r\n--- Lector OBD-II (PIC18F4620 + MCP2515) ---\r\n");

    mcp2515_init();
    printf("MCP2515 inicializado a 500 kbps.\r\n\r\n");

    uint8_t A, B;

    while (1) {
        OBD2_RequestPID(PID_ENGINE_RPM);
        if (OBD2_WaitResponse(PID_ENGINE_RPM, &A, &B)) {
            uint16_t rpm = ((uint16_t)A * 256 + B) / 4;
            printf("RPM: %u rpm\r\n", rpm);
        } else {
            printf("RPM: sin respuesta\r\n");
        }

        OBD2_RequestPID(PID_VEHICLE_SPEED);
        if (OBD2_WaitResponse(PID_VEHICLE_SPEED, &A, &B)) {
            printf("Velocidad: %u km/h\r\n", A);
        } else {
            printf("Velocidad: sin respuesta\r\n");
        }

        OBD2_RequestPID(PID_COOLANT_TEMP);
        if (OBD2_WaitResponse(PID_COOLANT_TEMP, &A, &B)) {
            int16_t temp = (int16_t)A - 40;
            printf("Temp. motor: %d C\r\n", temp);
        } else {
            printf("Temp. motor: sin respuesta\r\n");
        }

        OBD2_RequestPID(PID_ENGINE_LOAD);
        if (OBD2_WaitResponse(PID_ENGINE_LOAD, &A, &B)) {
            uint16_t load = ((uint16_t)A * 100) / 255;
            printf("Carga motor: %u %%\r\n", load);
        } else {
            printf("Carga motor: sin respuesta\r\n");
        }

        OBD2_RequestPID(PID_THROTTLE_POS);
        if (OBD2_WaitResponse(PID_THROTTLE_POS, &A, &B)) {
            uint16_t thr = ((uint16_t)A * 100) / 255;
            printf("Acelerador: %u %%\r\n", thr);
        } else {
            printf("Acelerador: sin respuesta\r\n");
        }

        printf("--------------------------------\r\n");
        __delay_ms(500);
    }
}
