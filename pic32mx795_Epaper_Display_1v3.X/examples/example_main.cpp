/**
 * @file    main.cpp
 * @brief   AplicaciÛn principal con soporte de texto e im·genes
 * @version 2.0 - CORREGIDO (column-major)
 */

#include "EPD_Driver.h"
#include "EPD_Graphics.h"
#include "HardwareProfile.h"
#include "custom_image_example.h"

#include <string.h>
#include <xc.h>
#include <stdint.h>
#include <memory>


/*==============================================================================
 * CONFIGURACI”N DEL SISTEMA
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
static uint8_t imageRed[5624] = {0};    // Buffer para rojo (0x00 = sin rojo)

// Objeto display global (para que drawDebug pueda usarlo)

/*==============================================================================
 * PROTOTIPOS
 *============================================================================*/
void led_init(void);
void delayMs(uint32_t ms);
void displayDemoText(void);
void displayDemoImage(void);
void displayDemoCombined(void);
/*
void drawDebug(void) {
    epd_clearBuffer(imageBW, 5624, COLOR_WHITE);
    epd_clearBuffer(imageRed, 5624, COLOR_WHITE);

    // Rect√°ngulos de referencia
    epd_drawRect(imageBW, 0, 0, 20, 20, COLOR_BLACK, 1);
    epd_drawRect(imageBW, EPD_WIDTH-20, EPD_HEIGHT-20, 20, 20, COLOR_BLACK, 1);
    epd_drawLine(imageBW, 0, 0, EPD_WIDTH-1, EPD_HEIGHT-1, COLOR_BLACK);

    // Texto de posici√≥n
    epd_drawString(imageBW, 10, 10, "TOP-LEFT", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, EPD_HEIGHT-20, "BOTTOM-LEFT", COLOR_BLACK, COLOR_WHITE);

    // Tabla de caracteres de prueba (may√∫sculas, min√∫sculas, n√∫meros)
    epd_drawString(imageBW, 10, 40, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, 50, "abcdefghijklmnopqrstuvwxyz", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, 60, "0123456789!@#$%^&*()", COLOR_BLACK, COLOR_WHITE);

    display.globalUpdate(imageBW, imageRed);
    delayMs(5000);  // 5 segundos para observar
}

*/
void dimagen(void);

/*==============================================================================
 * IMPLEMENTACI”N
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

    
    // Limpiar buffers
    epd_clearBuffer(imageBW, 5624, COLOR_WHITE);
    epd_clearBuffer(imageRed, 5624, COLOR_WHITE);
    
    // TÌtulo grande en la parte superior
    epd_drawString(imageBW, 10, 10, "EPaper 2.66\"", COLOR_BLACK, COLOR_WHITE);
    
    // LÌnea separadora
    epd_drawHLine(imageBW, 0, 295, 22, COLOR_BLACK);
    
    // InformaciÛn del sistema
    epd_drawString(imageBW, 10, 30, "Resolution: 296x152", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, 40, "Driver: PIC32MX795", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, 50, "SPI: 8 MHz", COLOR_BLACK, COLOR_WHITE);
    
    // LÌnea separadora
    epd_drawHLine(imageBW, 0, 295, 62, COLOR_BLACK);
    
    // Mensaje de estado
    epd_drawString(imageBW, 10, 70, "Status: OK", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, 80, "Demo: Text Mode", COLOR_BLACK, COLOR_WHITE);
    
    // Pie de p·gina
    epd_drawHLine(imageBW, 0, 295, 130, COLOR_BLACK);
    epd_drawString(imageBW, 10, 138, "Pervasive Displays", COLOR_BLACK, COLOR_WHITE);
    
    // Dibujar rect·ngulo alrededor del display
    epd_drawRect(imageBW, 2, 2, 292, 148, COLOR_BLACK, 0);
}

/**
 * @brief Demo 2: Mostrar imagen de prueba (usando funciones de dibujo)
 *        Ahora compatible con column-major.
 */
void displayDemoImage(void) {
    // Limpiar buffers
    epd_clearBuffer(imageBW, 5624, COLOR_WHITE);
    epd_clearBuffer(imageRed, 5624, COLOR_WHITE);
    
    // Dibujar borde usando epd_drawPixel
    for (uint16_t x = 0; x < EPD_WIDTH; x++) {
        epd_drawPixel(imageBW, x, 0, COLOR_BLACK);
        epd_drawPixel(imageBW, x, EPD_HEIGHT - 1, COLOR_BLACK);
    }
    for (uint16_t y = 0; y < EPD_HEIGHT; y++) {
        epd_drawPixel(imageBW, 0, y, COLOR_BLACK);
        epd_drawPixel(imageBW, EPD_WIDTH - 1, y, COLOR_BLACK);
    }
    
    // PatrÛn de ajedrez en el centro
    for (uint16_t y = 20; y < 132; y++) {
        for (uint16_t x = 20; x < 276; x++) {
            if (((x + y) & 1) == 0) {
                epd_drawPixel(imageBW, x, y, COLOR_BLACK);
            }
        }
    }
    
    // LÌneas diagonales
    for (uint16_t i = 0; i < EPD_WIDTH && i < EPD_HEIGHT; i++) {
        epd_drawPixel(imageBW, i, i, COLOR_BLACK);
        epd_drawPixel(imageBW, EPD_WIDTH - 1 - i, i, COLOR_BLACK);
    }
}

