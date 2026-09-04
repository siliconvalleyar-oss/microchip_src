/**
 * @file    main.cpp
 * @brief   Aplicación principal con soporte de texto e imágenes
 * @version 2.0
 */

#include "EPD_Driver.h"
#include "EPD_Graphics.h"
#include "HardwareProfile.h"
#include "custom_image_example.h"

#include <string.h>
#include <xc.h>
#include <stdint.h>

/*==============================================================================
 * CONFIGURACIÓN DEL SISTEMA
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

/*
const pins_t boardConfig = {
    .panelBusy = 48,    // RD0 - pin 48
    .panelDC = 49,      // RD1 - pin 49
    .panelReset = 50,   // RD2 - pin 50
    .panelCS = 51,      // RD3 - pin 51
    .panelON_EXT2 = NOT_CONNECTED,
    .panelSPI43_EXT2 = NOT_CONNECTED,
    .flashCS = NOT_CONNECTED
};
*/
const pins_t boardConfig = {
    .panelBusy = 32,    // RB0 - pin 16
    .panelDC = 33,      // RB1 - pin 15
    .panelReset = 34,   // RB2 - pin 14
    .panelCS = 35,      // RB3 - pin 13
    .panelON_EXT2 = NOT_CONNECTED,
    .panelSPI43_EXT2 = NOT_CONNECTED,
    .flashCS = NOT_CONNECTED
};

/*
const pins_t boardConfig = {
    .panelBusy = 16,    // RB0
    .panelDC = 17,      // RB1
    .panelReset = 18,   // RB2
    .panelCS = 19,      // RB3
    .panelON_EXT2 = NOT_CONNECTED,
    .panelSPI43_EXT2 = NOT_CONNECTED,
    .flashCS = 29       // RB13 (opcional, si se usa Flash)
};*/

//    .panelBusy    = RB0,     
//    .panelDC      = RB1,       
//    .panelReset   = RB2 ,    
//    .panelCS      = RB3,      

//10 GND white
//9 ECSM gray     RB3       ->  Panel_CS        // pin3 demoboard_mx795 - an3
//8 FCSM violet   RB13      ->  Flash_CS        // pin13 demoboard_mx795  - an13
//7 MOSI Blue      RF5      ->  MOSI            // pin 1 otro conector  demoboard_mx795
//6 MISO Green     RF4      ->  MISO            // pin 2   otro conector  demoboard_mx795
//5 RST Yellow    RB2       ->                  // pin2 demoboard_mx795 an2
//4 D/C Orange    RB1       ->                  // pin1 demoboard_mx795  an1
//3 BUSY Red      RB0       ->                  // pin0 demoboard_mx795 an0
//2 SCK Browm     RB14      ->                  // pin14 demoboard_mx795  - an13
//1 vcc Black      



// Buffers para las dos capas (Blanco/Negro y Rojo)
static uint8_t imageBW[5624] = {0};     // Buffer para blanco/negro
static uint8_t imageRed[5624] = {1};    // Buffer para rojo (0x00 = sin rojo)
//extern const uint8_t customPattern[5624];
/*==============================================================================
 * PROTOTIPOS
 *============================================================================*/
void led_init(void);
void delayMs(uint32_t ms);
void displayDemoText(void);
void displayDemoImage(void);
void displayDemoCombined(void);

/*==============================================================================
 * IMPLEMENTACIÓN
 *============================================================================*/

void led_init(void) {
    TRISECLR = (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2);
    LED0_OFF();
    LED1_OFF();
    LED2_OFF();
    LED3_OFF();
    LED4_OFF();
    LED5_OFF();
    LED6_OFF();
    LED7_OFF();
    
    TRISDCLR = (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2);
    LED8_OFF();
    LED9_OFF();
    LED10_OFF();
    LED11_OFF();
    LED12_OFF();
    LED13_OFF();
    LED14_OFF();
    LED15_OFF();
}

