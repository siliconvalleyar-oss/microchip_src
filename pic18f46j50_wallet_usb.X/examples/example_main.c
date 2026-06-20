//=============================================================================
// main.c - Crypto Wallet with SSD1306 OLED + USB CDC
// RB4 = Move/Scroll, RB5 = Enter/Select
// At startup: sends "Hola mundo" over USB CDC serial
//=============================================================================

#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "hardware_cfg.h"
#include "oled.h"
#include "i2c.h"
#include "wallet.h"
#include "usb/usb_cdc.h"

// Function declarations
void System_Init(void);
void ConfigureAllPinsAsOutput(void);
void ConfigureButtons(void);
uint8_t ReadButtons(void);

//=============================================================================
// MAIN
//=============================================================================
void main(void)
{
    // Inicializar sistema
    System_Init();
    ConfigureAllPinsAsOutput();
    ConfigureButtons();
    
    // Inicializar I2C y OLED
    I2C_Init();
    OLED_Init();
    
    // Inicializar USB CDC
    USB_CDC_Init();
    
    // Inicializar Wallet
    Wallet_Init();
    
    // Loop principal
    while(1)
    {
        uint8_t buttons = ReadButtons();
        Wallet_Update(buttons);
        
        // USB CDC tasks - must be called frequently (<2ms) during enumeration
        // Call multiple times per loop iteration to ensure reliable enumeration
        USB_CDC_Tasks();
        USB_CDC_Tasks();
        
        __delay_ms(1);
    }
}

//=============================================================================
// SYSTEM INIT
//=============================================================================
void System_Init(void)
{
    // Configurar oscilador interno a 16MHz
    OSCCON = 0b01110000;  // IRCF = 111, INTOSC 16MHz
    
    // Habilitar PLL para 48MHz (necesario para USB)
    OSCTUNEbits.PLLEN = 1;
    
    // Esperar estabilizacion del PLL
    __delay_ms(10);
    
    // Configurar todos los pines como digitales
    ANCON0 = 0xFF;
    ANCON1 = 0xFF;
}

//=============================================================================
// CONFIGURAR PINES
//=============================================================================
void ConfigureAllPinsAsOutput(void)
{
    TRISA = 0x00;
    LATA = 0x00;
    
    // RB0 (SDA) y RB1 (SCL) seran controlados por I2C software
    // RB4 y RB5 seran configurados como entradas en ConfigureButtons()
    // RB2 y RB3 son USB (D- y D+) - dejarlos como entradas digitales
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
    SW1_TRIS = 1;  // RB4 = Entrada (Move)
    SW2_TRIS = 1;  // RB5 = Entrada (Enter)
}

//=============================================================================
// LEER PULSADORES CON ANTIRREBOTE (deteccion de flanco)
//=============================================================================
uint8_t ReadButtons(void)
{
    static uint8_t last_sw1 = 0;
    static uint8_t last_sw2 = 0;
    uint8_t result = 0;
    
    // Lectura actual
    uint8_t sw1_current = SW1_PRESSED();
    uint8_t sw2_current = SW2_PRESSED();
    
    // Detectar flanco de subida (transicion de 0 a 1)
    if(sw1_current == 1 && last_sw1 == 0) {
        result |= 0x01;  // Bit 0 = SW1 (RB4) presionado
    }
    
    if(sw2_current == 1 && last_sw2 == 0) {
        result |= 0x02;  // Bit 1 = SW2 (RB5) presionado
    }
    
    // Actualizar estados anteriores
    last_sw1 = sw1_current;
    last_sw2 = sw2_current;
    
    return result;
}
