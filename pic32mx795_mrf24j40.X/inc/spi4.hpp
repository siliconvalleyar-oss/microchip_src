#pragma once
/**
 * spi4.hpp  –  HAL SPI4 para PIC32MX795F512L
 *
 * SPI4 conectado al MRF24J40:
 *   SCK4  → RG6  (pin SCK)
 *   SDO4  → RG8  (pin MOSI)
 *   SDI4  → RG7  (pin MISO)
 *   CS    → RG9  (GPIO manual, activo LOW)
 *   INT   → RD8  (interrupción externa INT1, activo LOW)
 *   RST   → RD9  (reset hardware, activo LOW)
 *   WAKE  → RD10 (wake, activo HIGH)
 *
 * Configuración SPI:
 *   - Master mode
 *   - CPOL=0, CPHA=0 (Mode 0,0)
 *   - 8 bits por palabra
 *   - PBCLK = 80 MHz → SPI4BRG = 3 → ~10 MHz
 *   - MSB primero
 */

#include <xc.h>
#include <cstdint>

namespace HAL {

class Spi4 {
public:
    static void init();

    // ── Transferencia básica ───────────────────
    // Envía 1 byte, devuelve el byte recibido simultáneamente
    static uint8_t transfer(uint8_t data);

    // ── Transferencia de buffer ────────────────
    // tx_buf y rx_buf pueden ser el mismo puntero (in-place)
    // rx_buf puede ser nullptr si no se necesita la respuesta
    static void transfer_buf(const uint8_t* tx_buf,
                              uint8_t*       rx_buf,
                              uint16_t       len);

    // ── CS manual ─────────────────────────────
    static void cs_low();    // Selecciona esclavo
    static void cs_high();   // Deselecciona esclavo

    // ── Helpers registro MRF24J40 ─────────────
    // Short address (2 bytes)
    static uint8_t mrf_read_short(uint8_t addr);
    static void    mrf_write_short(uint8_t addr, uint8_t data);
    // Long address (3 bytes)
    static uint8_t mrf_read_long(uint16_t addr);
    static void    mrf_write_long(uint16_t addr, uint8_t data);

    // ── GPIO del módulo ───────────────────────
    static void mrf_reset();
    static void mrf_wake();
    static bool mrf_int_asserted();   // true = INT en LOW

private:
    static void wait_tx_empty();
    static void flush_rx();
};

} // namespace HAL
