/**
 * @file    EPD_Driver.cpp
 * @brief   Implementaci√≥n del driver E-Paper para PIC32MX795
 * @author  Leo
 * @date    2026-04-03
 * @version 2.0
 * 
 * @details Implementaci√≥n completa sin plib.h usando registros nativos.
 *          SPI2 a 8 MHz configurado mediante PPS (Peripheral Pin Select).
 *          Core Timer para delays precisos.
 * 
 * @note    Compilador: XC32 v5.00, Target: PIC32MX795F512L
 * @note    Pines SPI: SCK=RB14, SDO=RF5, SDI=RF4
 */

#include "EPD_Driver.h"
#include <string.h>
#include <xc.h>
#include "HardwareProfile.h"



static inline uint32_t ReadCoreTimer(void) {
    uint32_t count;
    __asm__ volatile("mfc0 %0, $9" : "=r"(count));
    return count;
}

void delayMs(uint32_t ms) {
    uint32_t start = ReadCoreTimer();
    uint32_t ticks = (SYS_FREQ / 2 / 1000) * ms;  // 40,000 ticks por ms
    while ((ReadCoreTimer() - start) < ticks);
}

/*==============================================================================
 * CORE TIMER PARA DELAYS PRECISOS
 *============================================================================*/

/**
 * @brief Lee el valor del Core Timer (CP0 Count register)
 * @return Valor actual del timer
 * @note El Core Timer incrementa a SYS_FREQ/2 = 40 MHz
 */


/**
 * @brief Delay en milisegundos usando Core Timer
 * @param ms Tiempo en milisegundos
 * @note Precisi√≥n de ¬±1 tick a 40 MHz (25 ns)
 */
void epd_delayMs(uint32_t ms) {
    uint32_t start = ReadCoreTimer();
    uint32_t ticks = (SYS_FREQ / 2 / 1000) * ms;  // 40,000 ticks/ms
    
    while ((ReadCoreTimer() - start) < ticks);
}

/*==============================================================================
 * GPIO - MANEJO DE PINES (SIN PLIB)
 *============================================================================*/

/**
 * @brief Configura la direcci√≥n de un pin
 * @param pin N√∫mero de pin (0-111)
 * @param mode 0=INPUT, 1=OUTPUT
 * 
 * Mapeo de puertos:
 * - pins 0-15:   PORTA
 * - pins 16-31:  PORTB
 * - pins 32-47:  PORTC
 * - pins 48-63:  PORTD
 * - pins 64-79:  PORTE
 * - pins 80-95:  PORTF
 * - pins 96-111: PORTG
 */
void epd_pinMode(uint8_t pin, uint8_t mode) {
    uint32_t port = pin >> 5;
    uint32_t bit = 1 << (pin & 0x1F);
    
    if (mode == 0) {  // INPUT
        switch(port) {
           // case 0: TRISASET = bit; break;
            case 1: TRISBSET = bit; break;
            case 2: TRISCSET = bit; break;
            case 3: TRISDSET = bit; break;
            case 4: TRISESET = bit; break;
            case 5: TRISFSET = bit; break;
            case 6: TRISGSET = bit; break;
            default: break;
        }
    } else {  // OUTPUT
        switch(port) {
          //  case 0: TRISACLR = bit; break;
            case 1: TRISBCLR = bit; break;
            case 2: TRISCCLR = bit; break;
            case 3: TRISDCLR = bit; break;
            case 4: TRISECLR = bit; break;
            case 5: TRISFCLR = bit; break;
            case 6: TRISGCLR = bit; break;
            default: break;
        }
    }
}

/**
 * @brief Escribe un valor en un pin digital
 * @param pin N√∫mero de pin
 * @param value 0=LOW, 1=HIGH
 */
void epd_digitalWrite(uint8_t pin, uint8_t value) {
    uint32_t port = pin >> 5;
    uint32_t bit = 1 << (pin & 0x1F);
    
    if (value) {
        switch(port) {
//            case 0: LATASET = bit; break;
            case 1: LATBSET = bit; break;
            case 2: LATCSET = bit; break;
            case 3: LATDSET = bit; break;
            case 4: LATESET = bit; break;
            case 5: LATFSET = bit; break;
            case 6: LATGSET = bit; break;
            default: break;
        }
    } else {
        switch(port) {
        //    case 0: LATACLR = bit; break;
            case 1: LATBCLR = bit; break;
            case 2: LATCCLR = bit; break;
            case 3: LATDCLR = bit; break;
            case 4: LATECLR = bit; break;
            case 5: LATFCLR = bit; break;
            case 6: LATGCLR = bit; break;
            default: break;
        }
    }
}

