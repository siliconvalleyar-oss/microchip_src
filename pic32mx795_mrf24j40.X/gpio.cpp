/**
 * gpio.cpp  –  HAL GPIO PIC32MX795F512L
 */

#include "gpio.hpp"
#include <plib.h>   // Peripheral Library Microchip

// Delay simple basado en ciclos (PBCLK = 80 MHz por defecto)
#define _NOP()   asm volatile("nop")
static void delay_ms_blocking(uint16_t ms) {
    // PBCLK 80 MHz → 80000 ciclos = 1 ms (aprox, sin optimización)
    for (uint16_t m = 0; m < ms; m++)
        for (uint32_t c = 0; c < 80000UL; c++) _NOP();
}

namespace HAL {

void Gpio::init() {
    // Puerto E → salidas (LEDs RE0..RE7)
    TRISE = 0x0000;
    LATE  = 0x0000;  // todos apagados

    // RA0 → salida (buzzer), inicialmente apagado
    TRISAbits.TRISA0 = 0;
    LATAbits.LATA0   = 0;

    // Deshabilitar pull-ups analógicos en PORTE si aplica
    // (PIC32MX795 no tiene ADC en RE, no es necesario)
}

// ─────────────────────────────────────────────
//  LEDs
// ─────────────────────────────────────────────
void Gpio::led_on(Led l) {
    LATE |=  (1u << static_cast<uint8_t>(l));
}

void Gpio::led_off(Led l) {
    LATE &= ~(1u << static_cast<uint8_t>(l));
}

void Gpio::led_toggle(Led l) {
    LATEINV = (1u << static_cast<uint8_t>(l));
}

void Gpio::leds_all_off() {
    LATE = 0x0000;
}

void Gpio::leds_set_byte(uint8_t pattern) {
    LATE = (uint32_t)pattern;
}

// ─────────────────────────────────────────────
//  Buzzer (RA0, activo HIGH)
// ─────────────────────────────────────────────
void Gpio::beep(uint16_t duration_ms) {
    LATAbits.LATA0 = 1;
    delay_ms_blocking(duration_ms);
    LATAbits.LATA0 = 0;
}

void Gpio::beep_async_start() { LATAbits.LATA0 = 1; }
void Gpio::beep_async_stop()  { LATAbits.LATA0 = 0; }

// ─────────────────────────────────────────────
//  GPIO genérico
// ─────────────────────────────────────────────
void Gpio::pin_out(volatile uint32_t* tris, uint8_t bit) { *tris &= ~(1u << bit); }
void Gpio::pin_in (volatile uint32_t* tris, uint8_t bit) { *tris |=  (1u << bit); }
void Gpio::pin_set(volatile uint32_t* lat,  uint8_t bit) { *lat  |=  (1u << bit); }
void Gpio::pin_clr(volatile uint32_t* lat,  uint8_t bit) { *lat  &= ~(1u << bit); }
void Gpio::pin_tog(volatile uint32_t* lat,  uint8_t bit) { *(lat + 3) = (1u << bit); } // LATINV
bool Gpio::pin_read(volatile uint32_t* port, uint8_t bit){ return (*port >> bit) & 1u; }

} // namespace HAL