/**
 * @brief Demo 1: Mostrar texto en pantalla
 */
void displayDemoText(void) {
    LED0_ON();
    
    // Limpiar buffers
    epd_clearBuffer(imageBW, 5624, COLOR_WHITE);
    epd_clearBuffer(imageRed, 5624, COLOR_WHITE);
    
    // Título grande en la parte superior
    epd_drawString(imageBW, 10, 10, "EPaper 2.66\"", COLOR_BLACK, COLOR_WHITE);
    
    // Línea separadora
    epd_drawHLine(imageBW, 0, 295, 22, COLOR_BLACK);
    
    // Información del sistema
    epd_drawString(imageBW, 10, 30, "Resolution: 296x152", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, 40, "Driver: PIC32MX795", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, 50, "SPI: 8 MHz", COLOR_BLACK, COLOR_WHITE);
    
    // Línea separadora
    epd_drawHLine(imageBW, 0, 295, 62, COLOR_BLACK);
    
    // Mensaje de estado
    epd_drawString(imageBW, 10, 70, "Status: OK", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, 80, "Demo: Text Mode", COLOR_BLACK, COLOR_WHITE);
    
    // Pie de página
    epd_drawHLine(imageBW, 0, 295, 130, COLOR_BLACK);
    epd_drawString(imageBW, 10, 138, "Pervasive Displays", COLOR_BLACK, COLOR_WHITE);
    
    // Dibujar rectángulo alrededor del display
    epd_drawRect(imageBW, 2, 2, 292, 148, COLOR_BLACK, 0);
    
    LED0_OFF();
   // LED1_ON();
}

/**
 * @brief Demo 2: Mostrar imagen de prueba
 */
void displayDemoImage(void) {

    
    // Limpiar buffers
    epd_clearBuffer(imageBW, 5624, COLOR_WHITE);
    epd_clearBuffer(imageRed, 5624, COLOR_WHITE);
    
    // Generar patrón de prueba en el buffer
    uint16_t width = 296;
    uint16_t height = 152;
    uint16_t bytesPerRow = width / 8;
    
    // Dibujar borde
    for (uint16_t x = 0; x < width; x++) {
        uint32_t topByte = 0 * bytesPerRow + (x / 8);
        uint32_t bottomByte = (height - 1) * bytesPerRow + (x / 8);
        uint8_t bitMask = 0x80 >> (x % 8);
        imageBW[topByte] |= bitMask;
        imageBW[bottomByte] |= bitMask;
    }
    
    for (uint16_t y = 0; y < height; y++) {
        uint32_t leftByte = y * bytesPerRow + (0 / 8);
        uint32_t rightByte = y * bytesPerRow + ((width - 1) / 8);
        uint8_t leftMask = 0x80 >> (0 % 8);
        uint8_t rightMask = 0x80 >> ((width - 1) % 8);
        imageBW[leftByte] |= leftMask;
        imageBW[rightByte] |= rightMask;
    }
    
    // Patrón de ajedrez
    for (uint16_t y = 20; y < 132; y++) {
        for (uint16_t x = 20; x < 276; x++) {
            if (((x + y) & 1) == 0) {
                uint32_t byteIndex = y * bytesPerRow + (x / 8);
                uint8_t bitMask = 0x80 >> (x % 8);
                imageBW[byteIndex] |= bitMask;
            }
        }
    }
    
    // Líneas diagonales
    for (uint16_t i = 0; i < width && i < height; i++) {
        uint32_t byteIndex = i * bytesPerRow + (i / 8);
        uint8_t bitMask = 0x80 >> (i % 8);
        imageBW[byteIndex] |= bitMask;
        
        uint32_t byteIndex2 = i * bytesPerRow + ((width - 1 - i) / 8);
        uint8_t bitMask2 = 0x80 >> ((width - 1 - i) % 8);
        imageBW[byteIndex2] |= bitMask2;
    }
}

