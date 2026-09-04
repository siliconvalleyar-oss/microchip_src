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

#define DATA_PORT    PORTD
#define DATA_TRIS    TRISD
#define DATA_LAT     LATD

#define AS0_LAT      LATAbits.LATA6
#define AS0_TRIS     TRISAbits.TRISA6

#define AS1_LAT      LATAbits.LATA7
#define AS1_TRIS     TRISAbits.TRISA7

#define E_LAT        LATCbits.LATC0
#define E_TRIS       TRISCbits.TRISC0

#define R_LAT        LATCbits.LATC3
#define R_TRIS       TRISCbits.TRISC3

#define W_LAT        LATCbits.LATC4
#define W_TRIS       TRISCbits.TRISC4

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
    AS0_LAT = 0;
}

static void pulse_as1(void) {
    AS1_LAT = 0;
    NOP(); NOP(); NOP();
    AS1_LAT = 1;
    NOP(); NOP(); NOP();
    AS1_LAT = 0;
}

static uint8_t read_cycle(uint16_t address) {
    uint8_t data;

    set_data((uint8_t)(address & 0x00FF));
    pulse_as0();

    set_data((uint8_t)((address >> 8) & 0x1F));
    pulse_as1();

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
    set_data((uint8_t)(address & 0x00FF));
    pulse_as0();

    set_data((uint8_t)((address >> 8) & 0x1F));
    pulse_as1();

    set_data(data);
    data_bus_output();

    E_LAT = 0;
    R_LAT = 1;
    W_LAT = 0;
    NOP(); NOP(); NOP(); NOP();

    W_LAT = 1;
    NOP(); NOP();
    E_LAT = 1;
}

void m48txx_init(void) {
    AS0_TRIS = 0; AS0_LAT = 1;
    AS1_TRIS = 0; AS1_LAT = 1;
    E_TRIS   = 0; E_LAT   = 1;
    R_TRIS   = 0; R_LAT   = 1;
    W_TRIS   = 0; W_LAT   = 1;

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
    sec &= ~SECONDS_ST_BIT;
    write_cycle(RTC_REG_SECONDS, sec);
}

void m48txx_rtc_stop(void) {
    uint8_t sec = read_cycle(RTC_REG_SECONDS);
    sec |= SECONDS_ST_BIT;
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

void m48txx_get_datetime(TK_DateTime *dt) {
    uint8_t ctrl;

    ctrl = m48txx_read(RTC_REG_CONTROL);
    m48txx_write(RTC_REG_CONTROL, (uint8_t)(ctrl | CTRL_R_BIT));

    dt->seconds = bcd_to_bin(m48txx_read(RTC_REG_SECONDS) & 0x7F);
    dt->minutes = bcd_to_bin(m48txx_read(RTC_REG_MINUTES) & 0x7F);
    dt->hours   = bcd_to_bin(m48txx_read(RTC_REG_HOURS)   & 0x3F);
    dt->day     = bcd_to_bin(m48txx_read(RTC_REG_DAY)     & 0x07);
    dt->date    = bcd_to_bin(m48txx_read(RTC_REG_DATE)    & 0x3F);
    dt->month   = bcd_to_bin(m48txx_read(RTC_REG_MONTH)   & 0x1F);
    dt->year    = bcd_to_bin(m48txx_read(RTC_REG_YEAR));

    ctrl = m48txx_read(RTC_REG_CONTROL);
    m48txx_write(RTC_REG_CONTROL, (uint8_t)(ctrl & (uint8_t)~CTRL_R_BIT));
}

void m48txx_set_datetime(const TK_DateTime *dt) {
    uint8_t ctrl;

    ctrl = m48txx_read(RTC_REG_CONTROL);
    m48txx_write(RTC_REG_CONTROL, (uint8_t)(ctrl | CTRL_W_BIT));

    m48txx_write(RTC_REG_SECONDS, bin_to_bcd(dt->seconds));
    m48txx_write(RTC_REG_MINUTES, bin_to_bcd(dt->minutes));
    m48txx_write(RTC_REG_HOURS,   bin_to_bcd(dt->hours));
    m48txx_write(RTC_REG_DAY,     bin_to_bcd(dt->day));
    m48txx_write(RTC_REG_DATE,    bin_to_bcd(dt->date));
    m48txx_write(RTC_REG_MONTH,   bin_to_bcd(dt->month));
    m48txx_write(RTC_REG_YEAR,    bin_to_bcd(dt->year));

    ctrl = m48txx_read(RTC_REG_CONTROL);
    m48txx_write(RTC_REG_CONTROL, (uint8_t)(ctrl & (uint8_t)~CTRL_W_BIT));
}

uint8_t bin_to_bcd(uint8_t bin) {
    return (uint8_t)(((bin / 10) << 4) | (bin % 10));
}

uint8_t bcd_to_bin(uint8_t bcd) {
    return (uint8_t)(((bcd >> 4) * 10) + (bcd & 0x0F));
}
