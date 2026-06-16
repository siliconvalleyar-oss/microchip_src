#!/bin/bash

PROYECT_DIR="pic_mrf24j40_interrupt"
mkdir -p "$PROYECT_DIR"
cd "$PROYECT_DIR"

# -------------------- config.h --------------------
cat > config.h << 'EOF'
#ifndef CONFIG_H
#define CONFIG_H

#define _XTAL_FREQ 4000000

// Fuses compatibles con tu compilador
#pragma config OSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config DEBUG = OFF

#endif
EOF

# -------------------- spi.h --------------------
cat > spi.h << 'EOF'
#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void SPI_Init(void);
uint8_t SPI_Transfer(uint8_t data);
uint8_t SPI_ReadShortDebug(uint8_t addr);  // para pruebas

#endif
EOF

# -------------------- spi.c --------------------
cat > spi.c << 'EOF'
#include <xc.h>
#include "spi.h"
#include "mrf24j40.h"
#include "config.h"

void SPI_Init(void) {
    TRISCbits.TRISC3 = 0;   // SCK salida
    TRISCbits.TRISC4 = 1;   // MISO entrada (importante)
    TRISCbits.TRISC5 = 0;   // MOSI salida
    SSPSTAT = 0x00;          // SMP=0, CKE=0
    SSPCON1 = 0x20;          // SSPEN=1, CKP=0, Fosc/64 = 62.5 kHz
    SSPCON2 = 0x00;
}

uint8_t SPI_Transfer(uint8_t data) {
    SSPBUF = data;
    while (!SSPSTATbits.BF);
    return SSPBUF;
}

// Función de prueba: lee un registro short
uint8_t SPI_ReadShortDebug(uint8_t addr) {
    uint8_t cmd = (addr & 0x3F) << 1;
    uint8_t result;
    MRF_CS_L();
    __delay_us(1);
    SPI_Transfer(cmd);
    result = SPI_Transfer(0x00);
    __delay_us(1);
    MRF_CS_H();
    return result;
}
EOF

# -------------------- mrf24j40.h --------------------
cat > mrf24j40.h << 'EOF'
#ifndef MRF24J40_H
#define MRF24J40_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PAYLOAD 100

#define MRF_CS_PIN   LATC0
#define MRF_CS_L()   (MRF_CS_PIN = 0)
#define MRF_CS_H()   (MRF_CS_PIN = 1)

// Opcional: si usas reset por hardware, define este pin
// #define MRF_RESET_PIN   LATA1
// #define MRF_RESET_HW()

bool MRF_Init(uint8_t channel);
void MRF_SetPan(uint16_t pan);
void MRF_SetShortAddress(uint16_t addr);
uint16_t MRF_GetPan(void);
uint16_t MRF_GetShortAddress(void);

bool MRF_Send(uint16_t dest_addr, uint16_t dest_pan, const uint8_t* data, uint8_t len);
bool MRF_IsTxPending(void);
bool MRF_TxSuccess(void);
uint8_t MRF_TxRetries(void);

// Las funciones de manejo de eventos ahora serán llamadas desde la ISR
void MRF_HandleInterrupt(void);
bool MRF_HasPacket(void);
void MRF_GetRx(uint8_t* buf, uint8_t* len, uint8_t* lqi, int8_t* rssi);

// Para uso interno
void MRF_EnableRx(void);

#endif
EOF

# -------------------- mrf24j40.c --------------------
cat > mrf24j40.c << 'EOF'
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "mrf24j40.h"
#include "spi.h"
#include "config.h"

// Registros (igual que antes)
#define REG_SOFTRST    0x2A
#define REG_PACON2     0x18
#define REG_TXSTBL     0x2E
#define REG_PANIDL     0x01
#define REG_PANIDH     0x02
#define REG_SADRL      0x03
#define REG_SADRH      0x04
#define REG_TXNCON     0x1B
#define REG_INTSTAT    0x31
#define REG_TXSTAT     0x24
#define REG_BBREG1     0x39
#define REG_RXFLUSH    0x0D
#define REG_BBREG2     0x3A
#define REG_CCAEDTH    0x3F
#define REG_BBREG6     0x3E
#define REG_INTCON     0x32
#define REG_RFCTL      0x36

