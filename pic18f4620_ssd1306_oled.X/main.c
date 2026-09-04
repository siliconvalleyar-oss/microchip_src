#include "config.h"
#include "i2c_sw.h"
#include "oled.h"

// ConfiguraciÃ³n de fuses para oscilador externo HS 4MHz 
/*
#pragma config PLLDIV   = 5         // (20 MHz crystal? no, para 4 MHz usar 1? mejor explicar)
#pragma config CPUDIV   = OSC1_PLL2 // Para usar PLL, pero si usas cristal 4 MHz sin PLL es mejor:
// Configuración común para cristal externo de 4 MHz (sin PLL):
#pragma config FOSC     = HS        // Oscilador HS (cristal 4 MHz)
#pragma config PWRT     = ON        // Power-up timer
#pragma config BOR      = ON        // Brown-out reset
#pragma config BORV     = 2         // Brown-out voltage 2.5V
#pragma config WDT      = OFF       // Watchdog OFF
#pragma config WDTPS    = 32768
#pragma config MCLRE    = ON        // MCLR pin enabled
#pragma config LPT1OSC  = OFF
#pragma config PBADEN   = OFF       // PORTB<4:0> digital I/O on reset
#pragma config CCP2MX   = ON        // CCP2 on RC1 (no es PORTC, es ON/OFF)
#pragma config STVREN   = ON
#pragma config LVP      = OFF
#pragma config ICPRT    = OFF
#pragma config XINST    = OFF
#pragma config DEBUG    = OFF
*/
#pragma config OSC = HS
#pragma config FCMEN = OFF, IESO = OFF
#pragma config PWRT = ON, BOREN = SBORDIS, BORV = 3
#pragma config WDT = OFF, WDTPS = 32768
#pragma config CCP2MX = PORTC, PBADEN = OFF, LPT1OSC = OFF, MCLRE = ON
#pragma config STVREN = ON, LVP = OFF, XINST = OFF
#pragma config CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF, CPB = OFF, CPD = OFF
#pragma config WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF, WRTB = OFF, WRTC = OFF, WRTD = OFF
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF, EBTRB = OFF

void main(void)
{
    // Configurar pines digitales y deshabilitar comparadores
    ADCON1 = 0x0F;
    ADCON0 = 0x00;
    CMCON  = 0x07;

    I2C_SW_Init();
    OLED_Init();

    // Prueba: llenar toda la pantalla (debe verse completamente blanca)
    // OLED_FillScreen(0xFF);
    // __delay_ms(2000);
    // OLED_FillScreen(0x00);
    // __delay_ms(500);

    // Escribir textos en todas las filas (0..3)
    OLED_PutStr(0, 0, "Linea 0: 128x32");
    OLED_PutStr(1, 0, "Linea 1: Hola PIC");
    OLED_PutStr(2, 0, "Linea 2: I2C SW OK");
    OLED_PutStr(3, 0, "Linea 3: FINAL");
    OLED_Update();

    while (1) {
        // Bucle vacÃ­o
    }
}