/**
 * @brief Demo 3: Combinación de texto e imagen
 */
void displayDemoCombined(void) {

    // Limpiar buffers
    epd_clearBuffer(imageBW, 5624, COLOR_WHITE);
    epd_clearBuffer(imageRed, 5624, COLOR_WHITE);
    
    // Fondo con patrón sutil (líneas horizontales cada 8 píxeles)
    for (uint16_t y = 0; y < 152; y += 8) {
        epd_drawHLine(imageBW, 0, 295, y, COLOR_BLACK);
    }
    
    // Área de texto con fondo blanco (rectángulo blanco sobre fondo rayado)
    epd_drawRect(imageBW, 5, 5, 286, 70, COLOR_WHITE, 1);
    
    // Texto
    epd_drawString(imageBW, 15, 15, "E-Paper Display Demo", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 28, "2.66\" 296x152", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 41, "PIC32MX795 @80MHz", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 54, "Date: 2026-04-03", COLOR_BLACK, COLOR_WHITE);
    
    // Línea decorativa
    epd_drawHLine(imageBW, 5, 290, 80, COLOR_BLACK);
    
    // Información de estado
    epd_drawString(imageBW, 15, 90, "System Ready", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 103, "SPI: OK", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 116, "Display: OK", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 129, "Memory: OK", COLOR_BLACK, COLOR_WHITE);
    
    // Barra de progreso simulada
    epd_drawRect(imageBW, 15, 140, 260, 8, COLOR_BLACK, 0);
    epd_drawRect(imageBW, 16, 141, 200, 6, COLOR_BLACK, 1);
    

}






void test_pin_output(uint8_t pin, const char* nombre, uint16_t tiempo_ms) {
    // Configurar como salida
    epd_pinMode(pin, 1);
    // Parpadear 3 veces
    for (int i = 0; i < 3; i++) {
        epd_digitalWrite(pin, 1);
        delayMs(tiempo_ms);
        epd_digitalWrite(pin, 0);
        delayMs(tiempo_ms);
    }
    // Dejar en bajo
    epd_digitalWrite(pin, 0);
}


void test_pin_input(uint8_t pin, const char* nombre, uint8_t led_pin) {
    epd_pinMode(pin, 0);  // entrada
    epd_pinMode(led_pin, 1); // LED como salida
    // Leer 10 veces cada 100ms y mostrar en LED
    for (int i = 0; i < 10; i++) {
        uint8_t val = epd_digitalRead(pin);
        if (val) {
            // Si el pin est� en alto, enciende LED
            epd_digitalWrite(led_pin, 1);
        } else {
            epd_digitalWrite(led_pin, 0);
        }
        delayMs(100);
    }
    epd_digitalWrite(led_pin, 0);
}


void test_all_pins(void) {
    // Usar LED0 (RE7) para indicar inicio de prueba
    epd_pinMode(7, 1); // Asumiendo LED0 en RE7 (pin 7 seg�n hardware profile)
    epd_digitalWrite(7, 1);
    delayMs(500);
    epd_digitalWrite(7, 0);
    delayMs(500);

    // Lista de pines a probar como salida
    struct {
        uint8_t pin;
        const char* name;
    } output_pins[] = {
        {16, "RB0"},
        {17, "RB1"},
        {18, "RB2"},
        {19, "RB3"},
        {29, "RB13"},
        {30, "RB14"},
        {84, "RF4"},
        {85, "RF5"}
    };

    // Probar cada pin como salida (parpadeo)
    for (uint8_t i = 0; i < sizeof(output_pins)/sizeof(output_pins[0]); i++) {
        // Indicar con LED0 que estamos probando este pin
        epd_digitalWrite(7, 1);
        delayMs(100);
        epd_digitalWrite(7, 0);
        
        // Parpadear el pin 5 veces para que se vea bien
        test_pin_output(output_pins[i].pin, output_pins[i].name, 200);
        
        delayMs(500); // pausa entre pines
    }

    // Prueba especial para RB0 como entrada (BUSY)
    // Conecta un pulsador a RB0 o d�jalo flotante (flotante leer� 0 o 1 inestable)
    // Mostramos el estado en LED0 durante 2 segundos
    epd_pinMode(16, 0); // RB0 como entrada
    epd_pinMode(7, 1);  // LED0 salida
    for (int i = 0; i < 20; i++) {
        uint8_t val = epd_digitalRead(16);
        epd_digitalWrite(7, val);
        delayMs(100);
    }
    epd_digitalWrite(7, 0);

    // Finalizar: parpadeo r�pido de LED0
    for (int i = 0; i < 10; i++) {
        epd_digitalWrite(7, 1);
        delayMs(100);
        epd_digitalWrite(7, 0);
        delayMs(100);
    }
}


