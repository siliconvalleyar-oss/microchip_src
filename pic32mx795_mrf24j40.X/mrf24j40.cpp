/**
 * mrf24j40.cpp  –  Driver MRF24J40 PIC32MX795F512L
 *
 * ISR: INT1 (RD8, flanco descendente) → isr_handler()
 * Para registrar la ISR en MPLAB X agregar en main.cpp:
 *
 *   void __ISR(_EXTERNAL_1_VECTOR, IPL4AUTO) INT1_ISR() {
 *       DRV::Mrf24j40::isr_handler();
 *       IFS0CLR = _IFS0_INT1IF_MASK;
 *   }
 */

#include "mrf24j40.hpp"
#include "uart.hpp"

namespace DRV {

// ── Statics ───────────────────────────────────────────────────────────────────
RxInfo             Mrf24j40::_rx_info   {};
TxInfo             Mrf24j40::_tx_info   {};
uint8_t            Mrf24j40::_rx_buf[127]{};
NodeRole           Mrf24j40::_role      { NodeRole::END_DEVICE };
volatile uint8_t   Mrf24j40::_flag_rx   { 0 };
volatile uint8_t   Mrf24j40::_flag_tx   { 0 };
void             (*Mrf24j40::on_rx_cb)(){ nullptr };
void             (*Mrf24j40::on_tx_cb)(){ nullptr };

// ─────────────────────────────────────────────
//  init
// ─────────────────────────────────────────────
void Mrf24j40::init(NodeRole role, uint16_t pan, uint16_t addr16, uint8_t channel) {
    _role = role;

    HAL::Spi4::init();
    HAL::Spi4::mrf_reset();

    // Soft reset
    write_short(MRF_SOFTRST, 0x07);
    for (volatile uint32_t i = 0; i < 160000UL; i++) ;

    // Inicialización estándar (sección 3.2 del datasheet)
    write_short(MRF_PACON2,  0x98);
    write_short(MRF_TXSTBL,  0x95);
    write_long (MRF_RFCON0,  0x03);
    write_long (MRF_RFCON1,  0x01);
    write_long (MRF_RFCON2,  0x80);   // PLL ON
    write_long (MRF_RFCON6,  0x90);
    write_long (MRF_RFCON7,  0x80);
    write_long (MRF_RFCON8,  0x10);
    write_long (MRF_SLPCON1, 0x21);

    write_short(MRF_BBREG2,  0x80);   // CCA = ED
    write_short(MRF_CCAEDTH, 0x60);
    write_short(MRF_BBREG6,  0x40);   // Append RSSI

    set_pan(pan);
    set_addr16(addr16);
    set_channel(channel);
    configure_interrupts();

    write_short(MRF_RFCTL, 0x04);
    write_short(MRF_RFCTL, 0x00);
    for (volatile uint32_t i = 0; i < 16000UL; i++) ;

    // Configurar INT1 (RD8) en PIC32
    INTCONbits.INT1EP = 0;    // Flanco descendente
    IPC1bits.INT1IP   = 4;    // Prioridad 4
    IPC1bits.INT1IS   = 0;
    IFS0CLR = _IFS0_INT1IF_MASK;
    IEC0SET = _IEC0_INT1IE_MASK;

    HAL::Uart::logf(HAL::LogLevel::NET,
        "MRF24J40 init OK – Role=%d PAN=0x%04x ADDR=0x%04x CH=%d",
        (int)role, pan, addr16, channel);
}

void Mrf24j40::configure_interrupts() {
    // Habilitar solo RX e TXNIF; enmascarar el resto
    write_short(MRF_INTCON, 0b11110110);
}

// ─────────────────────────────────────────────
//  Rol
// ─────────────────────────────────────────────
void       Mrf24j40::set_role(NodeRole r) { _role = r; }
NodeRole   Mrf24j40::get_role()           { return _role; }

// ─────────────────────────────────────────────
//  Registros (delegan en SPI4)
// ─────────────────────────────────────────────
uint8_t Mrf24j40::read_short(uint8_t a)         { return HAL::Spi4::mrf_read_short(a); }
void    Mrf24j40::write_short(uint8_t a, uint8_t d){ HAL::Spi4::mrf_write_short(a, d); }
uint8_t Mrf24j40::read_long(uint16_t a)          { return HAL::Spi4::mrf_read_long(a); }
void    Mrf24j40::write_long(uint16_t a, uint8_t d){ HAL::Spi4::mrf_write_long(a, d); }

// ─────────────────────────────────────────────
//  Configuración de red
// ─────────────────────────────────────────────
void     Mrf24j40::set_pan(uint16_t p)  { write_short(MRF_PANIDH, p>>8); write_short(MRF_PANIDL, p&0xFF); }
uint16_t Mrf24j40::get_pan()            { return (uint16_t)((read_short(MRF_PANIDH)<<8)|read_short(MRF_PANIDL)); }
void     Mrf24j40::set_addr16(uint16_t a){ write_short(MRF_SADRH, a>>8); write_short(MRF_SADRL, a&0xFF); }
uint16_t Mrf24j40::get_addr16()         { return (uint16_t)((read_short(MRF_SADRH)<<8)|read_short(MRF_SADRL)); }

void Mrf24j40::set_channel(uint8_t ch) {
    if (ch < 11 || ch > 26) ch = 11;
    write_long(MRF_RFCON0, (uint8_t)(((ch - 11) << 4) | 0x03));
}

void Mrf24j40::set_long_addr(const uint8_t eui[8]) {
    for (int i = 0; i < 8; i++) write_short(MRF_EADR0 + i, eui[i]);
}

void Mrf24j40::set_txpower(uint8_t pwr) { write_long(MRF_RFCON3, pwr & 0xF8); }

uint8_t Mrf24j40::read_rssi()           { return read_long(MRF_RSSI); }
int8_t  Mrf24j40::rssi_dbm(uint8_t raw){ return (int8_t)(-90 + (raw >> 1)); }

// ─────────────────────────────────────────────
//  Transmisión  (cabecera MAC 802.15.4 + payload)
// ─────────────────────────────────────────────
bool Mrf24j40::send(uint16_t dest16, const uint8_t* data, uint8_t len) {
    if (len > 100) len = 100;
    int i = 0;

    write_long(i++, (uint8_t)bytes_MHR);
    write_long(i++, (uint8_t)(bytes_MHR + len));

    write_long(i++, 0b01100001);   // FC byte1: PAN compress | data frame
    write_long(i++, 0b10001000);   // FC byte2: 16-bit addr
    write_long(i++, 1);            // seq

    uint16_t pan = get_pan();
    write_long(i++, (uint8_t)(pan & 0xFF));
    write_long(i++, (uint8_t)(pan >> 8));
    write_long(i++, (uint8_t)(dest16 & 0xFF));
    write_long(i++, (uint8_t)(dest16 >> 8));

    uint16_t src = get_addr16();
    write_long(i++, (uint8_t)(src & 0xFF));
    write_long(i++, (uint8_t)(src >> 8));

    for (int q = 0; q < len; q++) write_long(i++, data[q]);

    // Disparo TX con ACK MAC
    write_short(MRF_TXNCON, (1 << MRF_TXNACKREQ) | (1 << MRF_TXNTRIG));
    return true;
}

// ─────────────────────────────────────────────
//  RX
// ─────────────────────────────────────────────
void Mrf24j40::rx_enable()  { write_short(MRF_BBREG1, 0x00); }
void Mrf24j40::rx_disable() { write_short(MRF_BBREG1, 0x04); }
void Mrf24j40::rx_flush()   { write_short(MRF_RXFLUSH, 0x01); }

// ─────────────────────────────────────────────
//  Sleep / Wake
// ─────────────────────────────────────────────
void Mrf24j40::sleep() {
    uint8_t v = read_short(MRF_SLPACK);
    write_short(MRF_SLPACK, v | 0x80);
}

void Mrf24j40::wakeup() {
    HAL::Spi4::mrf_wake();
    write_short(MRF_RFCTL, 0x04);
    write_short(MRF_RFCTL, 0x00);
    for (volatile uint32_t i = 0; i < 16000UL; i++) ;
}

// ─────────────────────────────────────────────
//  ISR handler (llamado desde el vector INT1)
// ─────────────────────────────────────────────
void Mrf24j40::isr_handler() {
    uint8_t irq = read_short(MRF_INTSTAT);

    if (irq & MRF_I_RXIF) {
        _flag_rx++;
        rx_disable();

        uint8_t flen = read_long(0x300);
        int dlen = flen - bytes_nodata;
        if (dlen < 0) dlen = 0;
        if (dlen > 116) dlen = 116;

        for (int j = 0; j < dlen; j++)
            _rx_info.rx_data[j] = read_long(0x301 + bytes_MHR + j);

        _rx_info.frame_length = flen;
        _rx_info.lqi          = read_long(0x301 + flen);
        _rx_info.rssi         = read_long(0x301 + flen + 1);

        rx_enable();
    }

    if (irq & MRF_I_TXNIF) {
        _flag_tx++;
        uint8_t stat         = read_short(MRF_TXSTAT);
        _tx_info.tx_ok       = !(stat & (1 << TXNSTAT));
        _tx_info.retries     = stat >> 6;
        _tx_info.channel_busy= (stat >> CCAFAIL) & 1;
    }
}

// ─────────────────────────────────────────────
//  Poll (despacha callbacks pendientes)
// ─────────────────────────────────────────────
void Mrf24j40::poll() {
    // También verificar pin INT por polling (fallback)
    if (HAL::Spi4::mrf_int_asserted()) isr_handler();

    if (_flag_rx && on_rx_cb) { _flag_rx = 0; on_rx_cb(); }
    if (_flag_tx && on_tx_cb) { _flag_tx = 0; on_tx_cb(); }
}

// ─────────────────────────────────────────────
//  Accesores
// ─────────────────────────────────────────────
RxInfo* Mrf24j40::get_rxinfo() { return &_rx_info; }
TxInfo* Mrf24j40::get_txinfo() { return &_tx_info; }
int     Mrf24j40::rx_datalength() {
    int d = _rx_info.frame_length - bytes_nodata;
    return d < 0 ? 0 : d;
}

} // namespace DRV
