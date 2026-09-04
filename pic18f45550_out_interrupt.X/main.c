#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 20000000

#pragma config FOSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF

//================ ADC =================

void adc_init(void)
{
    TRISBbits.TRISB4 = 1;     // RB4 entrada AN11

    ADCON1 = 0b00001011;      // AN11 analógico
    ADCON2 = 0b10101010;      // justificado derecha

    ADCON0 = 0b00101101;      // canal AN11 + ADC ON
}

uint16_t adc_read(void)
{
    ADCON0bits.GO = 1;

    while(ADCON0bits.GO);

    return (((uint16_t)ADRESH << 8) | ADRESL);
}

//================ INTERRUPCION =================

void __interrupt() ISR(void)
{
    if(INTCONbits.INT0IF)
    {
        INTCONbits.INT0IF = 0;

        LATBbits.LATB1 = 1;
        __delay_us(20);
        LATBbits.LATB1 = 0;
    }
}

//================ MAIN =================

void main(void)
{
    // PORTB
    TRISBbits.TRISB0 = 1;   // INT0 entrada
    TRISBbits.TRISB1 = 0;   // salida pulso
    TRISBbits.TRISB4 = 1;   // analogico
    TRISBbits.TRISB5 = 1;   // pulsador

    // PORTD LEDs
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD3 = 0;

    LATB = 0;
    LATD = 0;

    // activar pullups PORTB
    INTCON2bits.RBPU = 0;

    // ADC
    adc_init();

    // INT0
    INTCON2bits.INTEDG0 = 1; // flanco subida
    INTCONbits.INT0IE = 1;

    // interrupciones globales
    INTCONbits.GIE = 1;

    uint16_t adc_value;

    while(1)
    {
        // pulsador RB5
        if(PORTBbits.RB5 == 0)
            LATDbits.LATD0 = 1;
        else
            LATDbits.LATD0 = 0;

        // lectura ADC
        adc_value = adc_read();

        if(adc_value < 10)
            LATDbits.LATD3 = 1;
        else
            LATDbits.LATD3 = 0;

        __delay_ms(10);
    }
}