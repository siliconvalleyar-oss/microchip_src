#pragma once
/**
 * ssd1306.hpp  –  Driver OLED SSD1306 128×32 I2C para PIC32MX795F512L
 *
 * Funciones de texto:
 *   - Fuente 5×7 (caracteres ASCII 32..127)
 *   - 4 líneas de 21 caracteres en 128×32
 *   - Actualización de líneas individuales (no re-dibuja todo)
 *
 * Muestra:
 *   Línea 0: Modo del nodo (COORDINATOR / ROUTER / END DEVICE)
 *   Línea 1: Estado de red (JOINED / SCANNING / ERROR)
 *   Línea 2: Último mensaje recibido (truncado a 21 chars)
 *   Línea 3: Estado cifrado (AES-OK / HMAC-ERR) + RSSI
 */

#include <cstdint>
#include <cstring>
#include "mrf24j40.hpp"   // Para NodeRole

namespace DRV {

class Ssd1306 {
public:
    static bool init();

    // ── Operaciones de pantalla ───────────────
    static void clear();
    static void display();           // Envía el framebuffer completo
    static void set_contrast(uint8_t c);

    // ── Texto (fuente 5×7, escalada a 8 para 32px de altura) ─────────────────
    static void set_cursor(uint8_t col, uint8_t row);   // col:0..20, row:0..3
    static void put_char(char c);
    static void put_string(const char* s, uint8_t max_chars = 21);
    static void put_line(uint8_t row, const char* s);   // Borra línea y escribe

    // ── Vistas de la aplicación ───────────────
    static void show_node_mode(NodeRole role);
    static void show_net_status(const char* status);
    static void show_last_msg(const char* msg);
    static void show_crypto_status(bool ok, int8_t rssi_dbm);
    static void show_all(NodeRole role, const char* net_status,
                         const char* last_msg, bool crypto_ok, int8_t rssi);

private:
    static void send_cmd(uint8_t cmd);
    static void send_data(const uint8_t* data, uint8_t len);

    static uint8_t  _buf[128 * 4];   // Framebuffer: 128 cols × 4 pages (32 filas)
    static uint8_t  _cursor_col;
    static uint8_t  _cursor_row;

    // Fuente 5×7 (solo caracteres 32..127 → 96 chars, 5 bytes cada uno)
    static const uint8_t FONT5x7[96][5];
};

} // namespace DRV
