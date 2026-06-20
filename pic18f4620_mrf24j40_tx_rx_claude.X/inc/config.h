/*
 * config.h  - Configuracion central del proyecto
 * PIC18F4620 + MRF24J40 + OLED SSD1306
 * Compilador: XC8
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <xc.h>
#include <stdint.h>
#include <string.h>

/* ---- Oscilador ------------------------------------------ */
#define _XTAL_FREQ   4000000UL      /* 4 MHz oscilador externo */

/* ---- USART ---------------------------------------------- */
/*
 * Baud rate 9600 @ Fosc=4MHz
 * SPBRG = (Fosc / (16 * Baud)) - 1  con BRGH=1
 *       = (4000000 / (16 * 9600)) - 1 = 25.04 ~ 25
 * Error: ((26*16*9600)/4000000 - 1)*100 = 0.16%
 */
#define SPBRG_VAL    25//25   //13          /* BRGH=1, BRG16=0 */

/* Definir ROL del dispositivo:
 * Exactamente UNO de los dos debe estar activo.
 * TX: el PIC transmite datos al host (Raspberry/PC)
 * RX: el PIC recibe datos del host
 * En modo TX tambien puede recibir (USART full-duplex),
 * este define controla el comportamiento del loop principal.
 */
#define MODO_TX                     /* <-- cambiar a MODO_RX para receptor */
/* #define MODO_RX */

/* ---- Pines SPI (hardware MSSP) -------------------------- */
#define MRF_CS_TRIS  TRISCbits.TRISC0
#define MRF_CS_LAT   LATCbits.LATC0
/* RC3=SCK, RC4=SDI(MISO), RC5=SDO(MOSI)  <- hardware SPI */

/* ---- Pines I2C software (OLED SSD1306) ------------------ */
#define SCL_TRIS     TRISBbits.TRISB0
#define SDA_TRIS     TRISBbits.TRISB1
#define SCL_LAT      LATBbits.LATB0
#define SDA_LAT      LATBbits.LATB1
#define SCL_PORT     PORTBbits.RB0
#define SDA_PORT     PORTBbits.RB1


/* ---- Pines LEDs ----------------------------------------- */
#define LED0_TRIS    TRISDbits.TRISD0
#define LED1_TRIS    TRISDbits.TRISD1
#define LED2_TRIS    TRISDbits.TRISD2
#define LED3_TRIS    TRISDbits.TRISD3
#define LED0         LATDbits.LATD0
#define LED1         LATDbits.LATD1
#define LED2         LATDbits.LATD2
#define LED3         LATDbits.LATD3

/* ---- Pines Pulsadores (activo bajo, pull-up interno) ---- */
#define BTN0_TRIS    TRISBbits.TRISB4
#define BTN1_TRIS    TRISBbits.TRISB5
#define BTN0_PORT    PORTBbits.RB4
#define BTN1_PORT    PORTBbits.RB5

/* ---- Red IEEE 802.15.4 ---------------------------------- */
#define MRF_CHANNEL     20
#define MRF_PAN_ID      0xCAFE
#define MRF_MY_ADDR     0x0001      /* Direccion de este PIC  */
#define MRF_DEST_ADDR   0x0002      /* Destino: Raspberry A   */
#define MRF_ALT_ADDR    0x0003      /* Destino: Raspberry B   */

/* Maxima longitud de payload util */
#define MRF_MAX_PAYLOAD  50



/* ---- OLED SSD1306 --------------------------------------- */
#define OLED_ADDR       0x3C          /* dirección 7 bits */
#define OLED_WIDTH      128
#define OLED_HEIGHT     32            /* 32 píxeles de alto */
#define OLED_PAGES      (OLED_HEIGHT / 8)  /* = 4 */


/* ---- Delays de conveniencia ----------------------------- */
#define DELAY_1MS()     __delay_ms(1)
#define DELAY_10MS()    __delay_ms(10)
#define DELAY_100MS()   __delay_ms(100)

#endif /* CONFIG_H */