void dimagen(void) { \
    epd_clearBuffer(imageBW, 5624, COLOR_WHITE); 
    epd_clearBuffer(imageRed, 5624, COLOR_WHITE); 
    epd_loadImage(imageBW, customPattern, 5624); 
    //display.globalUpdate(imageBW, imageRed); 
}

/*==============================================================================
 * MAIN
 *============================================================================*/


int main(void) {
    
    // Configurar prefetch cache
    CHECONbits.PFMWS = 2;
    CHECONbits.PREFEN = 1;
    
    // Habilitar interrupciones vectorizadas
    INTCONbits.MVEC = 1;
    __builtin_enable_interrupts();
    
    // Inicializar LEDs
    
    spi4_init();

    led_init();
    LED0_ON();
    delayMs(300);
    LED0_OFF();
    // Inicializar display
    EPD_Driver display(eScreen_EPD_266, boardConfig);
    
    LED1_ON();
    delayMs(300);

    // Inicializar COG
    display.COG_initial();
        LED1_OFF();
    LED2_ON();
    delayMs(300);
    
    // ================================================================
    // DEMOS
    // ================================================================
    
    // Demo 1: Texto
  //  displayDemoText();
    dimagen();
    display.globalUpdate(imageBW, imageRed);
    delayMs(500);
       LED2_OFF();
       
       /*
    // Demo 2: Imagen de prueba
    displayDemoImage();
        LED3_ON();
    display.globalUpdate(imageBW, imageRed);
    delayMs(500);
        LED3_OFF();
    // Demo 3: Combinación
    displayDemoCombined();
    display.globalUpdate(imageBW, imageRed);
    delayMs(500);
    
    // Demo 4: Invertir colores (efecto de negativo)
    LED4_ON();
    epd_invertBuffer(imageBW, 5624);
    display.globalUpdate(imageBW, imageRed);
    delayMs(500);
     LED4_OFF();
    // Restaurar imagen original
    displayDemoCombined();
    display.globalUpdate(imageBW, imageRed);
    delayMs(500);
     LED5_ON();
    // Apagar COG
    display.COG_powerOff();
       LED5_OFF();
    delayMs(300);
     LED6_ON();
    // Loop principal - parpadeo de LEDs indicando fin
     */
    while (1) {
        delayMs(200);
        LED7_OFF();
        delayMs(200);
        LED7_ON();

        
        delayMs(200);
        LED8_OFF();
        delayMs(200);
        LED8_ON();

        delayMs(200);
        LED9_OFF();
        delayMs(200);
        LED9_ON();

        delayMs(200);
        LED10_OFF();
        delayMs(200);
        LED10_ON();


        delayMs(200);
        LED11_OFF();
        delayMs(200);
        LED11_ON();


        delayMs(200);
        LED12_OFF();
        delayMs(200);
        LED12_ON();

        delayMs(200);
        LED13_OFF();
        delayMs(200);
        LED13_ON();        
        
    }
    
    return 0;
}
