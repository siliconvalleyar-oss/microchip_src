#pragma once
/**
 * i2c.hpp  –  HAL I2C (I2C1) para PIC32MX795F512L
 *
 * Conectado al SSD1306 (OLED 128x32):
 *   SDA → RD0  (I2C1 SDA)
 *   SCL → RD10 (I2C1 SCL)
 *   Dirección I2C del SSD1306: 0x3C (SA0=GND)
 *
 * Frecuencia: 400 kHz (Fast Mode)
 * PBCLK = 80 MHz → I2C1BRG = 37 (ver fórmula datasheet PIC32)
 */

#include <xc.h>
#include <cstdint>

namespace HAL {

class I2c {
public:
    static void init();

    // ── Escritura al SSD1306 ───────────────────
    // Envía len bytes al dispositivo con dirección addr
    // Devuelve true si ACK recibido
    static bool write(uint8_t addr, const uint8_t* data, uint8_t len);

    // ── Primitivas ─────────────────────────────
    static bool start();
    static bool send_byte(uint8_t byte);
    static void stop();
    static void restart();

    // Dirección del SSD1306
    static constexpr uint8_t SSD1306_ADDR = 0x3C;

private:
    static bool wait_idle();
    static constexpr uint32_t TIMEOUT = 100000UL;
};

} // namespace HAL
