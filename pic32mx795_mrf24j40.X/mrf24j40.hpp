#pragma once
/**
 * mrf24j40.hpp  –  Driver MRF24J40 para PIC32MX795F512L
 *
 * Manejo completo del módulo IEEE 802.15.4 vía SPI4.
 * Interrupciones RX/TX por INT1 (RD8).
 * Soporte de roles: COORDINATOR, ROUTER, END_DEVICE (configurable en runtime).
 */

#include <xc.h>
#include <cstdint>
#include "spi4.hpp"

// ── Registros MRF24J40 ────────────────────────────────────────────────────────
#define MRF_RXMCR    0x00
#define MRF_PANIDL   0x01
#define MRF_PANIDH   0x02
#define MRF_SADRL    0x03
#define MRF_SADRH    0x04
#define MRF_EADR0    0x05
#define MRF_RXFLUSH  0x0D
#define MRF_ORDER    0x10
#define MRF_TXMCR    0x11
#define MRF_PACON2   0x18
#define MRF_TXBCON0  0x1A
#define MRF_TXNCON   0x1B
#define MRF_TXNTRIG  0
#define MRF_TXNACKREQ 2
#define MRF_TXSTAT   0x24
#define TXNSTAT      0
#define CCAFAIL      5
#define MRF_TXBCON1  0x25
#define MRF_SOFTRST  0x2A
#define MRF_SECCON0  0x2C
#define MRF_TXSTBL   0x2E
#define MRF_INTSTAT  0x31
#define MRF_INTCON   0x32
#define MRF_SLPACK   0x35
#define MRF_RFCTL    0x36
#define MRF_BBREG1   0x39
#define MRF_BBREG2   0x3A
#define MRF_BBREG6   0x3E
#define MRF_CCAEDTH  0x3F
#define MRF_RFCON0   0x200
#define MRF_RFCON1   0x201
#define MRF_RFCON2   0x202
#define MRF_RFCON3   0x203
#define MRF_RFCON6   0x206
#define MRF_RFCON7   0x207
#define MRF_RFCON8   0x208
#define MRF_RSSI     0x210
#define MRF_SLPCON1  0x220
#define MRF_TESTMODE 0x22F
#define MRF_I_RXIF   0x08
#define MRF_I_TXNIF  0x01

namespace DRV {

enum class NodeRole : uint8_t { COORDINATOR = 0, ROUTER = 1, END_DEVICE = 2 };

struct RxInfo {
    uint8_t frame_length;
    uint8_t rx_data[116];
    uint8_t lqi;
    uint8_t rssi;
};

struct TxInfo {
    uint8_t tx_ok     : 1;
    uint8_t retries   : 2;
    uint8_t channel_busy : 1;
};

class Mrf24j40 {
public:
    static void init(NodeRole role, uint16_t pan, uint16_t addr16, uint8_t channel = 15);

    // ── Rol ───────────────────────────────────
    static void set_role(NodeRole role);
    static NodeRole get_role();

    // ── Registros ─────────────────────────────
    static uint8_t  read_short(uint8_t addr);
    static void     write_short(uint8_t addr, uint8_t data);
    static uint8_t  read_long(uint16_t addr);
    static void     write_long(uint16_t addr, uint8_t data);

    // ── Red ───────────────────────────────────
    static void     set_pan(uint16_t pan);
    static uint16_t get_pan();
    static void     set_addr16(uint16_t addr);
    static uint16_t get_addr16();
    static void     set_channel(uint8_t ch);
    static void     set_long_addr(const uint8_t eui[8]);
    static void     set_txpower(uint8_t pwr);
    static uint8_t  read_rssi();
    static int8_t   rssi_dbm(uint8_t raw);

    // ── TX ────────────────────────────────────
    static bool send(uint16_t dest16, const uint8_t* data, uint8_t len);

    // ── RX ────────────────────────────────────
    static void rx_enable();
    static void rx_disable();
    static void rx_flush();

    // ── Sleep ─────────────────────────────────
    static void sleep();
    static void wakeup();

    // ── ISR (llamar desde la ISR de INT1) ─────
    static void isr_handler();

    // ── Poll (llamar en el loop si no hay IRQ) ─
    static void poll();

    // ── Callbacks (asignar antes de init) ──────
    static void (*on_rx_cb)();
    static void (*on_tx_cb)();

    // ── Acceso a datos RX/TX ──────────────────
    static RxInfo* get_rxinfo();
    static TxInfo* get_txinfo();
    static int     rx_datalength();

private:
    static void configure_interrupts();

    static RxInfo    _rx_info;
    static TxInfo    _tx_info;
    static uint8_t   _rx_buf[127];
    static NodeRole  _role;

    static volatile uint8_t _flag_rx;
    static volatile uint8_t _flag_tx;

    static constexpr int bytes_MHR    = 9;
    static constexpr int bytes_FCS    = 2;
    static constexpr int bytes_nodata = bytes_MHR + bytes_FCS;
};

} // namespace DRV
