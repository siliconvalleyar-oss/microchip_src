/**
 * @file m48t59y.h
 * @brief Public API para driver M48T559Y/M48T08 Timekeeper® SRAM
 */

#ifndef M48T59Y_H
#define M48T59Y_H

#include <stdint.h>
#include <stdbool.h>

void m48txx_init(void);
uint8_t m48txx_read(uint16_t address);
void m48txx_write(uint16_t address, uint8_t data);

static inline uint8_t m48txx_read_sram(uint16_t address) {
    if (address > 0x1FF7) address = 0x1FF7;
    return m48txx_read(address);
}

static inline void m48txx_write_sram(uint16_t address, uint8_t data) {
    if (address > 0x1FF7) address = 0x1FF7;
    m48txx_write(address, data);
}

#endif /* M48T59Y_H */
