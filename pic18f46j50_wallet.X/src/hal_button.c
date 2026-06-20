#include "hal/hal_config.h"
#include "hal/hal_button.h"

static uint8_t button_history[2] = {0xFF, 0xFF};

void BUTTON_Init(void)
{
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB5 = 1;
    INTCON2bits.RBPU = 0;
}

uint8_t BUTTON_GetState(uint8_t button)
{
    uint8_t pin_state = 1;
    
    if(button == 0)
        pin_state = PORTBbits.RB4;
    else if(button == 1)
        pin_state = PORTBbits.RB5;
    
    return (pin_state == 0) ? BUTTON_PRESSED : 0;
}
