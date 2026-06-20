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
