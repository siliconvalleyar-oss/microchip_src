//=============================================================================
// main.c - Firmware Principal para PIC18F46J50
//=============================================================================
#include "hal/hal_config.h"
#include "hal/hal_led.h"
#include "hal/hal_button.h"
#include "hal/hal_i2c.h"
#include "hal/ssd1306.h"
#include "hal/spi_sd.h"
#include "hal/FSconfig.h"
#include "usb/usb_cdc.h"

// Variables de estado
static uint8_t system_state = 0;
static char usb_rx_buffer[64];

//=============================================================================
// Prototipos
//=============================================================================
static void InitializeSystem(void);
static void ProcessTasks(void);
static void HandleCommands(char* cmd);

//=============================================================================
// Interrupciones
//=============================================================================
void __interrupt(high_priority) HighPriorityISR(void)
{
    // USB tasks si se usa interrupción
}

void __interrupt(low_priority) LowPriorityISR(void)
{
    // Tareas de baja prioridad
}

//=============================================================================
// Inicialización del Sistema
//=============================================================================
static void InitializeSystem(void)
{
    // Configuración analógica - todos digitales
    ANCON0 = 0xFF;
    ANCON1 = 0xFF;

    // Habilitar PLL para 48MHz (cristal 12MHz)
    OSCTUNEbits.PLLEN = 1;
    unsigned int pll_wait = 600;
    while(pll_wait--);

    // Inicializar periféricos
    LED_Init();
    BUTTON_Init();
    I2C_Init();
    SSD1306_Init();
    SPI_SD_Init();
    USB_CDC_Init();

    // Mensaje de inicio en OLED
    SSD1306_Clear();
    SSD1306_PrintString(0, 0, "Sistema Iniciado");
    SSD1306_PrintString(1, 0, "USB CDC Listo");

    // Parpadear LED de confirmación
    for(uint8_t i=0; i<3; i++) {
        LED_Toggle(LED0);
        __delay_ms(200);
    }
}

//=============================================================================
// Procesar comandos USB
//=============================================================================
static void HandleCommands(char* cmd)
{
    if(strcmp(cmd, "LED0_ON") == 0) {
        LED_On(LED0);
        USB_CDC_WriteString("LED0 encendido\r\n");
    }
    else if(strcmp(cmd, "LED0_OFF") == 0) {
        LED_Off(LED0);
        USB_CDC_WriteString("LED0 apagado\r\n");
    }
    else if(strcmp(cmd, "LED1_ON") == 0) {
        LED_On(LED1);
        USB_CDC_WriteString("LED1 encendido\r\n");
    }
    else if(strcmp(cmd, "LED1_OFF") == 0) {
        LED_Off(LED1);
        USB_CDC_WriteString("LED1 apagado\r\n");
    }
    else if(strcmp(cmd, "HELP") == 0) {
        USB_CDC_WriteString("\r\nComandos:\r\n");
        USB_CDC_WriteString("  LED0_ON, LED0_OFF\r\n");
        USB_CDC_WriteString("  LED1_ON, LED1_OFF\r\n");
        USB_CDC_WriteString("  HELP\r\n\r\n");
    }
    else if(strlen(cmd) > 0) {
        USB_CDC_WriteString("Comando no reconocido: ");
        USB_CDC_WriteString(cmd);
        USB_CDC_WriteString("\r\n");
    }
}

//=============================================================================
// Tareas periódicas
//=============================================================================
static void ProcessTasks(void)
{
    static uint8_t counter = 0;
    uint8_t button;

    USB_CDC_Tasks();

    // Leer datos USB
    uint8_t len = USB_CDC_ReadData(usb_rx_buffer, 63);
    if(len > 0) {
        usb_rx_buffer[len] = '\0';
        HandleCommands(usb_rx_buffer);
    }

    // Leer botones cada 100ms
    if(++counter >= 10) {
        counter = 0;

        button = BUTTON_GetState(0);
        if(button & BUTTON_PRESSED) {
            LED_Toggle(LED0);
            USB_CDC_WriteString("Boton 0 presionado\r\n");
        }

        button = BUTTON_GetState(1);
        if(button & BUTTON_PRESSED) {
            LED_Toggle(LED1);
            USB_CDC_WriteString("Boton 1 presionado\r\n");
        }
    }

    __delay_ms(10);
}

//=============================================================================
// Programa Principal
//=============================================================================
void main(void)
{
    InitializeSystem();

    while(1) {
        ProcessTasks();
    }
}