#define LREG_RFCON0    0x200
#define LREG_RFCON1    0x201
#define LREG_RFCON2    0x202
#define LREG_RFCON3    0x203
#define LREG_RFCON6    0x206
#define LREG_RFCON7    0x207
#define LREG_RFCON8    0x208
#define LREG_SLPCON1   0x220

#define TXNFIFO        0x000
#define RXFIFO         0x300

#define INT_RXIF       0x08
#define INT_TXNIF      0x01

#define TXNTRIG        0x01
#define TXNACKREQ      0x04

#define FCF_LO         0x61
#define FCF_HI         0x88

static uint8_t tx_pending = 0;
static uint8_t tx_ok = 0;
static uint8_t tx_retries = 0;
static uint8_t tx_seq = 0;

static uint8_t rx_buffer[MAX_PAYLOAD];
static uint8_t rx_len = 0;
static uint8_t rx_lqi = 0;
static int8_t rx_rssi = 0;
static volatile uint8_t rx_ready = 0;

static uint8_t read_short(uint8_t addr);
static void write_short(uint8_t addr, uint8_t val);
static uint8_t read_long(uint16_t addr);
static void write_long(uint16_t addr, uint8_t val);
static void wait_reset(void);
static void flush_rx(void);
static void rf_reset(void);
static void handle_tx(void);
static void handle_rx(void);

// Implementaciones de lectura/escritura (igual que antes)
uint8_t read_short(uint8_t addr) {
    uint8_t cmd = (addr & 0x3F) << 1;
    MRF_CS_L();
    SPI_Transfer(cmd);
    uint8_t val = SPI_Transfer(0x00);
    MRF_CS_H();
    return val;
}

void write_short(uint8_t addr, uint8_t val) {
    uint8_t cmd = ((addr & 0x3F) << 1) | 0x01;
    MRF_CS_L();
    SPI_Transfer(cmd);
    SPI_Transfer(val);
    MRF_CS_H();
}

uint8_t read_long(uint16_t addr) {
    uint8_t cmd0 = 0x80 | ((addr >> 3) & 0x7F);
    uint8_t cmd1 = (addr & 0x07) << 5;
    MRF_CS_L();
    SPI_Transfer(cmd0);
    SPI_Transfer(cmd1);
    uint8_t val = SPI_Transfer(0x00);
    MRF_CS_H();
    return val;
}

void write_long(uint16_t addr, uint8_t val) {
    uint8_t cmd0 = 0x80 | ((addr >> 3) & 0x7F);
    uint8_t cmd1 = ((addr & 0x07) << 5) | 0x10;
    MRF_CS_L();
    SPI_Transfer(cmd0);
    SPI_Transfer(cmd1);
    SPI_Transfer(val);
    MRF_CS_H();
}

void wait_reset(void) {
    for (uint8_t i = 0; i < 200; i++) {
        if ((read_short(REG_SOFTRST) & 0x07) == 0x00) return;
        __delay_ms(1);
    }
}

void flush_rx(void) {
    write_short(REG_BBREG1, 0x04);
    write_short(REG_RXFLUSH, 0x01);
    __delay_ms(1);
    write_short(REG_BBREG1, 0x00);
}

void rf_reset(void) {
    write_short(REG_RFCTL, 0x04);
    __delay_ms(1);
    write_short(REG_RFCTL, 0x00);
    __delay_ms(1);
}

