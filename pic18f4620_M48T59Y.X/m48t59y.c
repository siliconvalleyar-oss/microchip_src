/**
 * @file m48t59y.c
 * @brief Driver para M48T559Y/M48T08 Timekeeper® SRAM con interfaz multiplexada
 *
 * Interface: AD0-AD7 multiplexed, AS0, AS1, E, R, W
 * PIC18F4620 @ 20MHz
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

void m48txx_init(void);
uint8_t m48txx_read(uint16_t address);
void m48txx_write(uint16_t address, uint8_t data);

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
