/*
 convert logo.png -resize 128x64 -monochrome logo.bmp
 convert monocromo128x64.jpeg -resize 128x64 -monochrome -depth 1 gray:logo.bin
 xxd -i logo.bin > logo.c

 Compilador XC8 
 Microcontrolador Pic18f4550
 
 */   
#include <xc.h>
#include "oled.h"
#include "sdcard.h"
#include "fonts.h"
#include "image.h"
#include <stdint.h>
#include <string.h>  // Para memcpy

#define _XTAL_FREQ 4000000
// Frecuencia del PIC

// Logo definido en oled.c externamente
extern const uint8_t logo[];
extern unsigned int logo_bin_len;

void main(void) {
    // ------------------ Inicialización ------------------
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
    TRISD = 0xF0;
    LATD = 0x00;
LATD = 0b00000001;
__delay_ms(10000);
LATD = 0x00;
    I2C_Init();
    OLED_Init();
    OLED_Clear();

    OLED_DrawLogo(logo);

        __delay_ms(10000);
               OLED_Clear();   
OLED_ClearBuffer();                  // Limpia el buffer
OLED_SetFont(Font_Four, 8, 8);      // Configura la fuente
OLED_PrintText("HOLA MUNDO", 0, 0); // Escribe en el buffer
OLED_UpdateScreen();                 // Actualiza la pantalla
    // ------------------ Ejemplo de animación ------------------
    while(1) {
       OLED_DrawRect();    // Dibuja un rectángulo borde
        __delay_ms(500);
        OLED_Clear();
        __delay_ms(500);
       
                LATD = 0b00000001;
        __delay_ms(500);

        LATD = 0b00000010;
        __delay_ms(500);

        LATD = 0b00000100;
        __delay_ms(500);

        LATD = 0b00001000;
        __delay_ms(500);
    }
}

        
/*
#include <xc.h>
#include <stdint.h>
#include <string.h>


#include "oled.h"
#include "fonts.h"
#include "image.h"
#include "eprom.h"

#define _XTAL_FREQ 4000000

extern const uint8_t logo[];
extern unsigned int logo_bin_len;

char buffer[32];  // Buffer para lectura desde EEPROM

void main(void) {
    TRISD = 0xF0;
    LATD = 0x00;

    I2C_Init();
    
    OLED_Init();
    OLED_Clear();

    // Mostrar logo
    OLED_DrawLogo(logo);
    __delay_ms(2000);

    // Limpiar buffer y escribir texto
    OLED_ClearBuffer();
    OLED_SetFont(Font_Four, 8, 8);
    OLED_PrintText("EEPROM", 0, 0);
    OLED_UpdateScreen();
    __delay_ms(2000);

    // ------------------ EEPROM ------------------
    EEPROM_WriteString(0x00, "HOLA DESDE EEPROM");
    __delay_ms(100);

    // Leer y mostrar
    EEPROM_ReadString(0x00, buffer, sizeof(buffer));

    OLED_ClearBuffer();
    OLED_PrintText(buffer, 0, 16); // Mostrar texto leído
    OLED_UpdateScreen();
    __delay_ms(2000);

    // ------------------ Ejemplo parpadeo LEDs ------------------
    while(1) {
        LATD = 0b00000001; __delay_ms(5);
        LATD = 0b00000010; __delay_ms(5);
        LATD = 0b00000100; __delay_ms(5);
        LATD = 0b00001000; __delay_ms(5);
    }
}           
*/