bool MRF_Init(uint8_t channel) {
    write_short(REG_SOFTRST, 0x07);
    __delay_ms(2);
    wait_reset();

    write_short(REG_PACON2, 0x98);
    write_short(REG_TXSTBL, 0x95);

    write_long(LREG_RFCON1, 0x02);
    write_long(LREG_RFCON2, 0x80);
    write_long(LREG_RFCON3, 0x00);
    write_long(LREG_RFCON6, 0x90);
    write_long(LREG_RFCON7, 0x80);
    write_long(LREG_RFCON8, 0x10);
    write_long(LREG_SLPCON1, 0x21);

    if (channel < 11 || channel > 26) return false;
    uint8_t val = ((channel - 11) << 4) | 0x03;
    write_long(LREG_RFCON0, val);
    rf_reset();

    write_short(REG_BBREG2, 0x80);
    write_short(REG_CCAEDTH, 0x60);
    write_short(REG_BBREG6, 0x40);
    write_short(REG_INTCON, 0xF6);   // Habilitar interrupciones TX/RX en el MRF24J40

    flush_rx();
    rf_reset();
    return true;
}

void MRF_SetPan(uint16_t pan) {
    write_short(REG_PANIDL, pan & 0xFF);
    write_short(REG_PANIDH, (pan >> 8) & 0xFF);
}

void MRF_SetShortAddress(uint16_t addr) {
    write_short(REG_SADRL, addr & 0xFF);
    write_short(REG_SADRH, (addr >> 8) & 0xFF);
}

uint16_t MRF_GetPan(void) {
    return (uint16_t)read_short(REG_PANIDL) | ((uint16_t)read_short(REG_PANIDH) << 8);
}

uint16_t MRF_GetShortAddress(void) {
    return (uint16_t)read_short(REG_SADRL) | ((uint16_t)read_short(REG_SADRH) << 8);
}

bool MRF_Send(uint16_t dest_addr, uint16_t dest_pan, const uint8_t* data, uint8_t len) {
    if (len > MAX_PAYLOAD) return false;
    if (tx_pending) return false;

    uint16_t src_addr = MRF_GetShortAddress();
    uint8_t hdr_len = 9;
    uint8_t frm_len = hdr_len + len;

    write_long(TXNFIFO + 0, hdr_len);
    write_long(TXNFIFO + 1, frm_len);
    write_long(TXNFIFO + 2, FCF_LO);
    write_long(TXNFIFO + 3, FCF_HI);
    write_long(TXNFIFO + 4, tx_seq++);
    write_long(TXNFIFO + 5, dest_pan & 0xFF);
    write_long(TXNFIFO + 6, (dest_pan >> 8) & 0xFF);
    write_long(TXNFIFO + 7, dest_addr & 0xFF);
    write_long(TXNFIFO + 8, (dest_addr >> 8) & 0xFF);
    write_long(TXNFIFO + 9, src_addr & 0xFF);
    write_long(TXNFIFO + 10, (src_addr >> 8) & 0xFF);

    for (uint8_t i = 0; i < len; i++) {
        write_long(TXNFIFO + 11 + i, data[i]);
    }

    tx_pending = 1;
    write_short(REG_TXNCON, TXNACKREQ | TXNTRIG);
    return true;
}

// Esta función será llamada desde la ISR
void MRF_HandleInterrupt(void) {
    uint8_t irq = read_short(REG_INTSTAT);
    if (irq & INT_TXNIF) handle_tx();
    if (irq & INT_RXIF) handle_rx();
    write_short(REG_INTSTAT, irq);
}

void handle_tx(void) {
    uint8_t txstat = read_short(REG_TXSTAT);
    tx_ok = !(txstat & 0x01);
    tx_retries = (txstat >> 6) & 0x03;
    tx_pending = 0;
}

void handle_rx(void) {
    write_short(REG_BBREG1, 0x04);
    uint8_t frame_len = read_long(RXFIFO + 0);
    const uint8_t MIN_FRAME = 12;
    if (frame_len < MIN_FRAME || frame_len > 127) {
        flush_rx();
        return;
    }
    const uint8_t HDR = 9;
    const uint8_t FCS = 2;
    int payload_len = frame_len - HDR - FCS;
    if (payload_len <= 0 || payload_len > MAX_PAYLOAD) {
        flush_rx();
        return;
    }
    rx_len = (uint8_t)payload_len;
    for (uint8_t i = 0; i < rx_len; i++) {
        rx_buffer[i] = read_long(RXFIFO + 1 + HDR + i);
    }
    rx_lqi = read_long(RXFIFO + 1 + frame_len);
    uint8_t raw_rssi = read_long(RXFIFO + 1 + frame_len + 1);
    rx_rssi = -90 + (raw_rssi / 3);
    rx_ready = 1;
    flush_rx();
    write_short(REG_BBREG1, 0x00);
}

