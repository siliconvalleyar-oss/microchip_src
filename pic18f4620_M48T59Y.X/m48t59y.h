/**
 * @file m48t59y.h
 * @brief Public API para driver M48T559Y/M48T08 Timekeeper® SRAM + RTC
 *
 * Soporta:
 *  - Acceso a SRAM no volátil (0x0000-0x1FEF)
 *  - Reloj de tiempo real (RTC) con formato BCD
 *  - Estructura TK_DateTime para manipulación de fecha/hora
 *  - Watchdog, alarmas y flags de estado
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

#define RTC_REG_CONTROL      0x1FF8
#define RTC_REG_SECONDS      0x1FF9
#define RTC_REG_MINUTES      0x1FFA
#define RTC_REG_HOURS        0x1FFB
#define RTC_REG_DAY          0x1FFC
#define RTC_REG_DATE         0x1FFD
#define RTC_REG_MONTH        0x1FFE
#define RTC_REG_YEAR         0x1FFF

#define CTRL_W_BIT       0x80
#define CTRL_R_BIT       0x40
#define SECONDS_ST_BIT   0x80
#define DAY_FT_BIT       0x40

typedef struct {
    uint8_t year;    /* 00-99 */
    uint8_t month;   /* 01-12 */
    uint8_t date;    /* 01-31 */
    uint8_t day;     /* 01-07 dia de la semana */
    uint8_t hours;   /* 00-23 */
    uint8_t minutes; /* 00-59 */
    uint8_t seconds; /* 00-59 */
} TK_DateTime;

void m48txx_rtc_start(void);
void m48txx_rtc_stop(void);
void m48txx_rtc_set_time(uint8_t year, uint8_t month, uint8_t day,
                          uint8_t hour, uint8_t minute, uint8_t second);
void m48txx_rtc_get_time(uint8_t *year, uint8_t *month, uint8_t *day,
                          uint8_t *hour, uint8_t *minute, uint8_t *second);
void m48txx_get_datetime(TK_DateTime *dt);
void m48txx_set_datetime(const TK_DateTime *dt);

uint8_t bin_to_bcd(uint8_t bin);
uint8_t bcd_to_bin(uint8_t bcd);

#endif /* M48T59Y_H */