/**
 * @brief Demo 3: CombinaciÛn de texto e imagen
 */
void displayDemoCombined(void) {
    // Limpiar buffers
    epd_clearBuffer(imageBW, 5624, COLOR_WHITE);
    epd_clearBuffer(imageRed, 5624, COLOR_WHITE);
    
    // Fondo con patrÛn sutil (lÌneas horizontales cada 8 pÌxeles)
   // for (uint16_t y = 0; y < 152; y += 8) {
   //     epd_drawHLine(imageBW, 0, 295, y, COLOR_BLACK);
  //  }
    
    // ¡rea de texto con fondo blanco (rect·ngulo blanco sobre fondo rayado)
  //  epd_drawRect(imageBW, 5, 5, 286, 70, COLOR_WHITE, 1);
    
    // Texto
    epd_drawString(imageBW, 0, 0, "e-paper display demo", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 28, "2.66\" 296x152", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 41, "pic32mx795 @80mHz", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 54, "date: 2026-04-03", COLOR_BLACK, COLOR_WHITE);
    
    // LÌnea decorativa
    epd_drawHLine(imageBW, 5, 290, 80, COLOR_BLACK);
    
    // InformaciÛn de estado
    epd_drawString(imageBW, 15, 90, "System ready", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 103, "Spi: ok", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 116, "Display: ok", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 15, 129, "Memory: ok", COLOR_BLACK, COLOR_WHITE);
    
    // Barra de progreso simulada
    //epd_drawRect(imageBW, 15, 140, 260, 8, COLOR_BLACK, 0);
    //epd_drawRect(imageBW, 16, 141, 200, 6, COLOR_BLACK, 1);
}

/**
 * @brief Carga la imagen personalizada (funciona porque customPattern ya est· en column-major)
 */

void drawDebug(void) {
    epd_clearBuffer(imageBW, 5624, COLOR_WHITE);
    epd_clearBuffer(imageRed, 5624, COLOR_WHITE);

    // Rect√°ngulos de referencia
    epd_drawRect(imageBW, 0, 0, 20, 20, COLOR_BLACK, 1);
    epd_drawRect(imageBW, EPD_WIDTH-20, EPD_HEIGHT-20, 20, 20, COLOR_BLACK, 1);
//    epd_drawLine(imageBW, 0, 0, EPD_WIDTH-1, EPD_HEIGHT-1, COLOR_BLACK);

    // Texto de posici√≥n
    epd_drawString(imageBW, 10, 10, "TOP-LEFT", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, EPD_HEIGHT-20, "BOTTOM-LEFT", COLOR_BLACK, COLOR_WHITE);

    // Tabla de caracteres de prueba (may√∫sculas, min√∫sculas, n√∫meros)
    epd_drawString(imageBW, 10, 40, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, 50, "abcdefghijklmnopqrstuvwxyz", COLOR_BLACK, COLOR_WHITE);
    epd_drawString(imageBW, 10, 60, "0123456789!@#$%^&*()", COLOR_BLACK, COLOR_WHITE);

//    display.globalUpdate(imageBW, imageRed);
    delayMs(5000);  // 5 segundos para observar
}
void dimagen(void) {
    epd_clearBuffer(imageBW, 5624, COLOR_WHITE);
    epd_clearBuffer(imageRed, 5624, COLOR_WHITE);
    epd_loadImage(imageBW, customPattern, 5624);
    // Nota: la actualizaciÛn se har· desde main()
}

// Funciones de prueba de pines (se mantienen igual)
void test_pin_output(uint8_t pin, const char* nombre, uint16_t tiempo_ms) {
    epd_pinMode(pin, 1);
    for (int i = 0; i < 3; i++) {
        epd_digitalWrite(pin, 1);
        delayMs(tiempo_ms);
        epd_digitalWrite(pin, 0);
        delayMs(tiempo_ms);
    }
    epd_digitalWrite(pin, 0);
}

void test_pin_input(uint8_t pin, const char* nombre, uint8_t led_pin) {
    epd_pinMode(pin, 0);
    epd_pinMode(led_pin, 1);
    for (int i = 0; i < 10; i++) {
        uint8_t val = epd_digitalRead(pin);
        epd_digitalWrite(led_pin, val);
        delayMs(100);
    }
    epd_digitalWrite(led_pin, 0);
}

