#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "mrf24j40.h"
#include "usart.h"
#include "oled.h"
#include "spi.h"

#if !defined(MRF_MODE_TX) && !defined(MRF_MODE_RX)
    #define MRF_MODE_TX
#endif

#define PAN_ID          0xCAFE
#define CHANNEL         20      // Cambiar a 24 si se desea

#ifdef MRF_MODE_TX
    #define MODE_STRING "TRANSMISOR"
    #define MY_ADDR     0x0001
    #define DEST_ADDR   0x0002
    #define TX_INTERVAL_MS 2000
#else
    #define MODE_STRING "RECEPTOR"
    #define MY_ADDR     0x0002
#endif

#define LED_ACT     LATDbits.LATD0
#define LED_TX      LATDbits.LATD1
#define LED_RX      LATDbits.LATD2
#define LED_ERR     LATDbits.LATD3

#define BTN_TX1     PORTBbits.RB4
#define BTN_TX2     PORTBbits.RB5

static uint8_t running = 1;

static void system_init(void);
static void delay_ms(uint16_t ms);
static void test_spi(void);

#ifdef MRF_MODE_TX
    static void send_text_packet(void);
    static void send_data_packet(void);
    static void print_stats(void);
    static uint32_t packets_sent = 0;
    static uint32_t tx_success = 0;
    static uint32_t tx_fail = 0;
    static uint32_t tx_retries_total = 0;
#endif

#ifdef MRF_MODE_RX
    static void print_stats(void);
    static void process_received_packet(uint8_t* data, uint8_t len, uint8_t lqi, int8_t rssi);
    static uint32_t packets_received = 0;
    static uint32_t rx_lqi_sum = 0;
    static int32_t  rx_rssi_sum = 0;
#endif

// Interrupción externa INT1 (RB1) para el MRF24J40
void __interrupt(high_priority) isr_high(void) {
    // Verificar si la interrupción es por INT1
    if (INTCON3bits.INT1IF) {
        INTCON3bits.INT1IF = 0;   // limpiar bandera
        MRF_HandleInterrupt();     // procesar el evento del MRF24J40
        LED_RX = 1;                // indicador visual rápido (opcional)
        // No apagamos LED_RX aquí, se apagará en el bucle principal o con temporizador
    }
    // También puede haber interrupción de USART, pero ya la maneja el código de usart.c
}

