#pragma once
/**
 * gpio.hpp  –  HAL GPIO para PIC32MX795F512L
 * Compilador: XC32 / MPLAB X
 *
 * Puertos usados:
 *   RE0..RE7  → LEDs de estado
 *   RA0       → Buzzer (beep activo LOW)
 *   RD0..RD3  → LEDs debug extra (opcional)
 */

#include <xc.h>
#include <cstdint>

namespace HAL {

// ─────────────────────────────────────────────
//  Pines de LEDs (Puerto E completo)
// ─────────────────────────────────────────────
enum class Led : uint8_t {
    NET_ACTIVITY  = 0,   // RE0 – actividad de red (RX/TX)
    NODE_STATUS   = 1,   // RE1 – estado del nodo (online/offline)
    COORDINATOR   = 2,   // RE2 – modo Coordinator activo
    ROUTER        = 3,   // RE3 – modo Router activo
    END_DEVICE    = 4,   // RE4 – modo End Device activo
    CRYPTO_OK     = 5,   // RE5 – cifrado OK (verde)
    CRYPTO_ERR    = 6,   // RE6 – error de cifrado/hash (rojo)
    TX_ACTIVE     = 7,   // RE7 – transmisión en curso
};

// ─────────────────────────────────────────────
//  Clase GPIO
// ─────────────────────────────────────────────
class Gpio {
public:
    static void init();

    // ── LEDs Puerto E ─────────────────────────
    static void led_on   (Led l);
    static void led_off  (Led l);
    static void led_toggle(Led l);
    static void leds_all_off();
    static void leds_set_byte(uint8_t pattern);  // Escribe byte directo en PORTE

    // ── Buzzer RA0 ────────────────────────────
    static void beep(uint16_t duration_ms);   // Beep bloqueante corto
    static void beep_async_start();
    static void beep_async_stop();

    // ── GPIO genérico ─────────────────────────
    static void pin_out(volatile uint32_t* tris, uint8_t bit);
    static void pin_in (volatile uint32_t* tris, uint8_t bit);
    static void pin_set(volatile uint32_t* lat,  uint8_t bit);
    static void pin_clr(volatile uint32_t* lat,  uint8_t bit);
    static void pin_tog(volatile uint32_t* lat,  uint8_t bit);
    static bool pin_read(volatile uint32_t* port, uint8_t bit);

private:
    static constexpr uint8_t BUZZER_BIT = 0;  // RA0
};

} // namespace HAL
