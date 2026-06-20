
  
#include <xc.h>
#include <stdint.h>
#include <string.h>

#define _XTAL_FREQ 4000000

// Configuración del PIC18F4550
#pragma config FOSC = HS, WDT = OFF, LVP = OFF, PBADEN = OFF, XINST = OFF
#pragma config CPUDIV = OSC1_PLL2, PLLDIV = 1, USBDIV = 1
#pragma config FCMEN = OFF, IESO = OFF, PWRT = OFF, BOR = OFF
#pragma config VREGEN = OFF, CCP2MX = ON, MCLRE = ON, STVREN = ON

//----------------------------------------
// DECLARACIÓN DE FUNCIONES I2C
//----------------------------------------
void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Write(uint8_t data);
void I2C_SendFrame(void);

//----------------------------------------
// INICIALIZACIÓN I2C (MSSP Master)
//----------------------------------------
void I2C_Init(void)
{
    // Configurar pines RB0 (SDA) y RB1 (SCL) como entrada (modo I2C)
    TRISBbits.TRISB0 = 1;    // SDA como entrada (el hardware controla)
    TRISBbits.TRISB1 = 1;    // SCL como entrada (el hardware controla)
    
    // Configurar puertos como digitales
    ADCON1 = 0x0F;           // Todos los puertos digitales
    
    // Configurar MSSP como I2C Master
    SSPCON1 = 0x28;          // I2C Master mode
    SSPCON2 = 0x00;
    SSPSTAT = 0x00;
    
    // Configurar velocidad: 100kHz con Fosc=4MHz
    SSPADD = 9;              // (4MHz/(4*100kHz))-1 = 9
    
    // Habilitar MSSP
    SSPCON1bits.SSPEN = 1;
    
    __delay_us(100);
}

//----------------------------------------
// GENERAR CONDICIÓN START
//----------------------------------------
void I2C_Start(void)
{
    // Esperar que el bus esté libre
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));
    
    // Generar Start
    SSPCON2bits.SEN = 1;
    while (SSPCON2bits.SEN);  // Esperar que termine
}

//----------------------------------------
// GENERAR CONDICIÓN STOP
//----------------------------------------
void I2C_Stop(void)
{
    // Esperar que el bus esté libre
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));
    
    // Generar Stop
    SSPCON2bits.PEN = 1;
    while (SSPCON2bits.PEN);  // Esperar que termine
}

//----------------------------------------
// ESCRIBIR UN BYTE - SIN ESPERAR ACK
//----------------------------------------
void I2C_Write(uint8_t data)
{
    // Esperar que el módulo esté listo
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));
    
    // Escribir dato al buffer
    SSPBUF = data;
    
    // Esperar que se complete la transmisión del byte
    while (!SSPSTATbits.BF);   // Esperar buffer lleno (se envía)
    
    // IMPORTANTE: NO esperar ACK, solo esperar que termine la transmisión
     while (SSPCON2bits.ACKSTAT);  // <-- ESTO ESTABA BLOQUEANDO
    
    // Dar tiempo para que se complete el ciclo de ACK (9no pulso)
    __delay_us(10);  // Pequeña pausa para completar el ciclo de reloj
}

//----------------------------------------
// GENERAR UNA TRAMA I2C COMPLETA
//----------------------------------------
void I2C_SendFrame(void)
{
    // Iniciar comunicación
    I2C_Start();
    
    // Enviar dirección (ejemplo: 0x78)
    I2C_Write(0x78);
    
    // Enviar datos
    I2C_Write(0x00);
    I2C_Write(0xAE);
    
    // Terminar comunicación
    I2C_Stop();
}

//----------------------------------------
// MAIN PRINCIPAL
//----------------------------------------
void main(void) 
{
    // Inicialización de LEDs
    TRISD = 0xF0;        // RD0-RD3 como salidas
    LATD = 0x00;         // Todos apagados
    
    // Inicializar I2C
    I2C_Init();
    
    // Loop infinito
    while(1) 
    {
        // ========================================
        // ENVIAR SEÑAL I2C (sin esperar ACK)
        // ========================================
        I2C_SendFrame();
        
        // ========================================
        // ANIMACIÓN DE LEDs
        // ========================================
        LATD = 0b00000001;  // RD0
        __delay_ms(5);
        
        LATD = 0b00000010;  // RD1
        __delay_ms(5);
        
        LATD = 0b00000100;  // RD2
        __delay_ms(5);
        
        LATD = 0b00001000;  // RD3
        __delay_ms(5);
    }
}
 