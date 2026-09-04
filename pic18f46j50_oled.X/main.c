/**
 * @file    main.c
 * @brief   LEDs secuenciales con control por pulsadores y OLED
 * @version 3.8 - I2C en RB0 (SDA) y RB1 (SCL)
 */

#include <xc.h>
#include <stdio.h>

// ============================================================================
//  Configuración de bits
// ============================================================================
#pragma config WDTEN = OFF
#pragma config XINST = OFF

// ============================================================================
//  Definiciones de pines
// ============================================================================
// LEDs en RD0-RD3
#define LED0            LATDbits.LATD0
#define LED1            LATDbits.LATD1
#define LED2            LATDbits.LATD2
#define LED3            LATDbits.LATD3

// Pulsadores en RB4, RB5, RB6 (pines libres)
#define BUTTON_UP       PORTBbits.RB4
#define BUTTON_DOWN     PORTBbits.RB5
#define BUTTON_DIR      PORTBbits.RB6

#define LED_ON          1
#define LED_OFF         0
#define DEBOUNCE_TIME   50

// OLED definitions
#define OLED_ADDR       0x78    // 0x3C << 1
#define OLED_WIDTH      128
#define OLED_HEIGHT     32
#define OLED_PAGES      4

// ============================================================================
//  Variables globales
// ============================================================================
static uint8_t led_position = 0;
static uint8_t led_direction = 1;
static uint16_t led_speed = 200;
static uint16_t contador_ms = 0;
static uint8_t last_button_up = 1;
static uint8_t last_button_down = 1;
static uint8_t last_button_dir = 1;
static uint8_t oled_buffer[OLED_WIDTH * OLED_PAGES];
static uint8_t oled_ok = 0;

// ============================================================================
//  Delay usando NOPs (calibrado)
// ============================================================================
void DelayMs(uint16_t ms) {
    while(ms--) {
        for(uint16_t i = 0; i < 100; i++) {
            NOP(); NOP(); NOP(); NOP(); NOP();
            NOP(); NOP(); NOP(); NOP(); NOP();
        }
    }
}

// ============================================================================
//  I2C Functions usando MSSP2 (RB0=SDA, RB1=SCL)
// ============================================================================
void I2C_Wait(void) {
    while ((SSP2STAT & 0x04) || (SSP2CON2 & 0x1F));
}

void I2C_Init(void) {
    // Configurar pines RB0 (SDA) y RB1 (SCL) como entradas
    TRISBbits.TRISB0 = 1;   // SDA
    TRISBbits.TRISB1 = 1;   // SCL
    
    // Configurar módulo MSSP2 como I2C maestro
    SSP2CON1 = 0x28;        // I2C Master mode
    SSP2ADD = 9;            // 400 kHz (con 4 MHz)
    SSP2STAT = 0x00;
    
    // Habilitar módulo SSP2
    SSP2CON1bits.SSPEN = 1;
}

void I2C_Start(void) {
    I2C_Wait();
    SSP2CON2bits.SEN = 1;
    while(SSP2CON2bits.SEN);
}

void I2C_Stop(void) {
    I2C_Wait();
    SSP2CON2bits.PEN = 1;
    while(SSP2CON2bits.PEN);
}

uint8_t I2C_Write(uint8_t data) {
    I2C_Wait();
    SSP2BUF = data;
    while(SSP2STATbits.BF);
    return !SSP2CON2bits.ACKSTAT;
}

// ============================================================================
//  OLED Functions
// ============================================================================
void OLED_Command(uint8_t cmd) {
    if(!oled_ok) return;
    I2C_Start();
    if(!I2C_Write(OLED_ADDR)) { oled_ok = 0; I2C_Stop(); return; }
    if(!I2C_Write(0x00)) { oled_ok = 0; I2C_Stop(); return; }
    if(!I2C_Write(cmd)) { oled_ok = 0; I2C_Stop(); return; }
    I2C_Stop();
}

void OLED_Init(void) {
    DelayMs(100);
    
    // Verificar comunicación con el OLED
    I2C_Start();
    if(I2C_Write(OLED_ADDR)) {
        oled_ok = 1;
    } else {
        oled_ok = 0;
        I2C_Stop();
        return;
    }
    I2C_Stop();
    
    // Inicializar OLED 128x32
    OLED_Command(0xAE);     // Display OFF
    OLED_Command(0xD5);     // Oscillator frequency
    OLED_Command(0x80);
    OLED_Command(0xA8);     // Multiplex ratio
    OLED_Command(0x1F);     // 32 lines
    OLED_Command(0xD3);     // Display offset
    OLED_Command(0x00);
    OLED_Command(0x40);     // Start line
    OLED_Command(0x8D);     // Charge pump
    OLED_Command(0x14);
    OLED_Command(0x20);     // Memory mode
    OLED_Command(0x00);
    OLED_Command(0xA1);     // Segment remap
    OLED_Command(0xC8);     // COM scan direction
    OLED_Command(0xDA);     // COM pins
    OLED_Command(0x02);     // For 128x32
    OLED_Command(0x81);     // Contrast
    OLED_Command(0xCF);
    OLED_Command(0xD9);     // Pre-charge
    OLED_Command(0xF1);
    OLED_Command(0xDB);     // VCOM detect
    OLED_Command(0x40);
    OLED_Command(0xA4);     // Display ON resume
    OLED_Command(0xA6);     // Normal display
    OLED_Command(0x2E);     // Deactivate scroll
    OLED_Command(0xAF);     // Display ON
}

