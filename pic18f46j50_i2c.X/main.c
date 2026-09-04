#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "hardware_cfg.h"
#include "oled.h"
//#include "fonts.h"
#include "i2c.h"

void System_Init(void);
void ConfigureAllPinsAsOutput(void);
void ConfigureButtons(void);
uint8_t ReadButtons(void);


//----------------------------------------
// MAIN PRINCIPAL
//----------------------------------------
void main(void) 
{
    uint8_t led_counter = 0;
     uint8_t buttons;
    // Inicializar sistema
    System_Init();
    
    // Configurar pines
    ConfigureAllPinsAsOutput();
    ConfigureButtons();
    
    // Inicializar I2C por software y OLED
    I2C_Init();
    OLED_Init();
    
    // Mostrar mensajes
    OLED_SetCursor(0, 0);
    OLED_PrintString("HOLA MUNDO!");
    
    OLED_SetCursor(0, 2);
    OLED_PrintString("PIC18F46J50");
     __delay_ms(500);
     OLED_Clear();
    OLED_SetCursor(0, 0);
    OLED_PrintString("SSD1306 I2C");
    
    OLED_SetCursor(0, 2);
    OLED_PrintString("LED:0");
    
       // Loop principal
    while(1) 
    {
        // Leer pulsadores
        buttons = ReadButtons();
        
        // SW1 (RB4): Incrementa LED
        if(buttons & 0x01) {
            led_counter++;
            if(led_counter > 3) led_counter = 0;
        }
        
        // SW2 (RB5): Decrementa LED
        if(buttons & 0x02) {
            if(led_counter == 0)
                led_counter = 3;
            else
                led_counter--;
        }
        
        // Actualizar LEDs en puerto D
        LATD = (uint8_t)(1 << led_counter);
        
        // Mostrar contador en OLED
        OLED_SetCursor(70, 2);
        OLED_PrintNumber(led_counter);
        
        // Pequeño delay para evitar rebotes y dar tiempo de respuesta
        __delay_ms(50);
    }
}


void System_Init(void)
{
    // Configurar oscilador interno a 4MHz con PLL
    OSCCON = 0b01110000;  // INTOSC 4MHz, PLL enabled
    
    // Esperar estabilización
    __delay_ms(100);
    
    // Configurar todos los pines como digitales
    ANCON0 = 0xFF;
    ANCON1 = 0xFF;
}


//----------------------------------------
// CONFIGURAR PINES
//----------------------------------------
void ConfigureAllPinsAsOutput(void)
{
    TRISA = 0x00;
    LATA = 0x00;
    
    // RB0 (SDA) y RB1 (SCL) serán controlados por I2C software
    // RB4 y RB5 serán configurados como entradas en ConfigureButtons()
    TRISB = 0x00;
    LATB = 0x00;
    
    TRISC = 0x00;
    LATC = 0x00;
    
    TRISD = 0x00;
    LATD = 0x00;
    
    TRISE = 0x00;
    LATE = 0x00;
}


void ConfigureButtons(void)
{
    // Configurar RB4 y RB5 como entradas digitales
    SW1_TRIS = 1;  // Entrada
    SW2_TRIS = 1;  // Entrada
    
    // Habilitar pull-ups internos (opcional, si tus pulsadores son pull-down no los uses)
    // INTCON2bits.RBPU = 0;  // Habilita pull-ups globales en PORTB
    
    // Alternativa: pull-ups externos o configuración según tu hardware
    // Si usas pulsadores a tierra con pull-up externo, la lógica es invertida
}

//----------------------------------------
// LEER PULSADORES CON ANTIRREBOTE
//----------------------------------------
uint8_t ReadButtons(void)
{
    static uint8_t last_sw1 = 0;
    static uint8_t last_sw2 = 0;
    static uint8_t debounce_counter = 0;
    uint8_t result = 0;
    
    // Lectura actual
    uint8_t sw1_current = SW1_PRESSED();
    uint8_t sw2_current = SW2_PRESSED();
    
    // Detectar flanco de subida (transición de 0 a 1)
    if(sw1_current == 1 && last_sw1 == 0) {
        result |= 0x01;  // Bit 0 = SW1 presionado
    }
    
    if(sw2_current == 1 && last_sw2 == 0) {
        result |= 0x02;  // Bit 1 = SW2 presionado
    }
    
    // Actualizar estados anteriores
    last_sw1 = sw1_current;
    last_sw2 = sw2_current;
    
    return result;
}