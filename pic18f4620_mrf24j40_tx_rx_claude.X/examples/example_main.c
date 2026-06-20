/*
 * main.c  - Aplicacion principal PIC18F4620 + MRF24J40
 *
 * Comportamiento:
 *
 *  MODO_TX (define en config.h):
 *    - Loop: cada 2 segundos transmite via MRF24J40 un mensaje
 *      con contador al destino MRF_DEST_ADDR (Raspberry A).
 *    - Boton RB4: transmite mensaje inmediato.
 *    - Boton RB5: cambia destino entre Raspberry A y B.
 *    - LED0: parpadea al transmitir.
 *    - LED1: se enciende si TX exitoso, apaga si fallo.
 *    - LED2: alterna con cada boton presionado.
 *    - LED3: walking LED de actividad.
 *    - USART: reporta estado TX a PC @ 9600 baud.
 *    - OLED: muestra contador y estado.
 *
 *  MODO_RX (define en config.h):
 *    - Loop: espera paquetes MRF24J40.
 *    - Al recibir: muestra datos en OLED y los reenvía por USART.
 *    - LED0: parpadea al recibir.
 *    - LED1-3: patron con LQI del ultimo paquete.
 *    - Boton RB4: limpiar display.
 *    - Boton RB5: enviar respuesta ACK manual.
 */
#include "pic18_fuses.h"
#include "config.h"
#include "usart.h"
#include "spi.h"
#include "mrf24j40.h"
#include "i2c_sw.h"
#include "oled.h"
#include "leds_btns.h"

/* ---- Prototipos locales --------------------------------- */
static void SYS_Init(void);
static void MRF_Setup(void);

#ifdef MODO_TX
static void App_TX_Loop(void);
#else
static void App_RX_Loop(void);
#endif

/* ---- Contador de ms via polling (aproximado) ----------- */
/* Para un timer preciso usar TMR0/TMR1, aqui usamos delay */
static uint16_t ms_tick = 0u;

/* ================================================================
 * MAIN
 * ================================================================ */
void main(void)
{
    SYS_Init();

    /* Banner USART */
    USART_PutString("\r\n=============================\r\n");
    USART_PutString("  PIC18F4620 + MRF24J40\r\n");
#ifdef MODO_TX
    USART_PutString("  MODO: TRANSMISOR\r\n");
#else
    USART_PutString("  MODO: RECEPTOR\r\n");
#endif
    USART_PutString("  Fosc=4MHz  9600 baud\r\n");
    USART_PutString("  Canal: ");
    USART_PutUInt(MRF_CHANNEL);
    USART_PutString("  PAN: 0x");
    USART_PutHex((uint8_t)(MRF_PAN_ID >> 8u));
    USART_PutHex((uint8_t)(MRF_PAN_ID & 0xFFu));
    USART_PutString("\r\n  Mi addr: 0x");
    USART_PutHex((uint8_t)(MRF_MY_ADDR >> 8u));
    USART_PutHex((uint8_t)(MRF_MY_ADDR & 0xFFu));
    USART_PutString("\r\n=============================\r\n");

    /* OLED banner */
    OLED_Clear();
    OLED_PutStr(0, 0, "PIC18+MRF24J40");
#ifdef MODO_TX
    OLED_PutStr(1, 0, "  TRANSMISOR");
#else
    OLED_PutStr(1, 0, "  RECEPTOR");
#endif
    OLED_PutStr(3, 0, "PAN:0xCAFE");
    OLED_PutStr(4, 0, "Canal: 20");
    OLED_Update();
    __delay_ms(1500);

    /* Loop principal */
#ifdef MODO_TX
    App_TX_Loop();
#else
    App_RX_Loop();
#endif
}

/* ================================================================
 * Inicializacion del sistema
 * ================================================================ */
static void SYS_Init(void)
{
    /* Deshabilitar comparadores analogicos */
    CMCON  = 0x07;
    CVRCON = 0x00;

    /* PORTA/B/C/D/E como digitales */
    ADCON1 = 0x0F;   /* AN0-AN12 como digitales */
    TRISA  = 0x00;
    TRISD  = 0x00;
    TRISE  = 0x00;
    LATD   = 0x00;

    /* Inicializar perifericos en orden */
    LEDS_BTNS_Init();

    /* Efecto de arranque: LEDs en secuencia */
    LED_AllOn();
    __delay_ms(300);
    LED_AllOff();

    I2C_SW_Init();
    OLED_Init();
    USART_Init();
    SPI_Init();

    /* Inicializar y configurar MRF24J40 */
    MRF_Setup();
}

/* ================================================================
 * Configurar MRF24J40
 * ================================================================ */
