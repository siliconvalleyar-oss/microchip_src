/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef HARDWARE_CFG_H
#define	HARDWARE_CFG_H
#define _XTAL_FREQ 16000000    // 16MHz (4MHz interno * PLL)
//#define DISPLAY_128_X_64
#define DISPLAY_128_X_32
// Configuración del PIC18F46J50
#pragma config WDTEN = OFF      // Disable Watchdog Timer
#pragma config PLLDIV = 1       // No PLL prescaler
#pragma config STVREN = ON      // Stack overflow reset
#pragma config XINST = OFF      // Disable extended instruction set
#pragma config CP0 = OFF        // Code protection off
#pragma config OSC = INTOSCPLL  // Internal oscillator, PLL enabled
#pragma config T1DIG = ON       // Timer1 oscillator input
#pragma config LPT1OSC = OFF    // High power Timer1
#pragma config FCMEN = OFF      // Fail-safe clock monitor off
#pragma config IESO = OFF       // Internal/External switchover off
#pragma config WDTPS = 32768    // Watchdog timer postscaler

// Definiciones para el OLED SSD1306
#define OLED_ADDR    0x78    // Dirección I2C del OLED (0x3C << 1)
#define OLED_CMD     0x00    // Comando
#define OLED_DATA    0x40    // Datos

// Definiciones de pines para I2C por software
#define SCL_TRIS     TRISBbits.TRISB0
#define SDA_TRIS     TRISBbits.TRISB1

#define SCL_LAT      LATBbits.LATB0
#define SDA_LAT      LATBbits.LATB1

#define SCL_PORT     PORTBbits.RB0
#define SDA_PORT     PORTBbits.RB1

// Macros para control de pines I2C
#define SDA_OUTPUT() SDA_TRIS = 0
#define SDA_INPUT()  SDA_TRIS = 1
#define SCL_OUTPUT() SCL_TRIS = 0
#define SCL_INPUT()  SCL_TRIS = 1
#define SDA_HIGH()   SDA_LAT = 1
#define SDA_LOW()    SDA_LAT = 0
#define SCL_HIGH()   SCL_LAT = 1
#define SCL_LOW()    SCL_LAT = 0
#define SDA_READ()   SDA_PORT
#define SCL_READ()   SCL_PORT

// Definiciones para pulsadores
#define SW1_TRIS     TRISBbits.TRISB4
#define SW2_TRIS     TRISBbits.TRISB5
#define SW1_PORT     PORTBbits.RB4
#define SW2_PORT     PORTBbits.RB5

// Macros para pulsadores (asumiendo pulsadores pull-down)
// Si usas pull-up, cambia la lógica
#define SW1_PRESSED() (SW1_PORT == 1)
#define SW2_PRESSED() (SW2_PORT == 1)

#endif	/* XC_HEADER_TEMPLATE_H */