/**
 * @brief Lee el valor de un pin digital
 * @param pin N√∫mero de pin
 * @return 0=LOW, 1=HIGH
 */
uint8_t epd_digitalRead(uint8_t pin) {
    uint32_t port = pin >> 5;
    uint32_t bit = 1 << (pin & 0x1F);
    
    switch(port) {
   //     case 0: return (PORTA & bit) ? 1 : 0;
        case 1: return (PORTB & bit) ? 1 : 0;
        case 2: return (PORTC & bit) ? 1 : 0;
        case 3: return (PORTD & bit) ? 1 : 0;
        case 4: return (PORTE & bit) ? 1 : 0;
        case 5: return (PORTF & bit) ? 1 : 0;
        case 6: return (PORTG & bit) ? 1 : 0;
        default: return 0;
    }
}

/*==============================================================================
 * SPI2 - COMUNICACI√ìN CON EL DISPLAY
 * Configuraci√≥n mediante PPS (Peripheral Pin Select)
 *============================================================================*/

/**
 * @brief Inicializa el perif√©rico SPI2 como Master
 * @note Pines utilizados:
 *       - SCK2:  RB14 (pin 29) - configurado mediante PPS
 *       - SDO2:  RF5  (pin 32) - configurado mediante PPS
 *       - SDI2:  RF4  (pin 31) - configurado mediante PPS
 * @note Configuraci√≥n: 8 MHz, Mode 0 (CKP=0, CKE=0)
 */
/*==============================================================================
 * SPI2 - COMUNICACI”N CON EL DISPLAY (VERSI”N CORREGIDA)
 *============================================================================*/

// Desbloquear PPS en PIC32MX795
/*==============================================================================
 * SPI4 - COMUNICACI”N CON EL DISPLAY (CONFIGURADO CON PPS)
 *============================================================================
 * Pines configurados:
 *   - SCK4:  RB14 (pin 29)
 *   - SDO4:  RF5  (pin 32)
 *   - SDI4:  RF4  (pin 31)
 *   - CS:    RB13  (pin 28) - GPIO, no parte del perifÈrico SPI
 *============================================================================*/

// Desbloquear PPS en PIC32MX795

void spi_init(void) {
    // ========================================================================
    // 1. Configurar pines nativos SPI4
    // ========================================================================
    
    // SCK4 = RG6 (pin 23) - salida
    TRISGCLR = (1 << 6);
    
    // SDO4 = RG7 (pin 24) - salida
    TRISGCLR = (1 << 7);
    
    // SDI4 = RG8 (pin 25) - entrada
    TRISGSET = (1 << 8);
    
    // CS = RB13 (pin 28) - salida (GPIO)
    TRISBCLR = (1 << 13);
    epd_digitalWrite(45, 1);  // CS = RB13 = 45, HIGH inicialmente
    
    // ========================================================================
    // 2. Configurar perifÈrico SPI4
    // ========================================================================
    SPI4CON = 0;            // Reset SPI4
    
    // Baud rate: 80MHz / (2 * 8MHz) = 5 - 1 = 4
    SPI4BRG = 4;
    
    // ConfiguraciÛn SPI Mode 0
    SPI4CONbits.MSTEN = 1;   // Master mode
    SPI4CONbits.MODE16 = 0;  // 8-bit mode
    SPI4CONbits.MODE32 = 0;  // No 32-bit
    SPI4CONbits.CKP = 0;     // Clock polarity: idle low
    SPI4CONbits.CKE = 0;     // Clock edge: data on rising edge
    SPI4CONbits.SMP = 0;     // Sample at middle
    
    // Limpiar flags
    SPI4STATbits.SPIROV = 0;
    
    // Habilitar SPI4
    SPI4CONbits.ON = 1;
}

uint8_t spi_transfer(uint8_t data) {
    while (!SPI4STATbits.SPITBE);
    SPI4BUF = data;
    while (!SPI4STATbits.SPIRBF);
    return SPI4BUF;
}


