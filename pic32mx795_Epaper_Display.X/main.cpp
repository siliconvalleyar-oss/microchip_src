#include "EPD_Driver.h"
#include <string.h>
#include <xc.h>
#include <stdint.h>
#include "HardwareProfile.h"
           
/*==============================================================================
 * CONFIGURACIÃ“N DEL SISTEMA
 *============================================================================*/

#pragma config FNOSC = PRIPLL      
#pragma config POSCMOD = HS        
#pragma config FPLLIDIV = DIV_2    
#pragma config FPLLMUL = MUL_20    
#pragma config FPLLODIV = DIV_1    
#pragma config FPBDIV = DIV_1      
#pragma config FWDTEN = OFF        
#pragma config ICESEL = ICS_PGx1   
#pragma config CP = OFF 

const pins_t boardConfig = {
    .panelBusy = 32,    // RB0 - pin 16
    .panelDC = 33,      // RB1 - pin 15
    .panelReset = 34,   // RB2 - pin 14
    .panelCS = 35,      // RB3 - pin 13
    .panelON_EXT2 = NOT_CONNECTED,
    .panelSPI43_EXT2 = NOT_CONNECTED,
    .flashCS = NOT_CONNECTED
};

static uint8_t imageBW[2756] = {0};
static uint8_t imageSecond[2756] = {0};





void led_init(void) {
    TRISECLR = (1 << LED0_PIN) | (1 << LED1_PIN) | (1 << LED2_PIN);
     TRISECLR = (1 << LED3_PIN) | (1 << LED4_PIN) | (1 << LED5_PIN);
    LED0_OFF();
    LED1_OFF();
    LED2_OFF();
}

void generateTestPattern(uint8_t* buffer, uint8_t value) {
    memset(buffer, value, 2756);
    
    uint16_t width = 212;
    uint16_t height = 104;
    uint16_t bytesPerLine = width / 8;
    
    for (uint16_t x = 0; x < bytesPerLine; x++) {
        buffer[x] = 0xFF;
        buffer[(height - 1) * bytesPerLine + x] = 0xFF;
    }
    
    for (uint16_t y = 0; y < height; y++) {
        buffer[y * bytesPerLine] |= 0x80;
        buffer[y * bytesPerLine + bytesPerLine - 1] |= 0x01;
    }
}


void  loop_scope(void);


int main(void) {
//loop_scope();

    CHECONbits.PFMWS = 2;
    CHECONbits.PREFEN = 1;
    
    INTCONbits.MVEC = 1;
    __builtin_enable_interrupts();

    led_init();
    LED0_ON();
     delayMs(300);
    
    generateTestPattern(imageBW, 0x00);
    generateTestPattern(imageSecond, 0x00);
    
    EPD_Driver display(eScreen_EPD_213, boardConfig);
        
    LED1_ON();
    delayMs(300);  
   
    display.COG_initial(); 
    LED2_ON();
    display.globalUpdate(imageBW, imageSecond);
    LED3_ON();
    delayMs(300);

    display.COG_powerOff();
    LED4_ON();

    delayMs(300);
    while (1) {
        delayMs(500);
        LED0_OFF();
        LED1_OFF();
        LED2_OFF();
        LED3_OFF();
        LED4_OFF();
        delayMs(500);
        LED0_ON();
        LED1_ON();
        LED2_ON();
        LED3_ON();
        LED4_ON();        
    }
    return 0;
}


/*
Señal	Pin	Puerto	Función
SCK	29	RB14	Clock SPI
SDO	42	RF5	Master Out Slave In
SDI	41	RF4	Master In Slave Out
BUSY	48	RD0	Estado del display
DC	49	RD1	Data/Command
RESET	50	RD2	Reset
CS	51	RD3	Chip Select



pic32mx795  -> conector designacion del 0 al 16

CN0
Pin 16	PGED1/AN0/VREF+/CVREF+/PMA6/CN2/RB0
Pin 15	PGEC1/AN1/VREF-/CVREF-/CN3/RB1
Pin 14	AN2/C2IN-/CN4/RB2
Pin 13	AN3/C2IN+/CN5/RB3
Pin 12	AN4/C1IN-/CN6/RB4
Pin 11	AN5/C1IN+/VBUSON/CN7/RB5
Pin 17	PGEC2/AN6/OCFA/RB6
Pin 18	PGED2/AN7/RB7
Pin 21	AN8/SS4/U5RX/U2CTS/C1OUT/RB8
Pin 22	AN9/C2OUT/PMA7/RB9
Pin 23	TMS/AN10/CVREFOUT/PMA13/RB10
Pin 24	TDO/AN11/PMA12/RB11
Pin 27	TCK/AN12/PMA11/RB12
Pin 28	TDI/AN13/PMA10/RB13
Pin 29	AN14/SCK4/U5TX/U2RTS/PMALH/PMA1/RB14
Pin 30	AN15/OCFB/PMALL/PMA0/CN12/RB15

LEDs:

pin 1-> RE5 -> led 0 
pin 2-> RE6 -> led 1 
pin 3-> RE7 -> led 2 
pin 64 -> RE4 -> led 3 
pin 63 -> RE3 -> led 4  
pin 62 -> RE2 -> led 5
pin 61 -> RE1 -> led 6
pin 60 -> RE0 -> led 7

pines del 49 al 55  =>> led 8 al led 15

RD7 -> led 8
RD6 -> led 9
RD5 -> led 10
RD4 -> led 11
RD3 -> led 12
RD2 -> led 13
RD1 -> led 14
RD0 -> led 15


CN1
pin 46 -> RD11		ECRS/AEREFCLK/IC4/PMCS1/PMA14/INT4/RD11
pin 49 -> RD10		ECOL/AECRSDV/SCL1/IC3/PMCS2/PMA15/INT3/RD10
pin 50 -> RD9		AERXD0/ETXD2/SS3/U4RX/U1CTS/SDA1/IC2/INT2/RD9
pin 42 -> RF5		AC1RX/SCL5/SDO4/U2TX/PMA8/CN18/RF5
pin 41 -> RF4		AC1TX/SDA5/SDI4/U2RX/PMA9/CN17/RF4
pin 37 -> RF3		USBID/RF3

pin 59 -> RF1
pin 58 -> RF0

 * Mapeo de puertos:

 * - pins 16-31:  PORTB
 * - pins 32-47:  PORTC
 * - pins 48-63:  PORTD
 * - pins 64-79:  PORTE
 * - pins 80-95:  PORTF
 * - pins 96-111: PORTG

 */
void  loop_scope(void) {
    CHECONbits.PFMWS = 2;
    CHECONbits.PREFEN = 1;
    
    led_init();
    LED0_ON();  // Indicar inicio
 
    while(1){
    // Prueba 1: Secuencia de pulsos (1 ciclo, 1ms cada pulso)
    test_pines_display(1, 1);
    
    // Prueba 2: Carrusel (2 vueltas, 50ms cada pin)
    test_pines_carrusel(2, 50);
    
    // Prueba 3: Pines simultáneos (3 veces, 100ms)
    for(int i = 0; i < 3; i++) {
        test_pines_simultaneos(100);
    }
    
    LED0_OFF();
    }

}

