#ifndef CONFIG_H  // Evita inclusión múltiple del header
#define CONFIG_H  // Define el guardián de inclusión

#include <xc.h>      // Header específico del compilador XC8
#include <stdint.h>  // Tipos enteros estándar (uint8_t, uint16_t, etc.)
#include <stdbool.h> // Tipo booleano (true/false)

#define _XTAL_FREQ 16000000UL  // Frecuencia del cristal para delays del compilador

//--- Configuración de los Fuses del PIC18F4620 ---
#pragma config OSC = HSPLL   // Oscilador HS con PLL habilitado
#pragma config FCMEN = OFF   // Monitor de fallo de reloj desactivado
#pragma config IESO = OFF    // Conmutación interna/externa desactivada
#pragma config PWRT = ON     // Power-up Timer habilitado
#pragma config BOREN = OFF   // Brown-out Reset desactivado
#pragma config BORV = 0      // Voltaje de BOR en nivel 0
#pragma config WDT = OFF     // Watchdog Timer desactivado
#pragma config PBADEN = OFF  // Pines B como digitales al iniciar
#pragma config LPT1OSC = OFF // Oscilador de baja potencia desactivado
#pragma config MCLRE = ON    // Pin MCLR habilitado como reset
#pragma config STVREN = ON   // Stack overflow/underflow genera reset
#pragma config LVP = OFF     // Programación de bajo voltaje desactivada
#pragma config XINST = OFF   // Modo extendido de instrucciones desactivado
#pragma config DEBUG = OFF   // Modo debug desactivado

//--- Protección de memoria ---
#pragma config CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF  // Bloques de código sin protección
#pragma config CPB = OFF, CPD = OFF   // Boot block y EEPROM sin protección
#pragma config WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF  // Sin protección de escritura
#pragma config WRTC = OFF, WRTB = OFF, WRTD = OFF  // Sin protección de escritura adicional
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF  // Sin protección de tabla de lectura
#pragma config EBTRB = OFF   // Boot block sin protección de tabla de lectura

#define TONEINITIAL 60  // Valor inicial del tono de audio

typedef uint8_t byte;   // Alias para byte sin signo
typedef uint16_t word;  // Alias para palabra de 16 bits

//--- Asignación de pines ---
// Power
#define RETENCION_POWER  LATAbits.LATA0  // Pin que mantiene la alimentación del sistema
#define KEY_SENSE        PORTEbits.RE0   // Puerto de detección de tecla de encendido

// LEDs
#define LED_FERROSO      LATAbits.LATA1  // LED de indicación de metal ferroso
#define LED_NO_FERROSO   LATAbits.LATA2  // LED de indicación de metal no ferroso

// Control de sensibilidad
#define SENSMA           LATBbits.LATB0  // Pin de incremento del potenciómetro digital de sensibilidad
#define SENSME           LATBbits.LATB1  // Pin de decremento del potenciómetro digital de sensibilidad

// Control de volumen
#define VOLUMA           LATBbits.LATB2  // Pin de incremento del potenciómetro digital de volumen
#define VOLUME           LATBbits.LATB3  // Pin de decremento del potenciómetro digital de volumen

// Control de LCD
#define LCD_E            LATBbits.LATB4  // Pin E (Enable) del LCD
#define LCD_BACK         LATBbits.LATB5  // Pin de control del backlight del LCD
#define LCD_RS           LATBbits.LATB6  // Pin RS (Register Select) del LCD

// LCD data (4-bit mode mapped to lower nibble PORTD)
// Uses RD0-RD3 for DB4-DB7

// Audio / Coil / Sample
#define AUDIO_PWM        LATCbits.LATC2  // Salida PWM para el tono de audio
#define SAMPLE           LATCbits.LATC3  // Pin de muestreo de la señal del detector
#define PULSO_BOBINA     LATCbits.LATC4  // Pin de pulso de excitación de la bobina