/*==============================================================================
 * REGISTROS DE INICIALIZACI√ìN
 *============================================================================*/

/**
 * @brief Registros para pantallas peque√±as/medianas
 *        Valores est√°ndar para displays de 1.54" a 4.37"
 */
const uint8_t register_data_sm[6] = { 
    0x00,   // Soft-reset
    0x0e,   // Temperatura
    0x19,   // Temperatura activa
    0x02,   // PSR0
    0xcf,   // PSR1 (peque√±os)
    0x8d    // PSR1 (medianos)
};

/**
 * @brief Registros para pantallas medianas (4.2")
 */
const uint8_t register_data_mid[6] = { 
    0x00,   // Soft-reset
    0x0e,   // Temperatura
    0x19,   // Temperatura activa
    0x02,   // PSR0
    0x0f,   // PSR1
    0x89    // PSR1
};

/*==============================================================================
 * IMPLEMENTACI√ìN DE LA CLASE EPD_Driver
 *============================================================================*/

EPD_Driver::EPD_Driver(eScreen_EPD_t screen, pins_t board) {
    spi_basic = board;
    pdi_size = (uint16_t)(screen >> 8);
    
    // Configurar dimensiones seg√∫n tipo de pantalla
    switch (pdi_size) {
        case 0x15: width = 152; height = 152; break;  // 1.54"
        case 0x21: width = 212; height = 104; break;  // 2.13"
        case 0x26: width = 296; height = 152; break;  // 2.66"
        case 0x27: width = 264; height = 176; break;  // 2.71"
        case 0x28: width = 296; height = 128; break;  // 2.87"
        case 0x37: width = 416; height = 240; break;  // 3.70"
        case 0x41: width = 300; height = 400; break;  // 4.17"
        case 0x43: width = 480; height = 176; break;  // 4.37"
        default: width = 0; height = 0; break;
    }
    
    // Calcular tama√±o del frame buffer (ancho * alto / 8)
    image_data_size = (uint32_t)width * ((uint32_t)height / 8);
    
    // Copiar registros de inicializaci√≥n
    memcpy(register_data, register_data_sm, sizeof(register_data_sm));
}

void EPD_Driver::COG_initial() {
    // Configurar pines de control
    epd_pinMode(spi_basic.panelBusy, 0);   // Entrada
    epd_pinMode(spi_basic.panelDC, 1);     // Salida
    epd_pinMode(spi_basic.panelReset, 1);  // Salida
    epd_pinMode(spi_basic.panelCS, 1);     // Salida
    
    // Estado inicial de pines
    epd_digitalWrite(spi_basic.panelDC, 1);
    epd_digitalWrite(spi_basic.panelReset, 1);
    epd_digitalWrite(spi_basic.panelCS, 1);
    
    // Configurar pines opcionales si est√°n conectados
    if (spi_basic.panelON_EXT2 != NOT_CONNECTED) {
        epd_pinMode(spi_basic.panelON_EXT2, 1);
        epd_pinMode(spi_basic.panelSPI43_EXT2, 1);
        epd_digitalWrite(spi_basic.panelON_EXT2, 1);
        epd_digitalWrite(spi_basic.panelSPI43_EXT2, 0);
    }
    
    // Inicializar SPI
    spi_init();
    epd_delayMs(5);
    
    // Secuencia de reset y soft-reset
    _reset(1, 5, 10, 5, 1);
    _softReset();
    
    // Configurar temperatura
    uint8_t temp = 25;  // 25¬∞C
    _sendIndexData(0xE5, &temp, 1);           // Temperatura
    _sendIndexData(0xE0, &register_data[3], 1); // Temperatura activa
    _sendIndexData(0x00, &register_data[4], 2); // PSR
}

void EPD_Driver::globalUpdate(const uint8_t* data1s, const uint8_t* data2s) {
    _sendIndexData(0x10, data1s, image_data_size);  // Primer frame (BW)
    _sendIndexData(0x13, data2s, image_data_size);  // Segundo frame (Red/0x00)
    _DCDC_powerOn();                                 // Encender DC/DC
    _displayRefresh();                               // Refrescar pantalla
}

