/**
 * @file m48t59y.h
 * @brief Public API para driver M48T559Y/M48T08 Timekeeper® SRAM
 */

#ifndef M48T59Y_H
#define M48T59Y_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
//  FUNCIONES DE INICIALIZACION Y ACCESO BASICO
// ============================================================================

void m48txx_init(void);
uint8_t m48txx_read(uint16_t address);
void m48txx_write(uint16_t address, uint8_t data);

// ============================================================================
//  FUNCIONES DE SRAM NO VOLATIL
// ============================================================================

static inline uint8_t m48txx_read_sram(uint16_t address) {
    if (address > 0x1FF7) address = 0x1FF7;
    return m48txx_read(address);
}

static inline void m48txx_write_sram(uint16_t address, uint8_t data) {
    if (address > 0x1FF7) address = 0x1FF7;
    m48txx_write(address, data);
}

// ============================================================================
//  FUNCIONES DE RTC (REAL TIME CLOCK)
// ============================================================================

void m48txx_rtc_start(void);
void m48txx_rtc_stop(void);

void m48txx_rtc_set_time(uint8_t year, uint8_t month, uint8_t day,
                          uint8_t hour, uint8_t minute, uint8_t second);

void m48txx_rtc_get_time(uint8_t *year, uint8_t *month, uint8_t *day,
                          uint8_t *hour, uint8_t *minute, uint8_t *second);

// ============================================================================
//  CONVERSIONES BCD <-> BINARIO
// ============================================================================

uint8_t bin_to_bcd(uint8_t bin);
uint8_t bcd_to_bin(uint8_t bcd);

#endif /* M48T59Y_H */