// Botones
#define BOTON_MUTE       PORTCbits.RC5   // Botón de silencio (Mute)
#define BOTON_RESET      PORTCbits.RC6   // Botón de reset / auto-ajuste
#define BOTON_MAS        PORTCbits.RC7   // Botón de incremento
#define BOTON_MENOS      PORTAbits.RA3   // Botón de decremento
#define BOTON_MENU       PORTAbits.RA4   // Botón de menú
#define BOTON_LIGHT      PORTAbits.RA5   // Botón de backlight

// Máscaras de bits para los botones (compatibilidad con variable Botones)
#define BOTON_MUTE_MASK  0x01  // Bit 0: botón Mute
#define BOTON_RESET_MASK 0x02  // Bit 1: botón Reset
#define BOTON_MAS_MASK   0x04  // Bit 2: botón Más
#define BOTON_MENOS_MASK 0x08  // Bit 3: botón Menos
#define BOTON_MENU_MASK  0x10  // Bit 4: botón Menú
#define BOTON_LIGHT_MASK 0x20  // Bit 5: botón Luz

// Canales del ADC
#define ADC_CH_BATERIA   6  // Canal ADC para medición de batería (AN6)
#define ADC_CH_SENHAL    7  // Canal ADC para la señal del detector (AN7)

// Timing
#define TIEMPOTIMEOUTMENU 40  // Tiempo de timeout del menú en unidades de 100ms

// Límites de variables ajustables
#define DISCRIMINATIONMAXIMA 10  // Valor máximo de discriminación
#define VOLUMENMAXIMO 30        // Volumen máximo
#define SAMPLEMAXIMO 7          // Posición de muestreo máxima
#define GANANCIAMAXIMA 20       // Ganancia máxima
#define TONOMAXIMO 100          // Tono máximo
#define OFFSETTONOMAXIMO 80     // Offset de tono máximo

// Medición de batería
#define MAXIMO_BAT 115  // Valor ADC máximo de batería (equivalente a 100%)
#define ALARMA_BAT 105  // Valor ADC de alarma de batería baja
#define MINIMO_BAT 100  // Valor ADC mínimo de batería (equivalente a 0%)

// Direcciones de EEPROM (0x00-0x07 utilizadas)
#define EEPROM_ADDR_VOLUMEN       0x00  // Dirección EEPROM para Volumen
#define EEPROM_ADDR_DISCRIMINATION 0x01  // Dirección EEPROM para Discriminación
#define EEPROM_ADDR_SAMPLEPOS     0x02  // Dirección EEPROM para SamplePosition
#define EEPROM_ADDR_FLASHFLAG     0x03  // Dirección EEPROM para flag de validación
#define EEPROM_ADDR_GANANCIA      0x04  // Dirección EEPROM para Ganancia
#define EEPROM_ADDR_TONE          0x05  // Dirección EEPROM para Tono
#define EEPROM_ADDR_OFFSETTONO    0x06  // Dirección EEPROM para Offset de Tono
#define EEPROM_ADDR_OTROPARAM     0x07  // Dirección EEPROM para otro parámetro

#define FLASHFLAG_VALID 0x55  // Valor que indica que la EEPROM tiene datos válidos

// Valores por defecto
#define VOLUMEN_DEFAULT 3          // Volumen por defecto
#define DISCRIMINATION_DEFAULT 1   // Discriminación por defecto
#define SAMPLEPOSITION_DEFAULT 4   // Posición de muestreo por defecto
#define GANANCIA_DEFAULT 2         // Ganancia por defecto
#define TONE_DEFAULT 60            // Tono por defecto
#define OFFSETTONO_DEFAULT 0       // Offset de tono por defecto
#define OTROPARAM_DEFAULT 1        // Otro parámetro por defecto
#define SENSIBILIDAD_DEFAULT 15    // Sensibilidad por defecto

#endif  // Fin del guardián de inclusión
