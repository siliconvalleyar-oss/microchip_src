#pragma once
/**
 * uart.hpp  –  HAL UART2 para debug  (PIC32MX795F512L)
 *
 * UART2: 115200 bps, 8N1
 *   TX → RF5  (U2TX)
 *   RX → RF4  (U2RX)  – solo para referencia, no se usa en debug
 *
 * Proporciona formato de log tipo:
 *   [INFO]  mensaje
 *   [ERROR] mensaje
 *   [TX]    mensaje
 *   [RX]    mensaje
 *   [CRYPTO] mensaje
 */

#include <xc.h>
#include <cstdint>
#include <cstdarg>

namespace HAL {

enum class LogLevel : uint8_t {
    INFO   = 0,
    WARN   = 1,
    ERROR  = 2,
    TX     = 3,
    RX     = 4,
    CRYPTO = 5,
    NET    = 6,
    DEBUG  = 7,
};

class Uart {
public:
    static void init(uint32_t pbclk = 80000000UL, uint32_t baud = 115200UL);

    // ── Primitivas ─────────────────────────────
    static void put_char(char c);
    static void put_string(const char* s);
    static void put_hex_byte(uint8_t b);
    static void put_hex_buf(const uint8_t* buf, uint8_t len);
    static void put_uint16(uint16_t v);
    static void put_uint32(uint32_t v);
    static void newline();

    // ── Logger formateado ─────────────────────
    static void log(LogLevel lvl, const char* msg);
    static void logf(LogLevel lvl, const char* fmt, ...);  // mini printf

    // ── Helpers específicos ───────────────────
    static void log_tx_frame(uint16_t dest, const uint8_t* payload, uint8_t len);
    static void log_rx_frame(uint16_t src,  const uint8_t* payload, uint8_t len);
    static void log_crypto(bool ok, const char* detail);

private:
    static const char* level_str(LogLevel lvl);
    static void put_int(int32_t v);
};

} // namespace HAL
