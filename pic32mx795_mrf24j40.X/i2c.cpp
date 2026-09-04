/**
 * i2c.cpp  –  HAL I2C1 PIC32MX795F512L
 */

#include "i2c.hpp"

namespace HAL {

void I2c::init() {
    I2C1CON  = 0;
    // I2C1BRG = (PBCLK / (2 * Fscl)) - 2   →  80M/(2*400k)-2 = 98  ≈ 100
    // Para 400kHz con PBCLK=80MHz:
    I2C1BRG  = 98;
    I2C1CONbits.ON = 1;
}

bool I2c::wait_idle() {
    uint32_t t = TIMEOUT;
    while ((I2C1CON & 0x1F) || I2C1STATbits.TRSTAT) {
        if (!t--) return false;
    }
    return true;
}

bool I2c::start() {
    if (!wait_idle()) return false;
    I2C1CONbits.SEN = 1;
    uint32_t t = TIMEOUT;
    while (I2C1CONbits.SEN) if (!t--) return false;
    return true;
}

bool I2c::send_byte(uint8_t byte) {
    if (!wait_idle()) return false;
    I2C1TRN = byte;
    uint32_t t = TIMEOUT;
    while (I2C1STATbits.TRSTAT) if (!t--) return false;
    return !I2C1STATbits.ACKSTAT;  // 0 = ACK
}

void I2c::stop() {
    wait_idle();
    I2C1CONbits.PEN = 1;
    uint32_t t = TIMEOUT;
    while (I2C1CONbits.PEN) if (!t--) break;
}

void I2c::restart() {
    wait_idle();
    I2C1CONbits.RSEN = 1;
    uint32_t t = TIMEOUT;
    while (I2C1CONbits.RSEN) if (!t--) break;
}

bool I2c::write(uint8_t addr, const uint8_t* data, uint8_t len) {
    if (!start()) return false;
    if (!send_byte((uint8_t)(addr << 1))) { stop(); return false; }
    for (uint8_t i = 0; i < len; i++) {
        if (!send_byte(data[i])) { stop(); return false; }
    }
    stop();
    return true;
}

} // namespace HAL
