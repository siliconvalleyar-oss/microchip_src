/**
 * uart.cpp  –  HAL UART2 debug PIC32MX795F512L
 */

#include "uart.hpp"
#include <cstring>

namespace HAL {

void Uart::init(uint32_t pbclk, uint32_t baud) {
    // UART2: RF4=RX, RF5=TX
    TRISFbits.TRISF4 = 1;   // RX entrada
    TRISFbits.TRISF5 = 0;   // TX salida

    U2MODE = 0;
    U2STA  = 0;

    // BRG = PBCLK / (16 * baud) - 1
    U2BRG = (uint16_t)(pbclk / (16 * baud) - 1);

    U2MODEbits.UARTEN = 1;
    U2STAbits.UTXEN   = 1;
}

void Uart::put_char(char c) {
    while (U2STAbits.UTXBF) ;  // esperar espacio en TX FIFO
    U2TXREG = (uint8_t)c;
}

void Uart::put_string(const char* s) {
    while (*s) put_char(*s++);
}

void Uart::put_hex_byte(uint8_t b) {
    static const char hex[] = "0123456789ABCDEF";
    put_char(hex[b >> 4]);
    put_char(hex[b & 0x0F]);
}

void Uart::put_hex_buf(const uint8_t* buf, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        put_hex_byte(buf[i]);
        if (i < len - 1) put_char(' ');
    }
}

void Uart::put_uint16(uint16_t v) {
    put_string("0x");
    put_hex_byte((uint8_t)(v >> 8));
    put_hex_byte((uint8_t)(v & 0xFF));
}

void Uart::put_uint32(uint32_t v) {
    put_string("0x");
    put_hex_byte((uint8_t)(v >> 24));
    put_hex_byte((uint8_t)(v >> 16));
    put_hex_byte((uint8_t)(v >>  8));
    put_hex_byte((uint8_t)(v      ));
}

void Uart::newline() { put_char('\r'); put_char('\n'); }

// ─────────────────────────────────────────────
//  Logger
// ─────────────────────────────────────────────
const char* Uart::level_str(LogLevel lvl) {
    switch (lvl) {
        case LogLevel::INFO:   return "[INFO]  ";
        case LogLevel::WARN:   return "[WARN]  ";
        case LogLevel::ERROR:  return "[ERROR] ";
        case LogLevel::TX:     return "[TX]    ";
        case LogLevel::RX:     return "[RX]    ";
        case LogLevel::CRYPTO: return "[CRYPTO]";
        case LogLevel::NET:    return "[NET]   ";
        case LogLevel::DEBUG:  return "[DEBUG] ";
        default:               return "[?]     ";
    }
}

void Uart::log(LogLevel lvl, const char* msg) {
    put_string(level_str(lvl));
    put_char(' ');
    put_string(msg);
    newline();
}

// Mini-printf (soporta %s %d %u %x %02x)
void Uart::logf(LogLevel lvl, const char* fmt, ...) {
    put_string(level_str(lvl));
    put_char(' ');

    va_list args;
    va_start(args, fmt);
    const char* p = fmt;
    while (*p) {
        if (*p != '%') { put_char(*p++); continue; }
        p++;
        // Prefijo de ancho (ej: %02x)
        int width = 0;
        bool zero_pad = false;
        if (*p == '0') { zero_pad = true; p++; }
        while (*p >= '0' && *p <= '9') { width = width*10 + (*p++ - '0'); }

        switch (*p) {
            case 's': put_string(va_arg(args, const char*)); break;
            case 'd': put_int((int32_t)va_arg(args, int)); break;
            case 'u': {
                uint32_t u = va_arg(args, uint32_t);
                // Conversión simple sin sprintf
                char tmp[12]; int ti = 11; tmp[ti] = 0;
                do { tmp[--ti] = '0' + (u % 10); u /= 10; } while (u);
                put_string(&tmp[ti]);
                break;
            }
            case 'x': case 'X': {
                uint32_t u = va_arg(args, uint32_t);
                // Imprimir hex (hasta 8 dígitos)
                char tmp[9]; int ti = 8; tmp[ti] = 0;
                static const char H[] = "0123456789ABCDEF";
                do { tmp[--ti] = H[u & 0xF]; u >>= 4; } while (u);
                // zero-pad
                while (zero_pad && width > 0 && (8 - ti) < width) tmp[--ti] = '0';
                put_string(&tmp[ti]);
                break;
            }
            case '%': put_char('%'); break;
            default:  put_char(*p); break;
        }
        p++;
    }
    va_end(args);
    newline();
}

void Uart::put_int(int32_t v) {
    if (v < 0) { put_char('-'); v = -v; }
    char tmp[12]; int ti = 11; tmp[ti] = 0;
    if (v == 0) { put_char('0'); return; }
    uint32_t u = (uint32_t)v;
    do { tmp[--ti] = '0' + (u % 10); u /= 10; } while (u);
    put_string(&tmp[ti]);
}

void Uart::log_tx_frame(uint16_t dest, const uint8_t* payload, uint8_t len) {
    put_string("[TX]     DEST=");
    put_uint16(dest);
    put_string(" LEN=");
    char tmp[4]; int ti = 3; tmp[ti] = 0;
    uint8_t l = len;
    do { tmp[--ti] = '0' + (l % 10); l /= 10; } while (l && ti > 0);
    put_string(&tmp[ti]);
    put_string(" DATA=");
    put_hex_buf(payload, len);
    newline();
}

void Uart::log_rx_frame(uint16_t src, const uint8_t* payload, uint8_t len) {
    put_string("[RX]     SRC=");
    put_uint16(src);
    put_string(" LEN=");
    char tmp[4]; int ti = 3; tmp[ti] = 0;
    uint8_t l = len;
    do { tmp[--ti] = '0' + (l % 10); l /= 10; } while (l && ti > 0);
    put_string(&tmp[ti]);
    put_string(" DATA=");
    put_hex_buf(payload, len);
    newline();
}

void Uart::log_crypto(bool ok, const char* detail) {
    put_string(ok ? "[CRYPTO] OK  " : "[CRYPTO] ERR ");
    put_string(detail);
    newline();
}

} // namespace HAL