bool MRF_HasPacket(void) {
    return rx_ready;
}

void MRF_GetRx(uint8_t* buf, uint8_t* len, uint8_t* lqi, int8_t* rssi) {
    if (buf && len && lqi && rssi) {
        memcpy(buf, rx_buffer, rx_len);
        *len = rx_len;
        *lqi = rx_lqi;
        *rssi = rx_rssi;
    }
    rx_ready = 0;
}

bool MRF_IsTxPending(void) {
    return tx_pending;
}

bool MRF_TxSuccess(void) {
    return tx_ok;
}

uint8_t MRF_TxRetries(void) {
    return tx_retries;
}
EOF

# -------------------- usart.h --------------------
cat > usart.h << 'EOF'
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
EOF

# -------------------- usart.c --------------------
cat > usart.c << 'EOF'
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
EOF

# -------------------- oled.h --------------------
cat > oled.h << 'EOF'
#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include <stdbool.h>

bool OLED_Init(void);
void OLED_Clear(void);
void OLED_Update(void);
void OLED_DrawChar(uint8_t x, uint8_t y, char c, uint8_t size, bool color);
void OLED_DrawString(uint8_t x, uint8_t y, const char* str, uint8_t size, bool color);
void OLED_DrawDec32(uint32_t val, uint8_t x, uint8_t y, uint8_t size);

#endif
EOF

# -------------------- oled.c --------------------
cat > oled.c << 'EOF'
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "oled.h"
#include "config.h"

#define OLED_ADDR       0x78
#define OLED_WIDTH      128
#define OLED_HEIGHT     32
#define OLED_PAGES      4

#define OLED_CMD_OFF             0xAE
#define OLED_CMD_ON              0xAF
#define OLED_CMD_SET_DISP_CLK    0xD5
#define OLED_CMD_SET_MULTIPLEX   0xA8
#define OLED_CMD_SET_OFFSET      0xD3
#define OLED_CMD_SET_START_LINE  0x40
#define OLED_CMD_CHARGE_PUMP     0x8D
#define OLED_CMD_MEM_MODE        0x20
#define OLED_CMD_SEG_REMAP       0xA1
#define OLED_CMD_COM_SCAN_DIR    0xC8
#define OLED_CMD_SET_COM_PINS    0xDA
#define OLED_CMD_SET_CONTRAST    0x81
#define OLED_CMD_SET_PRECHARGE   0xD9
#define OLED_CMD_SET_VCOM_DETECT 0xDB
#define OLED_CMD_DISPLAY_RESUME  0xA4
#define OLED_CMD_NORMAL_DISP     0xA6
#define OLED_CMD_DEACT_SCROLL    0x2E

#define SDA_TRIS        TRISBbits.TRISB0
#define SCL_TRIS        TRISBbits.TRISB1
#define SDA_LAT         LATBbits.LATB0
#define SCL_LAT         LATBbits.LATB1
#define SDA_PORT        PORTBbits.RB0

static void i2c_delay(void) {
    __delay_us(5);
}

static void i2c_start(void) {
    SDA_LAT = 1;
    SCL_LAT = 1;
    i2c_delay();
    SDA_LAT = 0;
    i2c_delay();
    SCL_LAT = 0;
    i2c_delay();
}

static void i2c_stop(void) {
    SDA_LAT = 0;
    SCL_LAT = 1;
    i2c_delay();
    SDA_LAT = 1;
    i2c_delay();
}