void test_all_pins(void) {
    epd_pinMode(7, 1);
    epd_digitalWrite(7, 1);
    delayMs(500);
    epd_digitalWrite(7, 0);
    delayMs(500);

    struct {
        uint8_t pin;
        const char* name;
    } output_pins[] = {
        {16, "RB0"}, {17, "RB1"}, {18, "RB2"}, {19, "RB3"},
        {29, "RB13"}, {30, "RB14"}, {84, "RF4"}, {85, "RF5"}
    };

    for (uint8_t i = 0; i < sizeof(output_pins)/sizeof(output_pins[0]); i++) {
        epd_digitalWrite(7, 1);
        delayMs(100);
        epd_digitalWrite(7, 0);
        test_pin_output(output_pins[i].pin, output_pins[i].name, 200);
        delayMs(500);
    }

    epd_pinMode(16, 0);
    epd_pinMode(7, 1);
    for (int i = 0; i < 20; i++) {
        uint8_t val = epd_digitalRead(16);
        epd_digitalWrite(7, val);
        delayMs(100);
    }
    epd_digitalWrite(7, 0);

    for (int i = 0; i < 10; i++) {
        epd_digitalWrite(7, 1);
        delayMs(100);
        epd_digitalWrite(7, 0);
        delayMs(100);
    }
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
    
    // Inicializar SPI4 (pines nativos)
    //spi4_init();
   spi_init();
    // Inicializar LEDs
    led_init();
    LED0_ON();


    
    // Inicializar display
        EPD_Driver display(eScreen_EPD_266, boardConfig);

    delayMs(100);
    
    // Inicializar COG
    display.COG_initial();
    
    
    
    
    
    
    
    
    // Limpiar pantalla a blanco r·pidamente
display.clearScreenFast(true);
delayMs(100);

// Dibujar algo y usar fastUpdate
epd_clearBuffer(imageBW, 5624, COLOR_WHITE);
epd_drawString(imageBW, 10, 10, "Fast Update Test", COLOR_BLACK, COLOR_WHITE);
display.fastUpdate(imageBW, imageBW);
  delayMs(900);
    LED0_OFF();

      LED1_ON();  
    // ================================================================
 
    // Depuraci√≥n de posici√≥n
    drawDebug();
       //display.globalUpdate(imageBW, imageRed);
    display.fastUpdate(imageBW, imageRed);
        delayMs(900);
    // DEMOS
    // ================================================================
    LED1_OFF();

       LED2_ON();     
        
    // Demo 0: Imagen personalizada (cargada desde customPattern)
    dimagen();
    display.globalUpdate(imageBW, imageRed);
    delayMs(500);
    LED2_OFF();
    
    // Demo 1: Texto
    LED3_ON();
    displayDemoText();display.fastUpdate(imageBW, imageRed);
    //display.globalUpdate(imageBW, imageRed);
    delayMs(500);
    LED3_OFF();
  
    // Demo 2: Imagen de prueba (patrÛn generado)
    LED4_ON();
    displayDemoImage();display.fastUpdate(imageBW, imageRed);
    //display.globalUpdate(imageBW, imageRed);
    delayMs(500);
    LED4_OFF();
    
    // Demo 3: CombinaciÛn
    LED5_ON();
    displayDemoCombined();display.fastUpdate(imageBW, imageRed);
    //display.globalUpdate(imageBW, imageRed);
    delayMs(500);
    LED5_OFF();
    
    // Demo 4: Invertir colores
    LED6_ON();
    epd_invertBuffer(imageBW, 5624);display.fastUpdate(imageBW, imageRed);
    //display.globalUpdate(imageBW, imageRed);
    delayMs(500);
    LED6_OFF();
    
    // Restaurar imagen combinada
    LED7_ON();
    displayDemoCombined();display.fastUpdate(imageBW, imageRed);
   // display.globalUpdate(imageBW, imageRed);
    delayMs(500);
    LED7_OFF();
    
    // Apagar COG
    LED8_ON();
    display.COG_powerOff();
    LED8_OFF();
    delayMs(300);
    
    // Loop final con LEDs
    while (1) {
        delayMs(200);
        LED9_OFF(); delayMs(200); LED9_ON();
        LED10_OFF(); delayMs(200); LED10_ON();
        LED11_OFF(); delayMs(200); LED11_ON();
        LED12_OFF(); delayMs(200); LED12_ON();
        LED13_OFF(); delayMs(200); LED13_ON();
        LED14_OFF(); delayMs(200); LED14_ON();
        LED15_OFF(); delayMs(200); LED15_ON();
    }
    
    return 0;
}
