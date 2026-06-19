// main.c - Con USB CDC real
#include "hal/hal_config.h"
#include "hal/hal_led.h"
#include "hal/hal_button.h"
#include "usb/usb_cdc.h"

static char usb_rx_buffer[64];

//=============================================================================
// Inicialización
//=============================================================================
static void InitializeSystem(void)
{
    ANCON0 = 0xFF;
    ANCON1 = 0xFF;
    
    // Habilitar PLL para 48MHz
    OSCTUNEbits.PLLEN = 1;
    unsigned int pll_wait = 600;
    while(pll_wait--);
    
    LED_Init();
    BUTTON_Init();
    
    // Inicializar USB (importante: esperar a que se conecte)
    USB_CDC_Init();
    
    // Parpadear LEDs indicando inicio
    for(uint8_t i=0; i<3; i++) {
        LED_Toggle(LED0);
        __delay_ms(200);
    }
}

//=============================================================================
// Procesar comandos
//=============================================================================
static void HandleCommands(char* cmd)
{
    if(strcmp(cmd, "LED0_ON") == 0) {
        LED_On(LED0);
        USB_CDC_WriteString("LED0 ON\r\n");
    }
    else if(strcmp(cmd, "LED0_OFF") == 0) {
        LED_Off(LED0);
        USB_CDC_WriteString("LED0 OFF\r\n");
    }
    else if(strcmp(cmd, "LED1_ON") == 0) {
        LED_On(LED1);
        USB_CDC_WriteString("LED1 ON\r\n");
    }
    else if(strcmp(cmd, "LED1_OFF") == 0) {
        LED_Off(LED1);
        USB_CDC_WriteString("LED1 OFF\r\n");
    }
    else if(strcmp(cmd, "HELP") == 0) {
        USB_CDC_WriteString("\r\nComandos:\r\n");
        USB_CDC_WriteString("  LED0_ON, LED0_OFF\r\n");
        USB_CDC_WriteString("  LED1_ON, LED1_OFF\r\n");
        USB_CDC_WriteString("  HELP\r\n\r\n");
    }
    else if(strlen(cmd) > 0) {
        USB_CDC_WriteString("? ");
        USB_CDC_WriteString(cmd);
        USB_CDC_WriteString("\r\n");
    }
}

//=============================================================================
// Tareas principales
//=============================================================================
static void ProcessTasks(void)
{
    static uint8_t counter = 0;
    uint8_t button;
    
    USB_CDC_Tasks();
    
    // Leer comandos por USB
    uint8_t len = USB_CDC_ReadData(usb_rx_buffer, 63);
    if(len > 0) {
        // Eliminar caracteres de nueva línea
        for(uint8_t i=0; i<len; i++) {
            if(usb_rx_buffer[i] == '\r' || usb_rx_buffer[i] == '\n') {
                usb_rx_buffer[i] = '\0';
                break;
            }
        }
        HandleCommands(usb_rx_buffer);
    }
    
    // Leer botones
    if(++counter >= 10) {
        counter = 0;
        
        button = BUTTON_GetState(0);
        if(button & BUTTON_PRESSED) {
            LED_Toggle(LED0);
            USB_CDC_WriteString("Button 0 pressed\r\n");
        }
        
        button = BUTTON_GetState(1);
        if(button & BUTTON_PRESSED) {
            LED_Toggle(LED1);
            USB_CDC_WriteString("Button 1 pressed\r\n");
        }
    }
    
    __delay_ms(10);
}

//=============================================================================
// Main
//=============================================================================
void main(void)
{
    InitializeSystem();
    
    while(1) {
        ProcessTasks();
    }
}

