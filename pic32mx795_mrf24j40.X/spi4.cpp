/**
 * spi4.cpp  –  HAL SPI4 PIC32MX795F512L
 *
 * Todos los accesos al MRF24J40 pasan por aquí.
 * Se construyen los bytes del protocolo del chip directamente,
 * sin usar memcpy de enteros (evita errores de byte-order).
 */

#include "spi4.hpp"
#include <plib.h>

// Bits de control en los registros de PIC32
// RG6=SCK4, RG7=SDI4, RG8=SDO4, RG9=CS (manual)
// RD8=INT,  RD9=RST,  RD10=WAKE

namespace HAL {

// ─────────────────────────────────────────────
//  init
// ─────────────────────────────────────────────
void Spi4::init() {
    // 1. Configurar pines de propósito general del SPI4
    TRISGbits.TRISG6 = 0;  // SCK4  salida
    TRISGbits.TRISG7 = 1;  // SDI4  entrada
    TRISGbits.TRISG8 = 0;  // SDO4  salida
    TRISGbits.TRISG9 = 0;  // CS    salida (manual)
    LATGbits.LATG9   = 1;  // CS    = HIGH (deselect)

    // 2. RST, WAKE, INT
    TRISDbits.TRISD9  = 0; // RST  salida
    TRISDbits.TRISD10 = 0; // WAKE salida
    TRISDbits.TRISD8  = 1; // INT  entrada
    LATDbits.LATD9    = 1; // RST  = HIGH (inactivo)
    LATDbits.LATD10   = 1; // WAKE = HIGH

    // 3. Configurar SPI4
    SPI4CON = 0;               // Limpiar
    SPI4BRG = 3;               // PBCLK(80MHz) / (2*(3+1)) = 10 MHz
    // SPI4CON: MSTEN=1, CKE=1, SMP=0, MODE16=0, MODE32=0, ON=1
    SPI4CONbits.MSTEN = 1;     // Master
    SPI4CONbits.CKE   = 1;     // Datos cambian en flanco caída SCK (CPHA=0)
    SPI4CONbits.SMP   = 0;     // Muestrea en el medio
    SPI4CONbits.MODE16= 0;     // 8 bits
    SPI4CONbits.MODE32= 0;
    SPI4CONbits.ON    = 1;     // Encender SPI4

    flush_rx();
}

// ─────────────────────────────────────────────
//  Transfer básico (1 byte)
// ─────────────────────────────────────────────
uint8_t Spi4::transfer(uint8_t data) {
    wait_tx_empty();
    SPI4BUF = data;
    // Esperar que el buffer de recepción esté lleno
    while (!SPI4STATbits.SPIRBF) ;
    return (uint8_t)SPI4BUF;
}

// ─────────────────────────────────────────────
//  Transfer de buffer
// ─────────────────────────────────────────────
void Spi4::transfer_buf(const uint8_t* tx, uint8_t* rx, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        uint8_t r = transfer(tx ? tx[i] : 0x00);
        if (rx) rx[i] = r;
    }
}

// ─────────────────────────────────────────────
//  CS
// ─────────────────────────────────────────────
void Spi4::cs_low()  { LATGbits.LATG9 = 0; }
void Spi4::cs_high() { LATGbits.LATG9 = 1; }

// ─────────────────────────────────────────────
//  MRF24J40 – Short Address (2 bytes)
//
//  Formato wire:
//    Byte 0: [0][A5..A0][R/W]   bit7=0 short, bits[6:1]=addr, bit0=0(R)/1(W)
//    Byte 1: dato (W) o dummy (R)
//  Respuesta (R): byte 1 del RX
// ─────────────────────────────────────────────
uint8_t Spi4::mrf_read_short(uint8_t addr) {
    uint8_t b0 = (uint8_t)((addr << 1) & 0x7E);  // bit7=0, bit0=0 (read)
    cs_low();
    transfer(b0);
    uint8_t ret = transfer(0x00);  // dummy
    cs_high();
    return ret;
}

void Spi4::mrf_write_short(uint8_t addr, uint8_t data) {
    uint8_t b0 = (uint8_t)(((addr << 1) & 0x7E) | 0x01);  // bit0=1 (write)
    cs_low();
    transfer(b0);
    transfer(data);
    cs_high();
}

// ─────────────────────────────────────────────
//  MRF24J40 – Long Address (3 bytes)
//
//  Formato wire:
//    Byte 0: [1][A10..A4]         bit7=1 (long), addr[10:4]
//    Byte 1: [A3..A1][R/W][0][0]  addr[3:1], bit4=0(R)/1(W)
//    Byte 2: dato (W) o dummy (R)
//  Respuesta (R): byte 2 del RX
// ─────────────────────────────────────────────
uint8_t Spi4::mrf_read_long(uint16_t addr) {
    uint8_t b0 = (uint8_t)(0x80 | ((addr >> 3) & 0x7F));
    uint8_t b1 = (uint8_t)((addr << 5) & 0xE0);   // bit4=0 → read
    cs_low();
    transfer(b0);
    transfer(b1);
    uint8_t ret = transfer(0x00);
    cs_high();
    return ret;
}

void Spi4::mrf_write_long(uint16_t addr, uint8_t data) {
    uint8_t b0 = (uint8_t)(0x80 | ((addr >> 3) & 0x7F));
    uint8_t b1 = (uint8_t)(((addr << 5) & 0xE0) | 0x10);  // bit4=1 → write
    cs_low();
    transfer(b0);
    transfer(b1);
    transfer(data);
    cs_high();
}

// ─────────────────────────────────────────────
//  GPIO del módulo MRF
// ─────────────────────────────────────────────
void Spi4::mrf_reset() {
    LATDbits.LATD9 = 0;
    // Delay ~1ms: a 80 MHz PBCLK
    for (volatile uint32_t i = 0; i < 80000UL; i++) ;
    LATDbits.LATD9 = 1;
    for (volatile uint32_t i = 0; i < 160000UL; i++) ;
}

void Spi4::mrf_wake() {
    LATDbits.LATD10 = 0;
    for (volatile uint32_t i = 0; i < 8000UL; i++) ;
    LATDbits.LATD10 = 1;
}

bool Spi4::mrf_int_asserted() {
    return PORTDbits.RD8 == 0;  // INT activo en LOW
}

// ─────────────────────────────────────────────
//  Privados
// ─────────────────────────────────────────────
void Spi4::wait_tx_empty() {
    while (SPI4STATbits.SPITBF) ;
}

void Spi4::flush_rx() {
    volatile uint32_t dummy = SPI4BUF;
    (void)dummy;
    SPI4STATbits.SPIROV = 0;
}

} // namespace HAL
