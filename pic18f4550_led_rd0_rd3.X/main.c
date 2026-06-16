#include <xc.h>
#include <stdint.h>
#include <string.h>

#define _XTAL_FREQ 4000000

// Solo las configuraciones esenciales
#pragma config FOSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config XINST = OFF

extern const uint8_t logo[];
extern unsigned int logo_bin_len;

void main(void) {
    ADCON1 = 0x0F;

TRISA=0x00;
TRISB=0x00;
TRISC=0x00;
TRISD=0x00;
TRISE=0x00;
LATA=0x00;
LATB=0x00;
LATC=0x00; 
LATD=0x00;
LATE=0x00;
    
    while(1) {
        LATD = 0b00000001; __delay_ms(5);
        LATD = 0b00000010; __delay_ms(5);
        LATD = 0b00000100; __delay_ms(5);
        LATD = 0b00001000; __delay_ms(5);
        LATB = 0b00000001; __delay_ms(5);
        LATB = 0b00000010; __delay_ms(5);
        LATB = 0b00000000; __delay_ms(5);
    }
}