static uint8_t i2c_write_byte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        SDA_LAT = (data & 0x80) ? 1 : 0;
        data <<= 1;
        i2c_delay();
        SCL_LAT = 1;
        i2c_delay();
        SCL_LAT = 0;
        i2c_delay();
    }
    SDA_TRIS = 1;
    SCL_LAT = 1;
    i2c_delay();
    uint8_t ack = SDA_PORT;
    SCL_LAT = 0;
    SDA_TRIS = 0;
    i2c_delay();
    return ack;
}

static uint8_t oled_ok = 0;
static uint8_t buffer[OLED_WIDTH * OLED_PAGES];

static void oled_write_cmd(uint8_t cmd) {
    if (!oled_ok) return;
    i2c_start();
    if (i2c_write_byte(OLED_ADDR) != 0) { oled_ok = 0; i2c_stop(); return; }
    if (i2c_write_byte(0x00) != 0)      { oled_ok = 0; i2c_stop(); return; }
    if (i2c_write_byte(cmd) != 0)       { oled_ok = 0; i2c_stop(); return; }
    i2c_stop();
}

static void oled_write_data(uint8_t data) {
    if (!oled_ok) return;
    i2c_start();
    if (i2c_write_byte(OLED_ADDR) != 0) { oled_ok = 0; i2c_stop(); return; }
    if (i2c_write_byte(0x40) != 0)      { oled_ok = 0; i2c_stop(); return; }
    if (i2c_write_byte(data) != 0)      { oled_ok = 0; i2c_stop(); return; }
    i2c_stop();
}

bool OLED_Init(void) {
    SDA_TRIS = 0;
    SCL_TRIS = 0;
    SDA_LAT = 1;
    SCL_LAT = 1;
    __delay_ms(100);

    i2c_start();
    if (i2c_write_byte(OLED_ADDR) == 0) {
        oled_ok = 1;
    } else {
        oled_ok = 0;
        i2c_stop();
        return false;
    }
    i2c_stop();

    oled_write_cmd(OLED_CMD_OFF);
    oled_write_cmd(OLED_CMD_SET_DISP_CLK);   oled_write_cmd(0x80);
    oled_write_cmd(OLED_CMD_SET_MULTIPLEX);  oled_write_cmd(0x1F);
    oled_write_cmd(OLED_CMD_SET_OFFSET);     oled_write_cmd(0x00);
    oled_write_cmd(OLED_CMD_SET_START_LINE);
    oled_write_cmd(OLED_CMD_CHARGE_PUMP);    oled_write_cmd(0x14);
    oled_write_cmd(OLED_CMD_MEM_MODE);       oled_write_cmd(0x00);
    oled_write_cmd(OLED_CMD_SEG_REMAP);
    oled_write_cmd(OLED_CMD_COM_SCAN_DIR);
    oled_write_cmd(OLED_CMD_SET_COM_PINS);   oled_write_cmd(0x02);
    oled_write_cmd(OLED_CMD_SET_CONTRAST);   oled_write_cmd(0xCF);
    oled_write_cmd(OLED_CMD_SET_PRECHARGE);  oled_write_cmd(0xF1);
    oled_write_cmd(OLED_CMD_SET_VCOM_DETECT);oled_write_cmd(0x40);
    oled_write_cmd(OLED_CMD_DISPLAY_RESUME);
    oled_write_cmd(OLED_CMD_NORMAL_DISP);
    oled_write_cmd(OLED_CMD_DEACT_SCROLL);

    OLED_Clear();
    OLED_Update();
    oled_write_cmd(OLED_CMD_ON);
    return true;
}

void OLED_Clear(void) {
    memset(buffer, 0, sizeof(buffer));
}

void OLED_Update(void) {
    if (!oled_ok) return;
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        oled_write_cmd(0xB0 + page);
        oled_write_cmd(0x00);
        oled_write_cmd(0x10);
        i2c_start();
        if (i2c_write_byte(OLED_ADDR) != 0) { oled_ok = 0; i2c_stop(); return; }
        if (i2c_write_byte(0x40) != 0)      { oled_ok = 0; i2c_stop(); return; }
        for (uint8_t col = 0; col < OLED_WIDTH; col++) {
            if (i2c_write_byte(buffer[page * OLED_WIDTH + col]) != 0) {
                oled_ok = 0;
                i2c_stop();
                return;
            }
        }
        i2c_stop();
    }
}

