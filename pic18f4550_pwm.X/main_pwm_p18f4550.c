#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 20000000

#pragma config FOSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF

uint8_t duty0=0;
uint8_t duty1=64;
uint8_t duty2=128;
uint8_t duty3=192;

void pwm_hw_init()
{
    TRISCbits.TRISC2 = 0;

    PR2 = 124;
    CCP1CON = 0b00001100;
    CCPR1L = 62;
    T2CON = 0b00000101;
}


void pwm_set(uint16_t duty)
{
    CCPR1L = duty >> 2;                  // 8 bits altos
    CCP1CONbits.DC1B = duty & 0x03;      // 2 bits bajos
} 

void main(void)
{
    TRISD = 0x00;
    LATD = 0x00;

    pwm_hw_init();

    uint8_t pwm_counter=0;
    uint8_t fade=0;
    int8_t dir=1;

    
    uint16_t duty = 0;


    while(1)
    {
        pwm_set(duty);

        duty += dir;

        if(duty >= 1023) dir = -1;
        if(duty == 0) dir = 1;

        __delay_ms(5);
    }
    
    
    while(1)
    {
        pwm_counter++;

        LATBbits.LATB0 = (pwm_counter < duty0);
        LATBbits.LATB1 = (pwm_counter < duty1);
        LATDbits.LATD2 = (pwm_counter < duty2);
        LATDbits.LATD3 = (pwm_counter < duty3);

        if(pwm_counter==0)
        {
            fade += dir;

            if(fade==255) dir=-1;
            if(fade==0) dir=1;

            duty0 = fade;
            duty1 = fade;
            duty2 = fade;
            duty3 = fade;
        }
    }
}