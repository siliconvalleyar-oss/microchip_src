#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 20000000

#pragma config FOSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF

void pwm_cycle(uint8_t duty)
{
    for(uint8_t i = 0; i < 100; i++)
    {
        if(i < duty)
        {
            LATBbits.LATB1 = 1;
            LATDbits.LATD0 = 1;
        }
        else
        {
            LATBbits.LATB1 = 0;
            LATDbits.LATD0 = 0;
        }

        __delay_us(100);
    }
}

void main(void)
{
    ADCON1 = 0x0F;

    TRISBbits.TRISB1 = 0;
    TRISDbits.TRISD0 = 0;

    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB5 = 1;

    LATB = 0;
    LATD = 0;

    uint8_t mode = 0;
    uint8_t duty = 0;
    uint8_t up = 1;

    while(1)
    {
        // ===== BOTONES =====

        if(PORTBbits.RB4 == 0)
        {
            __delay_ms(20);
            if(PORTBbits.RB4 == 0)
            {
                mode++;
                if(mode > 6) mode = 0;

                while(PORTBbits.RB4 == 0);
                __delay_ms(20);
            }
        }

        if(PORTBbits.RB5 == 0)
        {
            __delay_ms(20);
            if(PORTBbits.RB5 == 0)
            {
                if(mode == 0) mode = 6;
                else mode--;

                while(PORTBbits.RB5 == 0);
                __delay_ms(20);
            }
        }

        // ===== MODOS =====

        switch(mode)
        {
            // ? 0: PWM completo
            case 0:
                pwm_cycle(duty);

                if(up)
                {
                    duty++;
                    if(duty >= 100) up = 0;
                }
                else
                {
                    duty--;
                    if(duty == 0) up = 1;
                }
            break;

            // ? 1: Parpadeo
            case 1:
                LATBbits.LATB1 = 1;
                LATDbits.LATD0 = 1;
                __delay_ms(1000);

                LATBbits.LATB1 = 0;
                LATDbits.LATD0 = 0;
                __delay_ms(1000);
            break;

            // ? 2: 100%
            case 2:
                LATBbits.LATB1 = 1;
                LATDbits.LATD0 = 1;
            break;

            // ? 3: 75%
            case 3:
                pwm_cycle(90
                        ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++);
            break;

            // ? 4: 50%
            case 4:
                pwm_cycle(50);
            break;

            // ? 5: 25%
            case 5:
                pwm_cycle(2);
            break;

            // ? 6: PWM 5% - 90%
            case 6:
                pwm_cycle(duty);

                if(up)
                {
                    duty++;
                    if(duty >= 90) up = 0;
                }
                else
                {
                    duty--;
                    if(duty <= 5) up = 1;
                }
            break;
        }
    }
}