void EPD_Driver::COG_powerOff() {
    _sendIndexData(0x02, &register_data[0], 0);  // Apagar DC/DC
    
    // Esperar que termine la operaci√≥n
    while (epd_digitalRead(spi_basic.panelBusy) != 1);
    
    // Poner pines en estado seguro
    epd_digitalWrite(spi_basic.panelDC, 0);
    epd_digitalWrite(spi_basic.panelCS, 0);
    epd_digitalWrite(spi_basic.panelBusy, 0);
    epd_delayMs(150);
    epd_digitalWrite(spi_basic.panelReset, 0);
}

void EPD_Driver::_sendIndexData(uint8_t index, const uint8_t* data, uint32_t len) {
    // Enviar comando (DC = LOW)
    epd_digitalWrite(spi_basic.panelDC, 0);
    epd_digitalWrite(spi_basic.panelCS, 0);
    spi_transfer(index);
    epd_digitalWrite(spi_basic.panelCS, 1);
    
    // Enviar datos (DC = HIGH)
    epd_digitalWrite(spi_basic.panelDC, 1);
    epd_digitalWrite(spi_basic.panelCS, 0);
    for (uint32_t i = 0; i < len; i++) {
        spi_transfer(data[i]);
    }
    epd_digitalWrite(spi_basic.panelCS, 1);
}


void EPD_Driver::_softReset() {
    _sendIndexData(0x00, &register_data[1], 1);
    
    // Timeout de 5 segundos
    uint32_t timeout = 5000;  // 5000 ms
    while (epd_digitalRead(spi_basic.panelBusy) != 1 && timeout > 0) {
        epd_delayMs(1);
        timeout--;
    }
    
    if (timeout == 0) {
        // Error: BUSY nunca se puso en HIGH
        while(1) {
            // Parpadeo r·pido indicando error
//            LED0_ON();
            epd_delayMs(300);
  //          LED0_OFF();
            epd_delayMs(300);
        }
    }
}

void EPD_Driver::_displayRefresh() {
    _sendIndexData(0x12, &register_data[0], 1);  // Display refresh
    while (epd_digitalRead(spi_basic.panelBusy) != 1);
}

void EPD_Driver::_reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5) {
    epd_delayMs(ms1);
    epd_digitalWrite(spi_basic.panelReset, 1);
    epd_delayMs(ms2);
    epd_digitalWrite(spi_basic.panelReset, 0);
    epd_delayMs(ms3);
    epd_digitalWrite(spi_basic.panelReset, 1);
    epd_delayMs(ms4);
    epd_digitalWrite(spi_basic.panelCS, 1);
    epd_delayMs(ms5);
}

void EPD_Driver::_DCDC_powerOn() {
    _sendIndexData(0x04, &register_data[0], 1);  // Power on
    while (epd_digitalRead(spi_basic.panelBusy) != 1);
}



/**
 * @brief Prueba de secuencia de pulsos en pines RB0, RB1, RB2, RB3
 * @param repeticiones N˙mero de veces que repetir· la secuencia
 * @param tiempo_ms Tiempo entre pulsos en milisegundos
 * 
 * @note Los pines corresponden a:
 *       - RB0 = panelBusy (pin 16)
 *       - RB1 = panelDC   (pin 15)
 *       - RB2 = panelReset (pin 14)
 *       - RB3 = panelCS   (pin 13)
 */

void nueva_funcion(void) {
    // RB3 = pin 35 (CS)
    epd_digitalWrite(35, 1);  // Encender RB3
    delayMs(1);                // Esperar 1 ms
    epd_digitalWrite(35, 0);  // Apagar RB3
}

// VersiÛn mejorada con todos los pines:
void pulsos_1ms(void) {
    // Pulso en RB0 (BUSY)
    epd_digitalWrite(32, 1);
    delayMs(1);
    epd_digitalWrite(32, 0);
    
    // Pulso en RB1 (DC)
    epd_digitalWrite(33, 1);
    delayMs(1);
    epd_digitalWrite(33, 0);
    
    // Pulso en RB2 (RESET)
    epd_digitalWrite(34, 1);
    delayMs(1);
    epd_digitalWrite(34, 0);
    
    // Pulso en RB3 (CS)
    epd_digitalWrite(35, 1);
    delayMs(1);
    epd_digitalWrite(35, 0);
}

