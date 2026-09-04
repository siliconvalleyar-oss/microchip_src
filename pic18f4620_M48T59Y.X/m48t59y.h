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
//  REGISTROS DEL RTC (M48T559Y / M48T08)
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

// Bits del registro de Control (0x1FF8)
#define CTRL_W_BIT       0x80   /* WRITE  - detiene actualizacion para escribir */
#define CTRL_R_BIT       0x40   /* READ   - congela lectura */
#define CTRL_SIGN_BIT    0x20   /* signo de calibracion */

// Bit STOP en el registro de segundos (0x1FF9)
#define SECONDS_ST_BIT   0x80

// Bit FT en el registro de dia (0x1FFC)
#define DAY_FT_BIT       0x40

// Bits del registro de Flags (0x1FF0)
#define FLAG_WDF_BIT     0x80   /* Watchdog flag */
#define FLAG_AF_BIT      0x40   /* Alarm flag */
#define FLAG_BL_BIT      0x10   /* Battery low */

// ============================================================================
//  ESTRUCTURA DE FECHA/HORA
// ============================================================================

typedef struct {
    uint8_t year;    /* 00-99 */
    uint8_t month;   /* 01-12 */
    uint8_t date;    /* 01-31 */
    uint8_t day;     /* 01-07 dia de la semana */
    uint8_t hours;   /* 00-23 */
    uint8_t minutes; /* 00-59 */
    uint8_t seconds; /* 00-59 */
} TK_DateTime;

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
//  FUNCIONES DE ALTA NIVEL (DateTime)
// ============================================================================

void m48txx_get_datetime(TK_DateTime *dt);
void m48txx_set_datetime(const TK_DateTime *dt);

// ============================================================================
//  CONVERSIONES BCD <-> DECIMAL
// ============================================================================

uint8_t bin_to_bcd(uint8_t bin);
uint8_t bcd_to_bin(uint8_t bcd);

#endif /* M48T59Y_H */