static void MRF_Setup(void)
{
    uint8_t ok;
    uint16_t pan, addr;

    USART_PutString("[MRF] Inicializando...\r\n");
    LED_Pattern(0b0101);

    ok = MRF_Init();
    if (!ok) {
        USART_PutString("[MRF] ERROR: No responde!\r\n");
        OLED_Clear();
        OLED_PutStr(0, 0, "MRF ERROR!");
        OLED_PutStr(1, 0, "Sin respuesta");
        OLED_Update();
        /* Parpadear LED3 indefinidamente */
        while (1) {
            LED3 ^= 1;
            __delay_ms(200);
        }
    }

    /* Configurar red */
    MRF_SetPAN(MRF_PAN_ID);
    MRF_SetAddr(MRF_MY_ADDR);

    /* Verificar registros */
    pan  = MRF_GetPAN();
    addr = MRF_GetAddr();

    USART_PutString("[MRF] PAN=0x");
    USART_PutHex((uint8_t)(pan >> 8u));
    USART_PutHex((uint8_t)(pan & 0xFFu));
    USART_PutString("  ADDR=0x");
    USART_PutHex((uint8_t)(addr >> 8u));
    USART_PutHex((uint8_t)(addr & 0xFFu));

    if (pan == MRF_PAN_ID && addr == MRF_MY_ADDR) {
        USART_PutString("  [OK]\r\n");
        LED_Pattern(0b1010);
    } else {
        USART_PutString("  [WARN: mismatch]\r\n");
        LED_Pattern(0b1111);
    }

    __delay_ms(200);
    LED_AllOff();
}

/* ================================================================
 * Loop TRANSMISOR
 * ================================================================ */
#ifdef MODO_TX
static void App_TX_Loop(void)
{
    uint16_t counter  = 0u;
    uint16_t tx_timer = 0u;           /* ms */
    uint16_t dest     = MRF_DEST_ADDR;
    uint8_t  tx_ok_flag;
    char     msg[32];
    uint8_t  msg_len;
    uint8_t  i;

    USART_PutString("[TX] Loop iniciado\r\n");
    OLED_Clear();
    OLED_PutStr(0, 0, "TX LOOP");
    OLED_PutStr(2, 0, "CNT:");
    OLED_PutStr(4, 0, "DST:");
    OLED_PutStr(6, 0, "ST:");
    OLED_Update();

    while (1) {
        /* Debounce de botones cada ciclo */
        BTN_Update();

        /* ---- Boton RB4: TX manual ---- */
        if (BTN_Pressed(0)) {
            tx_timer = 2001u;   /* Forzar TX inmediato */
            LED2 ^= 1u;
            USART_PutString("[BTN0] TX manual\r\n");
        }

        /* ---- Boton RB5: cambiar destino ---- */
        if (BTN_Pressed(1)) {
            if (dest == MRF_DEST_ADDR) dest = MRF_ALT_ADDR;
            else                        dest = MRF_DEST_ADDR;
            LED2 ^= 1u;
            USART_PutString("[BTN1] Destino: 0x");
            USART_PutHex((uint8_t)(dest >> 8u));
            USART_PutHex((uint8_t)(dest & 0xFFu));
            USART_PutString("\r\n");
        }

        /* ---- Transmitir cada 2 segundos ---- */
        tx_timer++;
        if (tx_timer > 2000u) {
            tx_timer = 0u;

            /* Construir mensaje: "PIC:NNNN" */
            msg[0] = 'P'; msg[1] = 'I'; msg[2] = 'C'; msg[3] = ':';
            /* Convertir contador a string */
            i = 4u;
            if (counter == 0u) {
                msg[i++] = '0';
            } else {
                uint16_t tmp = counter;
                uint8_t start = i;
                char tmp_c;
                while (tmp > 0u) { msg[i++] = (char)('0' + tmp%10u); tmp/=10u; }
                /* Invertir parte numerica */
                uint8_t end = i - 1u;
                while (start < end) {
                    tmp_c = msg[start]; msg[start] = msg[end]; msg[end] = tmp_c;
                    start++; end--;
                }
            }
            msg[i] = '\0';
            msg_len = i;

            /* LED0: indicar TX */
            LED0 = 1u;

            /* Transmitir */
            USART_PutString("[TX] -> 0x");
            USART_PutHex((uint8_t)(dest >> 8u));
            USART_PutHex((uint8_t)(dest & 0xFFu));
            USART_PutString(" msg=\"");
            USART_PutString(msg);
            USART_PutString("\" ... ");

            tx_ok_flag = MRF_Send(dest, (const uint8_t *)msg, msg_len);

            LED0 = 0u;

            if (tx_ok_flag) {
                LED1 = 1u;
                USART_PutString("OK (r=");
                USART_PutUInt(MRF_TxRetries());
                USART_PutString(")\r\n");
                OLED_PutStr(6, 0, "ST: TX OK   ");
            } else {
                LED1 = 0u;
                USART_PutString("FALLO\r\n");
                OLED_PutStr(6, 0, "ST: TX FAIL ");
            }

            /* Actualizar OLED */
            OLED_Clear();
            OLED_PutStr(0, 0, "TX LOOP");
            OLED_PutStr(2, 0, "MSG:");
            OLED_PutStr(2, 4, msg);
            OLED_PutStr(4, 0, "DST:");
            if (dest == MRF_DEST_ADDR)
                OLED_PutStr(4, 4, "RaspA 0x0001");
            else
                OLED_PutStr(4, 4, "RaspB 0x0002");
            OLED_PutStr(6, 0, tx_ok_flag ? "OK     " : "FALLO  ");
            OLED_Update();

            counter++;

            /* LED walking: actividad */
            LED_Walk();
        }

        /* ---- Polling RX (recibir respuestas) ---- */
        {
            MRF_RxPacket pkt;
            if (MRF_Poll(&pkt)) {
                USART_PutString("[RX] De 0x");
                USART_PutHex((uint8_t)(pkt.src_addr >> 8u));
                USART_PutHex((uint8_t)(pkt.src_addr & 0xFFu));
                USART_PutString(": ");
                USART_PutString((const char *)pkt.data);
                USART_PutString("  LQI=");
                USART_PutUInt(pkt.lqi);
                USART_PutString("\r\n");
            }
        }

        __delay_ms(1);
    }
}
#endif /* MODO_TX */

