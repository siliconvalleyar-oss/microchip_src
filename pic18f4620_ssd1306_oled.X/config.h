#ifndef CONFIG_H
#define CONFIG_H

#include <xc.h>
#include <stdint.h>
#include <string.h>

#define _XTAL_FREQ   4000000UL      /* 4 MHz oscilador externo o interno */

/* ---- Pines I2C software --------------------------------- */
#define SCL_TRIS     TRISBbits.TRISB0
#define SDA_TRIS     TRISBbits.TRISB1
#define SCL_LAT      LATBbits.LATB0
#define SDA_LAT      LATBbits.LATB1
#define SCL_PORT     PORTBbits.RB0
#define SDA_PORT     PORTBbits.RB1

/* ---- OLED SSD1306 --------------------------------------- */
#define OLED_ADDR       0x3C          /* dirección 7 bits */
#define OLED_WIDTH      128
#define OLED_HEIGHT     32            /* 32 píxeles de alto */
#define OLED_PAGES      (OLED_HEIGHT / 8)  /* = 4 */

/* ---- Delays --------------------------------------------- */
#define DELAY_1MS()     __delay_ms(1)
#define DELAY_10MS()    __delay_ms(10)
#define DELAY_100MS()   __delay_ms(100)

#endif
