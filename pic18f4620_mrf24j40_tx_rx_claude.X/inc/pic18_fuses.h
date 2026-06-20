/*
 * pic18_fuses.h  - Configuration bits PIC18F4620
 *
 * OSC externo HS 4 MHz
 * WDT OFF, LVP OFF, MCLR habilitado
 * Compilador XC8 - pragma config
 */
#ifndef PIC18_FUSES_H
#define PIC18_FUSES_H

// CONFIG1H
#pragma config OSC    = HS          // Oscilador externo HS
#pragma config FCMEN  = OFF
#pragma config IESO   = OFF

// CONFIG2L
#pragma config PWRT   = ON          // Power-up Timer ON
#pragma config BOREN  = SBORDIS
#pragma config BORV   = 3

// CONFIG2H
#pragma config WDT    = OFF         // Watchdog OFF
#pragma config WDTPS  = 32768

// CONFIG3H
#pragma config CCP2MX = PORTC       // CCP2 en RC1
#pragma config PBADEN = OFF         // PORTB digital al reset
#pragma config LPT1OSC = OFF
#pragma config MCLRE  = ON          // MCLR habilitado

// CONFIG4L
#pragma config STVREN = ON
#pragma config LVP    = OFF         // LVP deshabilitado
#pragma config XINST  = OFF         // Instrucciones extendidas OFF

// CONFIG5L / CONFIG5H / CONFIG6L / CONFIG6H / CONFIG7L / CONFIG7H
#pragma config CP0    = OFF
#pragma config CP1    = OFF
#pragma config CP2    = OFF
#pragma config CP3    = OFF
#pragma config CPB    = OFF
#pragma config CPD    = OFF
#pragma config WRT0   = OFF
#pragma config WRT1   = OFF
#pragma config WRT2   = OFF
#pragma config WRT3   = OFF
#pragma config WRTB   = OFF
#pragma config WRTC   = OFF
#pragma config WRTD   = OFF
#pragma config EBTR0  = OFF
#pragma config EBTR1  = OFF
#pragma config EBTR2  = OFF
#pragma config EBTR3  = OFF
#pragma config EBTRB  = OFF

#endif /* PIC18_FUSES_H */