/* ================================================================
 * Loop RECEPTOR
 * ================================================================ */
#ifndef MODO_TX
static void App_RX_Loop(void)
{
    MRF_RxPacket pkt;
    uint16_t     rx_count = 0u;

    USART_PutString("[RX] Esperando paquetes...\r\n");
    OLED_Clear();
    OLED_PutStr(0, 0, "RX LOOP");
    OLED_PutStr(2, 0, "Esperando...");
    OLED_Update();

    while (1) {
        BTN_Update();

        /* ---- Boton RB4: limpiar display ---- */
        if (BTN_Pressed(0)) {
            OLED_Clear();
            OLED_PutStr(0, 0, "RX - limpiado");
            OLED_Update();
            LED2 ^= 1u;
        }

        /* ---- Boton RB5: enviar ACK/respuesta ---- */
        if (BTN_Pressed(1)) {
            const char *ack = "ACK-PIC";
            MRF_SendStr(MRF_DEST_ADDR, ack);
            USART_PutString("[BTN1] ACK enviado\r\n");
            LED2 ^= 1u;
        }

        /* ---- Polling MRF ---- */
        if (MRF_Poll(&pkt)) {
            rx_count++;
            LED0 = 1u;

            /* USART: mostrar paquete */
            USART_PutString("[RX #");
            USART_PutUInt(rx_count);
            USART_PutString("] src=0x");
            USART_PutHex((uint8_t)(pkt.src_addr >> 8u));
            USART_PutHex((uint8_t)(pkt.src_addr & 0xFFu));
            USART_PutString(" len=");
            USART_PutUInt(pkt.len);
            USART_PutString(" data=\"");
            USART_PutString((const char *)pkt.data);
            USART_PutString("\" LQI=");
            USART_PutUInt(pkt.lqi);
            USART_PutString(" RSSI=");
            USART_PutUInt((uint16_t)(uint8_t)pkt.rssi_dbm);
            USART_PutString("dBm\r\n");

            /* OLED */
            OLED_Clear();
            OLED_PutStr(0, 0, "RECIBIDO");
            OLED_PutStr(1, 0, "DATA:");
            OLED_PutStr(1, 5, (const char *)pkt.data);
            OLED_PutStr(3, 0, "SRC:");
            OLED_PutUInt(3, 4, pkt.src_addr);
            OLED_PutStr(5, 0, "LQI:");
            OLED_PutUInt(5, 4, pkt.lqi);
            OLED_PutStr(6, 0, "CNT:");
            OLED_PutUInt(6, 4, rx_count);
            OLED_Update();

            /* LEDs: patron con LQI (bits 7:4) */
            LED_Pattern((uint8_t)(pkt.lqi >> 4u));

            LED0 = 0u;
        }

        /* ---- Polling USART RX ---- */
        if (USART_DataReady()) {
            uint8_t c;
            if (USART_GetChar(&c, 10u)) {
                /* Echo */
                USART_PutChar(c);
                /* Comando 'r' = reset contador */
                if (c == 'r' || c == 'R') {
                    rx_count = 0u;
                    USART_PutString("\r\n[CMD] Contador reseteado\r\n");
                }
            }
        }

        __delay_ms(1);
    }
}
#endif /* !MODO_TX */