void main(void) {
    system_init();

    USART_PrintString("\r\n╔══════════════════════════════════════════╗\r\n");
    USART_PrintString("║    MRF24J40 + INTERRUPCIÓN - PIC18F4620 ║\r\n");
    USART_PrintString("║    Modo: ");
    USART_PrintString(MODE_STRING);
    USART_PrintString("                    ║\r\n");
    USART_PrintString("║    USART: 19200 baudios                 ║\r\n");
    USART_PrintString("║    PAN:0xCAFE  Canal:20                 ║\r\n");
    USART_PrintString("╚══════════════════════════════════════════╝\r\n\r\n");

    // Prueba de comunicación SPI (lectura de SOFTRST)
    test_spi();

    if (!MRF_Init(CHANNEL)) {
        USART_PrintString("ERROR: MRF24J40 no responde\r\n");
        LED_ERR = 1;
        while(1);
    }
    MRF_SetPan(PAN_ID);
    MRF_SetShortAddress(MY_ADDR);

    USART_PrintString("PAN:0x");
    USART_PrintHex16(PAN_ID);
    USART_PrintString(" Addr:0x");
    USART_PrintHex16(MY_ADDR);
    USART_PrintString(" CH:");
    USART_PrintDec8(CHANNEL);
    USART_PrintString("\r\n");

    #ifdef MRF_MODE_TX
        USART_PrintString("Destino:0x");
        USART_PrintHex16(DEST_ADDR);
        USART_PrintString("\r\n");
        USART_PrintString("Cmd: s=stats t=texto d=datos\r\n");
        USART_PrintString("Botones: RB5(texto) RB4(datos)\r\n\r\n");
    #else
        USART_PrintString("Esperando paquetes (interrupción activada)...\r\n");
        USART_PrintString("Cmd: s=stats c=clear\r\n\r\n");
    #endif

    if (OLED_Init()) {
        OLED_Clear();
        OLED_DrawString(0, 0, MODE_STRING, 1, 1);
        OLED_DrawString(0, 16, "Interrupt OK", 1, 1);
        OLED_Update();
    } else {
        USART_PrintString("OLED no detectado\r\n");
    }

    LED_ACT = 1;
    uint8_t heartbeat = 0;
    uint32_t last_auto_tx = 0;

    while (running) {
        // Procesar comandos USART
        if (USART_DataAvailable()) {
            char cmd = USART_ReadChar();
            #ifdef MRF_MODE_TX
                if (cmd == 's') print_stats();
                else if (cmd == 't') send_text_packet();
                else if (cmd == 'd') send_data_packet();
            #else
                if (cmd == 's') print_stats();
                else if (cmd == 'c') {
                    packets_received = 0;
                    rx_lqi_sum = 0;
                    rx_rssi_sum = 0;
                    USART_PrintString("Stats cleared\r\n");
                }
            #endif
        }

        // Verificar si hay un paquete listo (señalado por la interrupción)
        if (MRF_HasPacket()) {
            uint8_t buffer[MAX_PAYLOAD];
            uint8_t len;
            uint8_t lqi;
            int8_t rssi;
            MRF_GetRx(buffer, &len, &lqi, &rssi);
            #ifdef MRF_MODE_RX
                process_received_packet(buffer, len, lqi, rssi);
            #endif
            LED_RX = 1;
            delay_ms(50);
            LED_RX = 0;
        }

        #ifdef MRF_MODE_TX
            // Transmisión automática
            if (last_auto_tx >= TX_INTERVAL_MS) {
                send_data_packet();
                last_auto_tx = 0;
            }
            last_auto_tx += 10;

            static uint8_t btn1_last = 1, btn2_last = 1;
            uint8_t btn1_now = BTN_TX1;
            uint8_t btn2_now = BTN_TX2;
            if (btn1_last == 1 && btn1_now == 0) {
                delay_ms(20);
                if (BTN_TX1 == 0) send_text_packet();
            }
            if (btn2_last == 1 && btn2_now == 0) {
                delay_ms(20);
                if (BTN_TX2 == 0) send_data_packet();
            }
            btn1_last = btn1_now;
            btn2_last = btn2_now;
        #endif

        // Heartbeat LED
        if (++heartbeat >= 50) {
            heartbeat = 0;
            LED_ACT = !LED_ACT;
        }

        delay_ms(10);
    }

    while(1);
}

void system_init(void) {
    OSCCON = 0x60;
    TRISD = 0x00;
    LATD = 0x00;
    TRISB |= 0x30;          // RB4, RB5 entradas
    nRBPU = 0;              // pull-ups habilitados

    // Configurar pin RB1 como entrada para la interrupción INT1
    TRISBbits.TRISB1 = 1;
    // Configurar INT1 en flanco de bajada (porque el MRF24J40 activa INT a nivel bajo)
    INTCON2bits.INTEDG1 = 0;   // flanco de bajada
    // Limpiar bandera y habilitar INT1
    INTCON3bits.INT1IF = 0;
    INTCON3bits.INT1IE = 1;    // habilitar interrupción INT1
    // Prioridad alta (por defecto)
    RCONbits.IPEN = 1;         // habilitar prioridades (opcional)
    INTCONbits.PEIE = 1;       // interrupciones periféricas
    INTCONbits.GIE = 1;        // interrupciones globales

    SPI_Init();
    USART_Init(19200);
    OLED_Init();
}

void delay_ms(uint16_t ms) {
    for (uint16_t i = 0; i < ms; i++) __delay_ms(1);
}

void test_spi(void) {
    uint8_t val;
    USART_PrintString("\r\n[TEST SPI] Leyendo REG_SOFTRST...\r\n");
    for (uint8_t i = 0; i < 3; i++) {
        val = SPI_ReadShortDebug(REG_SOFTRST);
        USART_PrintString("Valor = 0x");
        USART_PrintHex8(val);
        USART_PrintString("\r\n");
        __delay_ms(100);
    }
    if (val == 0x00) {
        USART_PrintString("SPI: Comunicación correcta (SOFTRST=0x00)\r\n");
    } else {
        USART_PrintString("SPI: ¡ERROR! Verifique MISO (RC4) y alimentación.\r\n");
    }
}