void test_pines_display(uint16_t repeticiones, uint16_t tiempo_ms) {
    // Configurar los 4 pines como salidas
    epd_pinMode(32, 1);  // RB0 como salida
    epd_pinMode(33, 1);  // RB1 como salida
    epd_pinMode(34, 1);  // RB2 como salida
    epd_pinMode(35, 1);  // RB3 como salida
    
    // Inicializar todos en LOW
    epd_digitalWrite(32, 0);
    epd_digitalWrite(33, 0);
    epd_digitalWrite(34, 0);
    epd_digitalWrite(35, 0);
    
    for(uint16_t r = 0; r < repeticiones; r++) {
        // Secuencia 1: RB0
        epd_digitalWrite(32, 1);
        delayMs(tiempo_ms);
        epd_digitalWrite(32, 0);
        delayMs(tiempo_ms);
        
        // Secuencia 2: RB1
        epd_digitalWrite(33, 1);
        delayMs(tiempo_ms);
        epd_digitalWrite(33, 0);
        delayMs(tiempo_ms);
        
        // Secuencia 3: RB2
        epd_digitalWrite(34, 1);
        delayMs(tiempo_ms);
        epd_digitalWrite(34, 0);
        delayMs(tiempo_ms);
        
        // Secuencia 4: RB3
        epd_digitalWrite(35, 1);
        delayMs(tiempo_ms);
        epd_digitalWrite(35, 0);
        delayMs(tiempo_ms);
    }
}

/**
 * @brief Prueba todos los pines simult·neamente
 * @param tiempo_ms Tiempo encendido/apagado
 */
void test_pines_simultaneos(uint16_t tiempo_ms) {
    epd_pinMode(32, 1);
    epd_pinMode(33, 1);
    epd_pinMode(34, 1);
    epd_pinMode(35, 1);
    
    // Todos encendidos
    epd_digitalWrite(32, 1);
    epd_digitalWrite(33, 1);
    epd_digitalWrite(34, 1);
    epd_digitalWrite(35, 1);
    delayMs(tiempo_ms);
    
    // Todos apagados
    epd_digitalWrite(32, 0);
    epd_digitalWrite(33, 0);
    epd_digitalWrite(34, 0);
    epd_digitalWrite(35, 0);
    delayMs(tiempo_ms);
}

/**
 * @brief Prueba estilo "carrusel" (uno encendido a la vez)
 * @param repeticiones N˙mero de vueltas
 * @param tiempo_ms Tiempo que cada pin permanece encendido
 */
void test_pines_carrusel(uint16_t repeticiones, uint16_t tiempo_ms) {
    epd_pinMode(32, 1);
    epd_pinMode(33, 1);
    epd_pinMode(34, 1);
    epd_pinMode(35, 1);
    
    // Apagar todos inicialmente
    epd_digitalWrite(32, 0);
    epd_digitalWrite(33, 0);
    epd_digitalWrite(34, 0);
    epd_digitalWrite(35, 0);
    
    for(uint16_t r = 0; r < repeticiones; r++) {
        // RB0 solo
        epd_digitalWrite(32, 1);
        delayMs(tiempo_ms);
        epd_digitalWrite(32, 0);
        
        // RB1 solo
        epd_digitalWrite(33, 1);
        delayMs(tiempo_ms);
        epd_digitalWrite(33, 0);
        
        // RB2 solo
        epd_digitalWrite(34, 1);
        delayMs(tiempo_ms);
        epd_digitalWrite(34, 0);
        
        // RB3 solo
        epd_digitalWrite(35, 1);
        delayMs(tiempo_ms);
        epd_digitalWrite(35, 0);
    }
}

/**
 * @brief Prueba de frecuencia m·xima (sin delays)
 */
void test_frecuencia_maxima(void) {
    epd_pinMode(32, 1);
    epd_pinMode(33, 1);
    epd_pinMode(34, 1);
    epd_pinMode(35, 1);
    
    while(1) {
        // Oscilar todos los pines lo m·s r·pido posible
        epd_digitalWrite(32, 1);
        epd_digitalWrite(33, 1);
        epd_digitalWrite(34, 1);
        epd_digitalWrite(35, 1);
        
        epd_digitalWrite(32, 0);
        epd_digitalWrite(33, 0);
        epd_digitalWrite(34, 0);
        epd_digitalWrite(35, 0);
    }
}