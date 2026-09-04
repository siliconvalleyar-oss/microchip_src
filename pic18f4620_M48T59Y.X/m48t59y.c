/**
 * @file m48t59y.c
 * @brief Driver para M48T559Y/M48T08 Timekeeper® SRAM con interfaz multiplexada
 *
 * Interface: AD0-AD7 multiplexed, AS0, AS1, E, R, W
 * PIC18F4620 @ 20MHz
 */

#include "m48t59y.h"
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
//  PIN DEFINITIONS - Modificar segun hardware
// ============================================================================

// Data Bus: PORTD (RD0-RD7)
#define DATA_PORT    PORTD
#define DATA_TRIS    TRISD
#define DATA_LAT     LATD

// AS0 (Address Strobe 0): RA6
#define AS0_LAT      LATAbits.LATA6
#define AS0_TRIS     TRISAbits.TRISA6

// AS1 (Address Strobe 1): RA7
#define AS1_LAT      LATAbits.LATA7
#define AS1_TRIS     TRISAbits.TRISA7

// E (Chip Enable, active low): RC0
#define E_LAT        LATCbits.LATC0
#define E_TRIS       TRISCbits.TRISC0

// R (Read Enable, active low): RC3
#define R_LAT        LATCbits.LATC3
#define R_TRIS       TRISCbits.TRISC3

// W (Write Enable, active low): RC4
#define W_LAT        LATCbits.LATC4
#define W_TRIS       TRISCbits.TRISC4

// ============================================================================
//  M48T559Y / M48T08 REGISTER MAP
// ============================================================================

#define RTC_REG_CONTROL    0x1FF8
#define RTC_REG_SECONDS    0x1FF9
#define RTC_REG_MINUTES    0x1FFA
#define RTC_REG_HOURS      0x1FFB
#define RTC_REG_DAY        0x1FFC
#define RTC_REG_DATE       0x1FFD
#define RTC_REG_MONTH      0x1FFE
#define RTC_REG_YEAR       0x1FFF

#define RTC_CTRL_WRITE_BIT 0x80
#define RTC_CTRL_READ_BIT  0x40
#define RTC_SEC_STOP_BIT   0x80

// ============================================================================
//  PRIVATE FUNCTIONS
// ============================================================================

static void data_bus_input(void) {
    DATA_TRIS = 0xFF;
}

static void data_bus_output(void) {
    DATA_TRIS = 0x00;
}

static void set_data(uint8_t val) {
    DATA_LAT = val;
}

static uint8_t get_data(void) {
    return DATA_PORT;
}

static void pulse_as0(void) {
    AS0_LAT = 0;
    NOP(); NOP(); NOP();
    AS0_LAT = 1;
    NOP(); NOP(); NOP();
}

static void pulse_as1(void) {
    AS1_LAT = 0;
    NOP(); NOP(); NOP();
    AS1_LAT = 1;
    NOP(); NOP(); NOP();
}

static uint8_t read_cycle(uint16_t address) {
    uint8_t data;

    // Address phase
    set_data((uint8_t)(address & 0x00FF));
    pulse_as0();  // latch A0-A7

    set_data((uint8_t)((address >> 8) & 0x1F));
    pulse_as1();  // latch A8-A12

    // Control phase (read)
    data_bus_input();
    E_LAT = 0;
    R_LAT = 0;
    W_LAT = 1;
    NOP(); NOP(); NOP();

    data = get_data();

    R_LAT = 1;
    NOP(); NOP();
    E_LAT = 1;

    data_bus_output();
    return data;
}

static void write_cycle(uint16_t address, uint8_t data) {
    // Address phase
    set_data((uint8_t)(address & 0x00FF));
    pulse_as0();

    set_data((uint8_t)((address >> 8) & 0x1F));
    pulse_as1();

    // Data phase
    set_data(data);
    data_bus_output();

    // Control phase (write)
    E_LAT = 0;
    R_LAT = 1;
    W_LAT = 0;
    NOP(); NOP(); NOP(); NOP();

    W_LAT = 1;
    NOP(); NOP();
    E_LAT = 1;
}

// ============================================================================
//  PUBLIC API
// ============================================================================

void m48txx_init(void) {
    // Control pins as outputs, idle high (disabled)
    AS0_TRIS = 0; AS0_LAT = 1;
    AS1_TRIS = 0; AS1_LAT = 1;
    E_TRIS   = 0; E_LAT   = 1;
    R_TRIS   = 0; R_LAT   = 1;
    W_TRIS   = 0; W_LAT   = 1;

    // Data bus as output initially
    data_bus_output();
    set_data(0x00);
}

uint8_t m48txx_read(uint16_t address) {
    return read_cycle(address);
}

void m48txx_write(uint16_t address, uint8_t data) {
    write_cycle(address, data);
}

void m48txx_rtc_start(void) {
    uint8_t sec = read_cycle(RTC_REG_SECONDS);
    sec &= ~RTC_SEC_STOP_BIT;
    write_cycle(RTC_REG_SECONDS, sec);
}

void m48txx_rtc_stop(void) {
    uint8_t sec = read_cycle(RTC_REG_SECONDS);
    sec |= RTC_SEC_STOP_BIT;
    write_cycle(RTC_REG_SECONDS, sec);
}

void m48txx_rtc_set_time(uint8_t year, uint8_t month, uint8_t day,
                          uint8_t hour, uint8_t minute, uint8_t second) {
    m48txx_rtc_stop();
    write_cycle(RTC_REG_YEAR,    year);
    write_cycle(RTC_REG_MONTH,   month);
    write_cycle(RTC_REG_DATE,    day);
    write_cycle(RTC_REG_HOURS,   hour);
    write_cycle(RTC_REG_MINUTES, minute);
    write_cycle(RTC_REG_SECONDS, second);
    m48txx_rtc_start();
}

void m48txx_rtc_get_time(uint8_t *year, uint8_t *month, uint8_t *day,
                          uint8_t *hour, uint8_t *minute, uint8_t *second) {
    *year   = read_cycle(RTC_REG_YEAR);
    *month  = read_cycle(RTC_REG_MONTH);
    *day    = read_cycle(RTC_REG_DATE);
    *hour   = read_cycle(RTC_REG_HOURS);
    *minute = read_cycle(RTC_REG_MINUTES);
    *second = read_cycle(RTC_REG_SECONDS);
}

uint8_t bin_to_bcd(uint8_t bin) {
    return ((bin / 10) << 4) | (bin % 10);
}

uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}