#ifdef MRF_MODE_TX
void send_text_packet(void) {
    const char* msg = "Hola PIC18F4620";
    uint8_t len = strlen(msg);
    USART_PrintString("\r\n[TX] Texto: ");
    USART_PrintString(msg);
    if (MRF_Send(DEST_ADDR, PAN_ID, (uint8_t*)msg, len)) {
        uint16_t timeout = 100;
        while (MRF_IsTxPending() && timeout--) {
            delay_ms(1);
        }
        if (MRF_TxSuccess()) {
            USART_PrintString(" -> OK (retries=");
            USART_PrintDec8(MRF_TxRetries());
            USART_PrintString(")\r\n");
            tx_success++;
            LED_TX = 1; delay_ms(30); LED_TX = 0;
        } else {
            USART_PrintString(" -> FAIL\r\n");
            tx_fail++;
            LED_ERR = 1; delay_ms(100); LED_ERR = 0;
        }
    } else {
        USART_PrintString(" -> BUSY\r\n");
    }
    packets_sent++;
    tx_retries_total += MRF_TxRetries();
}

void send_data_packet(void) {
    uint8_t payload[100];
    static uint16_t counter = 0;
    for (uint8_t i = 0; i < 100; i++) payload[i] = (counter + i) & 0xFF;
    counter++;
    USART_PrintString("\r\n[TX] Datos 100B [");
    USART_PrintHex8(payload[0]);
    USART_PrintString("...");
    USART_PrintHex8(payload[99]);
    USART_PrintString("] ");
    if (MRF_Send(DEST_ADDR, PAN_ID, payload, 100)) {
        uint16_t timeout = 100;
        while (MRF_IsTxPending() && timeout--) {
            delay_ms(1);
        }
        if (MRF_TxSuccess()) {
            USART_PrintString("OK (retries=");
            USART_PrintDec8(MRF_TxRetries());
            USART_PrintString(")\r\n");
            tx_success++;
            LED_TX = 1; delay_ms(30); LED_TX = 0;
        } else {
            USART_PrintString("FAIL\r\n");
            tx_fail++;
            LED_ERR = 1; delay_ms(100); LED_ERR = 0;
        }
    } else {
        USART_PrintString("BUSY\r\n");
    }
    packets_sent++;
    tx_retries_total += MRF_TxRetries();
}

void print_stats(void) {
    USART_PrintString("\r\n=== ESTADÍSTICAS TX ===\r\n");
    USART_PrintString("Enviados: "); USART_PrintDec32(packets_sent);
    USART_PrintString(", OK: "); USART_PrintDec32(tx_success);
    USART_PrintString(", Fail: "); USART_PrintDec32(tx_fail);
    USART_PrintString("\r\nRetransmisiones: "); USART_PrintDec32(tx_retries_total);
    USART_PrintString("\r\n");
}
#endif

#ifdef MRF_MODE_RX
void process_received_packet(uint8_t* data, uint8_t len, uint8_t lqi, int8_t rssi) {
    packets_received++;
    rx_lqi_sum += lqi;
    rx_rssi_sum += rssi;
    USART_PrintString("\r\n[RX #"); USART_PrintDec32(packets_received);
    USART_PrintString("] "); USART_PrintDec8(len); USART_PrintString(" bytes: ");
    for (uint8_t i = 0; i < (len < 16 ? len : 16); i++) {
        USART_PrintHex8(data[i]); USART_PrintChar(' ');
    }
    if (len > 16) USART_PrintString("...");
    USART_PrintString("\r\n      LQI: "); USART_PrintDec8(lqi);
    USART_PrintString("/255, RSSI: "); USART_PrintDec8(rssi); USART_PrintString(" dBm\r\n");
    char text[17] = {0};
    uint8_t txtlen = len > 16 ? 16 : len;
    for (uint8_t i = 0; i < txtlen; i++) text[i] = (data[i] >= 32 && data[i] <= 126) ? data[i] : '.';
    OLED_Clear();
    OLED_DrawString(0, 0, "RX #", 1, 1);
    OLED_DrawDec32(packets_received, 40, 0, 1);
    OLED_DrawString(0, 16, text, 1, 1);
    char lqi_str[20];
    sprintf(lqi_str, "LQI:%d RSSI:%d", lqi, rssi);
    OLED_DrawString(0, 48, lqi_str, 1, 1);
    OLED_Update();
}

void print_stats(void) {
    USART_PrintString("\r\n=== ESTADÍSTICAS RX ===\r\n");
    USART_PrintString("Recibidos: "); USART_PrintDec32(packets_received);
    if (packets_received > 0) {
        USART_PrintString("\r\nLQI prom: "); USART_PrintDec16(rx_lqi_sum / packets_received);
        USART_PrintString("\r\nRSSI prom: "); USART_PrintDec16(rx_rssi_sum / packets_received); USART_PrintString(" dBm");
    }
    USART_PrintString("\r\n");
}
#endif
