#include "hal/hal_config.h"
#include "hal/hal_led.h"

void LED_Init(void)
{
    // Configurar RD2 y RD3 como salidas digitales
    TRISDbits.TRISD2 = 0;   // LED0
    TRISDbits.TRISD3 = 0;   // LED1

    // Inicializar apagados
    LATDbits.LATD2 = 0;
    LATDbits.LATD3 = 0;
}

void LED_On(uint8_t led)
{
    if(led == LED0)
        LATDbits.LATD2 = 1;
    else if(led == LED1)
        LATDbits.LATD3 = 1;
}

void LED_Off(uint8_t led)
{
    if(led == LED0)
        LATDbits.LATD2 = 0;
    else if(led == LED1)
        LATDbits.LATD3 = 0;
}

void LED_Toggle(uint8_t led)
{
    if(led == LED0)
        LATDbits.LATD2 = !LATDbits.LATD2;
    else if(led == LED1)
        LATDbits.LATD3 = !LATDbits.LATD3;
}