void OLED_SetCursor(uint8_t page, uint8_t col) {
    if(!oled_ok) return;
    OLED_Command(0xB0 + page);
    OLED_Command(col & 0x0F);
    OLED_Command(0x10 | (col >> 4));
}

void OLED_PrintChar(char c, uint8_t x, uint8_t y) {
    if(!oled_ok) return;
    if(x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    
    static const uint8_t font[][5] = {
        {0x00,0x00,0x00,0x00,0x00},
        {0x3E,0x51,0x49,0x45,0x3E}, // 0
        {0x00,0x42,0x7F,0x40,0x00}, // 1
        {0x42,0x61,0x51,0x49,0x46}, // 2
        {0x21,0x41,0x45,0x4B,0x31}, // 3
        {0x18,0x14,0x12,0x7F,0x10}, // 4
        {0x27,0x45,0x45,0x45,0x39}, // 5
        {0x3C,0x4A,0x49,0x49,0x30}, // 6
        {0x01,0x71,0x09,0x05,0x03}, // 7
        {0x36,0x49,0x49,0x49,0x36}, // 8
        {0x06,0x49,0x49,0x29,0x1E}, // 9
        {0x7E,0x11,0x11,0x11,0x7E}, // A
        {0x7F,0x49,0x49,0x49,0x36}, // B
        {0x3E,0x41,0x41,0x41,0x22}, // C
        {0x7F,0x41,0x41,0x22,0x1C}, // D
        {0x7F,0x49,0x49,0x49,0x41}, // E
        {0x7F,0x09,0x09,0x09,0x01}, // F
        {0x3E,0x41,0x49,0x49,0x7A}, // G
        {0x7F,0x08,0x08,0x08,0x7F}, // H
        {0x00,0x41,0x7F,0x41,0x00}, // I
        {0x20,0x40,0x41,0x3F,0x01}, // J
        {0x7F,0x08,0x14,0x22,0x41}, // K
        {0x7F,0x40,0x40,0x40,0x40}, // L
        {0x7F,0x02,0x0C,0x02,0x7F}, // M
        {0x7F,0x04,0x08,0x10,0x7F}, // N
        {0x3E,0x41,0x41,0x41,0x3E}, // O
        {0x7F,0x09,0x09,0x09,0x06}, // P
        {0x3E,0x41,0x51,0x21,0x5E}, // Q
        {0x7F,0x09,0x19,0x29,0x46}, // R
        {0x46,0x49,0x49,0x49,0x31}, // S
        {0x01,0x01,0x7F,0x01,0x01}, // T
        {0x3F,0x40,0x40,0x40,0x3F}, // U
        {0x1F,0x20,0x40,0x20,0x1F}, // V
        {0x7F,0x20,0x18,0x20,0x7F}, // W
        {0x63,0x14,0x08,0x14,0x63}, // X
        {0x07,0x08,0x70,0x08,0x07}, // Y
        {0x61,0x51,0x49,0x45,0x43}, // Z
    };
    
    uint8_t idx = 0;
    if(c >= '0' && c <= '9') idx = c - '0' + 1;
    else if(c >= 'A' && c <= 'Z') idx = c - 'A' + 11;
    else if(c == ' ') idx = 0;
    else return;
    
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    
    for(uint8_t i = 0; i < 5; i++) {
        if(x + i < OLED_WIDTH) {
            uint8_t col = font[idx][i];
            for(uint8_t j = 0; j < 7; j++) {
                if(col & (1 << j)) {
                    if(page == (y + j) / 8) {
                        uint8_t new_bit = (y + j) % 8;
                        oled_buffer[page * OLED_WIDTH + x + i] |= (1 << new_bit);
                    }
                }
            }
        }
    }
}

void OLED_PrintText(const char* str, uint8_t x, uint8_t y) {
    if(!oled_ok) return;
    while(*str && x < OLED_WIDTH) {
        OLED_PrintChar(*str++, x, y);
        x += 6;
    }
}

void OLED_UpdateScreen(void) {
    if(!oled_ok) return;
    for(uint8_t page = 0; page < OLED_PAGES; page++) {
        OLED_SetCursor(page, 0);
        I2C_Start();
        if(!I2C_Write(OLED_ADDR)) { oled_ok = 0; I2C_Stop(); return; }
        if(!I2C_Write(0x40)) { oled_ok = 0; I2C_Stop(); return; }
        for(uint8_t col = 0; col < OLED_WIDTH; col++) {
            I2C_Write(oled_buffer[page * OLED_WIDTH + col]);
        }
        I2C_Stop();
    }
}

void OLED_ClearBuffer(void) {
    for(uint16_t i = 0; i < (OLED_WIDTH * OLED_PAGES); i++) {
        oled_buffer[i] = 0x00;
    }
}

// ============================================================================
//  Funciones de LEDs y pulsadores
// ============================================================================

void LED_AllOn(void) {
    LED0 = LED_ON; LED1 = LED_ON; LED2 = LED_ON; LED3 = LED_ON;
}

void LED_AllOff(void) {
    LED0 = LED_OFF; LED1 = LED_OFF; LED2 = LED_OFF; LED3 = LED_OFF;
}

void LED_SetPosition(uint8_t pos) {
    LED_AllOff();
    switch(pos) {
        case 0: LED0 = LED_ON; break;
        case 1: LED1 = LED_ON; break;
        case 2: LED2 = LED_ON; break;
        case 3: LED3 = LED_ON; break;
    }
}

void LED_UpdateSequence(void) {
    if(led_direction) {
        led_position++;
        if(led_position > 3) led_position = 0;
    } else {
        if(led_position == 0) led_position = 3;
        else led_position--;
    }
    LED_SetPosition(led_position);
}

uint8_t ReadButton(uint8_t current, uint8_t *last) {
    if(current == 0 && *last == 1) {
        DelayMs(DEBOUNCE_TIME);
        *last = current;
        return 1;
    }
    *last = current;
    return 0;
}

void HandleButtons(void) {
    if(ReadButton(BUTTON_UP, &last_button_up)) {
        if(led_speed > 50) led_speed -= 50;
        else led_speed = 50;
        LED_AllOn(); DelayMs(50); LED_AllOff();
    }
    
    if(ReadButton(BUTTON_DOWN, &last_button_down)) {
        if(led_speed < 500) led_speed += 50;
        else led_speed = 500;
        LED_AllOn(); DelayMs(50); LED_AllOff();
    }
    
    if(ReadButton(BUTTON_DIR, &last_button_dir)) {
        led_direction = !led_direction;
        LED_AllOn(); DelayMs(100); LED_AllOff();
        DelayMs(100); LED_AllOn(); DelayMs(100); LED_AllOff();
    }
}

// ============================================================================
//  Inicialización del sistema
// ============================================================================
void System_Init(void) {
    OSCCON = 0x60;          // 8 MHz
    
    for(uint8_t i = 0; i < 100; i++) {
        NOP();
    }
    
    // LEDs como salidas en RD0-RD3
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;
    
    // Pulsadores como entradas en RB4, RB5, RB6
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB5 = 1;
    TRISBbits.TRISB6 = 1;
    
    // Pines I2C RB0 (SDA) y RB1 (SCL) - se configuran en I2C_Init()
    
    // Pines digitales
    ANCON0 = 0xFF;          // Todo puerto B digital
    ANCON1 = 0x1F;          // RD0-RD3 digitales
    
    // Pull-ups para pulsadores
    INTCON2bits.RBPU = 0;
    
    // Apagar LEDs
    LED0 = LED_OFF;
    LED1 = LED_OFF;
    LED2 = LED_OFF;
    LED3 = LED_OFF;
}

// ============================================================================
//  Función principal
// ============================================================================
void main(void) {
    char buffer[20];
    
    System_Init();
    
    // Prueba de LEDs al inicio
    LED_AllOn();
    DelayMs(500);
    LED_AllOff();
    DelayMs(500);
    
    // Inicializar I2C (MSSP2) y OLED
    I2C_Init();
    OLED_Init();
    
    // Mostrar mensaje de estado
    if(oled_ok) {
        OLED_ClearBuffer();
        OLED_PrintText("OLED OK!", 45, 0);
        OLED_PrintText("PIC18F46J50", 25, 8);
        OLED_PrintText("LED SEQUENCER", 15, 16);
        OLED_UpdateScreen();
    } else {
        // Indicar que OLED no responde (parpadeo rápido en LED0)
        for(int i = 0; i < 20; i++) {
            LED0 = LED_ON; DelayMs(100); LED0 = LED_OFF; DelayMs(100);
        }
    }
    
    // Bucle principal
    while(1) {
        HandleButtons();
        
        if(oled_ok) {
            sprintf(buffer, "SPD:%03d", led_speed);
            OLED_PrintText(buffer, 10, 32);
            sprintf(buffer, "DIR:%s", led_direction ? "FWD" : "REV");
            OLED_PrintText(buffer, 80, 32);
            OLED_UpdateScreen();
        }
        
        DelayMs(1);
        contador_ms++;
        
        if(contador_ms >= led_speed) {
            contador_ms = 0;
            LED_UpdateSequence();
        }
    }
}