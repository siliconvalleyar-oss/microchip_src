/**
 * @file m48t59y.c
 * @brief Driver para M48T559Y/M48T08 Timekeeper® SRAM con interfaz multiplexada
 *
 * Interface: AD0-AD7 multiplexed, AS0, AS1, E, R, W
 * PIC18F4620 @ 20MHz
 *
 * Características:
 *  - Acceso a SRAM 8KB no volátil
 *  - RTC con formato BCD (año, mes, día, hora, minuto, segundo)
 *  - Watchdog timer
 *  - Alarmas
 *  - Flags de estado (batería baja, watchdog, alarma)
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

#define RTC_REG_FLAGS        0x1FF0
#define RTC_REG_ALARM_SEC    0x1FF2
#define RTC_REG_ALARM_MIN    0x1FF3
#define RTC_REG_ALARM_HOUR   0x1FF4
#define RTC_REG_ALARM_DATE   0x1FF5
#define RTC_REG_INTERRUPT    0x1FF6
#define RTC_REG_WATCHDOG     0x1FF7
#define RTC_REG_CONTROL      0x1FF8
#define RTC_REG_SECONDS      0x1FF9
#define RTC_REG_MINUTES      0x1FFA
#define RTC_REG_HOURS        0x1FFB
#define RTC_REG_DAY          0x1FFC
#define RTC_REG_DATE         0x1FFD
#define RTC_REG_MONTH        0x1FFE
#define RTC_REG_YEAR         0x1FFF

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
    AS0_TRIS = 0; AS0_LAT = 0;
    AS1_TRIS = 0; AS1_LAT = 0;
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

// ============================================================================
//  RTC FUNCTIONS
// ============================================================================

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

// ============================================================================
//  HIGH-LEVEL DATETIME API (BCD conversion built-in)
// ============================================================================

void m48txx_get_datetime(TK_DateTime *dt) {
    uint8_t ctrl;

    // Congelar actualizacion de los registros (bit READ = 1)
    ctrl = m48txx_read(RTC_REG_CONTROL);
    m48txx_write(RTC_REG_CONTROL, ctrl | CTRL_R_BIT);

    dt->seconds = bcd_to_bin(m48txx_read(RTC_REG_SECONDS) & 0x7F);
    dt->minutes = bcd_to_bin(m48txx_read(RTC_REG_MINUTES) & 0x7F);
    dt->hours   = bcd_to_bin(m48txx_read(RTC_REG_HOURS)   & 0x3F);
    dt->day     = bcd_to_bin(m48txx_read(RTC_REG_DAY)     & 0x07);
    dt->date    = bcd_to_bin(m48txx_read(RTC_REG_DATE)    & 0x3F);
    dt->month   = bcd_to_bin(m48txx_read(RTC_REG_MONTH)   & 0x1F);
    dt->year    = bcd_to_bin(m48txx_read(RTC_REG_YEAR));

    // Liberar el bit READ para que el reloj vuelva a actualizar
    ctrl = m48txx_read(RTC_REG_CONTROL);
    m48txx_write(RTC_REG_CONTROL, ctrl & (uint8_t)~CTRL_R_BIT);
}

void m48txx_set_datetime(const TK_DateTime *dt) {
    uint8_t ctrl;

    // Congelar registros para poder escribirlos (bit WRITE = 1)
    ctrl = m48txx_read(RTC_REG_CONTROL);
    m48txx_write(RTC_REG_CONTROL, ctrl | CTRL_W_BIT);

    m48txx_write(RTC_REG_SECONDS, bin_to_bcd(dt->seconds));
    m48txx_write(RTC_REG_MINUTES, bin_to_bcd(dt->minutes));
    m48txx_write(RTC_REG_HOURS,   bin_to_bcd(dt->hours));
    m48txx_write(RTC_REG_DAY,     bin_to_bcd(dt->day));
    m48txx_write(RTC_REG_DATE,    bin_to_bcd(dt->date));
    m48txx_write(RTC_REG_MONTH,   bin_to_bcd(dt->month));
    m48txx_write(RTC_REG_YEAR,    bin_to_bcd(dt->year));

    // Soltar el bit WRITE: transfiere los valores a los contadores reales
    ctrl = m48txx_read(RTC_REG_CONTROL);
    m48txx_write(RTC_REG_CONTROL, ctrl & (uint8_t)~CTRL_W_BIT);
}

// ============================================================================
//  BCD CONVERSION
// ============================================================================

uint8_t bin_to_bcd(uint8_t bin) {
    return ((bin / 10) << 4) | (bin % 10);
}

uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}