static const uint8_t font[][5] = {
    {0x00,0x00,0x00,0x00,0x00},
    {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00}, {0x42,0x61,0x51,0x49,0x46},
    {0x21,0x41,0x45,0x4B,0x31}, {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03}, {0x36,0x49,0x49,0x49,0x36},
    {0x06,0x49,0x49,0x29,0x1E}, {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36},
    {0x3E,0x41,0x41,0x41,0x22}, {0x7F,0x41,0x41,0x22,0x1C}, {0x7F,0x49,0x49,0x49,0x41},
    {0x7F,0x09,0x09,0x09,0x01}, {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F},
    {0x00,0x41,0x7F,0x41,0x00}, {0x20,0x40,0x41,0x3F,0x01}, {0x7F,0x08,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40}, {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F},
    {0x3E,0x41,0x41,0x41,0x3E}, {0x7F,0x09,0x09,0x09,0x06}, {0x3E,0x41,0x51,0x21,0x5E},
    {0x7F,0x09,0x19,0x29,0x46}, {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01},
    {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F}, {0x7F,0x20,0x18,0x20,0x7F},
    {0x63,0x14,0x08,0x14,0x63}, {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
};

void OLED_DrawChar(uint8_t x, uint8_t y, char c, uint8_t size, bool color) {
    if (!oled_ok) return;
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    uint8_t idx = 0;
    if (c >= '0' && c <= '9')      idx = c - '0' + 1;
    else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 11;
    else if (c == ' ')             idx = 0;
    else return;
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t pattern = font[idx][col];
        for (uint8_t row = 0; row < 7; row++) {
            if (pattern & (1 << row)) {
                uint8_t px = x + col;
                uint8_t py = y + row;
                if (px < OLED_WIDTH && py < OLED_HEIGHT) {
                    uint8_t page = py / 8;
                    uint8_t bit = py % 8;
                    if (color)
                        buffer[page * OLED_WIDTH + px] |= (1 << bit);
                    else
                        buffer[page * OLED_WIDTH + px] &= ~(1 << bit);
                }
            }
        }
    }
}

void OLED_DrawString(uint8_t x, uint8_t y, const char* str, uint8_t size, bool color) {
    uint8_t current_x = x;
    while (*str && current_x < OLED_WIDTH) {
        OLED_DrawChar(current_x, y, *str, size, color);
        current_x += 6;
        str++;
    }
}

void OLED_DrawDec32(uint32_t val, uint8_t x, uint8_t y, uint8_t size) {
    char buf[12];
    uint8_t i = 10;
    buf[11] = 0;
    do {
        buf[i--] = '0' + (val % 10);
        val /= 10;
    } while (val && i);
    OLED_DrawString(x, y, &buf[i+1], size, true);
}
EOF

# -------------------- main.c (con interrupción INT1) --------------------
cat > main.c << 'EOF'
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
EOF

# -------------------- Makefile --------------------
cat > Makefile << 'EOF'
XC8 = xc8
CHIP = 18F4620
CFLAGS = --chip=$(CHIP) --opt=all
SOURCES = main.c mrf24j40.c spi.c usart.c oled.c

all: tx rx

tx: $(SOURCES)
	$(XC8) $(CFLAGS) -DMRF_MODE_TX $(SOURCES) -o firmware_tx.hex

rx: $(SOURCES)
	$(XC8) $(CFLAGS) -DMRF_MODE_RX $(SOURCES) -o firmware_rx.hex

clean:
	rm -f *.hex *.cof *.lst *.obj *.d *.p1 *.o

.PHONY: all clean tx rx
EOF

echo "Proyecto generado en: $PROYECT_DIR"
echo "Configuración: SPI con interrupción INT1 (RB1) para el MRF24J40"
echo "Prueba de comunicación SPI incluida al inicio."
echo "Compila con: make tx   o   make rx"
