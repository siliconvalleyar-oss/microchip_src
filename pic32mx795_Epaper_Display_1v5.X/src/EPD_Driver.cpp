/**
 * @file    EPD_Driver.cpp
 * @brief   Implementación del driver E-Paper para PIC32MX795
 * @author  Leo
 * @date    2026-04-03
 * @version 3.0 - CORREGIDO
 *
 * BUGS CORREGIDOS:
 *  1. spi_init() usaba pines SPI4 NATIVOS incorrectos (RG6/RG7/RG8).
 *     PIC32MX795 NO tiene SPI4 nativo en PORTG ? esos pines son de SPI2.
 *     Los pines correctos con PPS son RB14(SCK4), RF5(SDO4), RF4(SDI4).
 *     SOLUCIÓN: Configurar PPS correctamente para SPI4.
 *
 *  2. spi_init() mapeaba CS a pin 45 (RB13 con la fórmula pin>>5 da
 *     puerto 1 = PORTB, bit 13). El valor 45 = 32+13 ? PORTB bit 13. OK,
 *     pero llamaba epd_digitalWrite(45,1) ANTES de configurar el TRIS.
 *     SOLUCIÓN: TRIS configurado primero, luego estado inicial.
 *
 *  3. epd_pinMode() y epd_digitalWrite() usan el esquema pin = puerto*32 + bit.
 *     Los pins pasados desde boardConfig son valores DIRECTOS (16,17,18,19)
 *     que NO coinciden con ese esquema:
 *       pin 16 ? puerto 16>>5=0 (PORTA), bit 0  ? ERROR, debería ser PORTB bit 0
 *       pin 17 ? puerto 0, bit 17                ? ERROR
 *     SOLUCIÓN: Cambiar boardConfig para usar el esquema correcto:
 *       RB0 = 32+0=32, RB1=33, RB2=34, RB3=35, RB13=45
 *     Los pines en main.cpp y boardConfig deben ser 32,33,34,35.
 *
 *  4. CKE=0 es incorrecto para el protocolo del display Pervasive Displays.
 *     El EPD requiere datos capturados en flanco de bajada del clock (SPI Mode 0
 *     con CKE=1, es decir CPOL=0, CPHA=0 según convención Motorola/PIC32:
 *     CKP=0, CKE=1 ? datos cambian en flanco de bajada, capturan en subida).
 *     SOLUCIÓN: CKE=1.
 *
 *  5. image_data_size calculado como width * (height/8) en lugar de
 *     (width * height) / 8. Para 296x152: 296*(152/8)=296*19=5624 ? coincide
 *     por ser divisible, pero la fórmula es incorrecta en general.
 *     SOLUCIÓN: imagen_data_size = ((uint32_t)width * height + 7) / 8.
 *
 *  6. _softReset() envía register_data[1] (0x0e) como comando de soft-reset.
 *     El comando de reset del Pervasive Displays UC8171 es 0x00, el dato 0x0e
 *     es el valor de temperatura. El índice debe ser 0x00 y el dato 0x0e.
 *     El código actual: _sendIndexData(0x00, &register_data[1], 1) ? envía
 *     cmd=0x00, data=0x0e. Eso es correcto, pero la lógica de BUSY está
 *     invertida: el display Pervasive Displays BUSY = LOW cuando está ocupado,
 *     HIGH cuando está listo. El código espera != 1 que es correcto, OK.
 *
 *  7. COG_powerOff() llama epd_digitalWrite(spi_basic.panelBusy, 0) pero
 *     panelBusy es una ENTRADA, escribir en él no tiene sentido y puede
 *     causar conflicto. SOLUCIÓN: eliminar esa línea.
 *
 *  8. _sendIndexData() levanta CS entre comando y datos. El protocolo del
 *     display requiere que CS permanezca BAJO durante toda la transacción
 *     (índice + datos). SOLUCIÓN: CS baja al inicio, sube al final.
 *
 *  9. main.cpp tiene un while(1) en medio de main que nunca llega al código
 *     de inicialización real del display. SOLUCIÓN: ver main.cpp corregido.
 *
 * 10. El PPS en PIC32MX795 requiere secuencia de desbloqueo antes de
 *     escribir en los registros RPxR. Sin el unlock, los registros son
 *     de solo lectura y el mapeo de pines no tiene efecto.
 *     SOLUCIÓN: Agregar secuencia SYSKEY unlock/lock alrededor de PPS.
 */

#include "EPD_Driver.h"
#include <string.h>
#include <xc.h>
#include "HardwareProfile.h"

#define FAST_MODE 0
        
//static uint8_t previousBW[5624] = {0};

uint8_t EPD_Driver::tempClearBuffer[frameSize_EPD_266];

/*==============================================================================
 * CORE TIMER
 *============================================================================*/
static inline uint32_t ReadCoreTimer(void) {
    uint32_t count;
    __asm__ volatile("mfc0 %0, $9" : "=r"(count));
    return count;
}

void delayMs(uint32_t ms) {
    uint32_t start = ReadCoreTimer();
    uint32_t ticks = (SYS_FREQ / 2 / 1000) * ms;
    while ((ReadCoreTimer() - start) < ticks);
}

void epd_delayMs(uint32_t ms) {
    delayMs(ms);
}

/*==============================================================================
 * GPIO
 * Esquema de numeración: pin = puerto*32 + bit
 *   PORTA = puerto 0  ? pines  0-15
 *   PORTB = puerto 1  ? pines 32-47   (RB0=32, RB1=33, ..., RB13=45, RB14=46)
 *   PORTC = puerto 2  ? pines 64-79
 *   PORTD = puerto 3  ? pines 96-111
 *   PORTE = puerto 4  ? pines 128-143
 *   PORTF = puerto 5  ? pines 160-175  (RF4=164, RF5=165)
 *   PORTG = puerto 6  ? pines 192-207
 *============================================================================*/
void epd_pinMode(uint8_t pin, uint8_t mode) {
    uint32_t port = pin >> 5;
    uint32_t bit  = 1u << (pin & 0x1F);

    if (mode == 0) {   /* INPUT */
        switch (port) {
            case 1: TRISBSET = bit; break;
            case 2: TRISCSET = bit; break;
            case 3: TRISDSET = bit; break;
            case 4: TRISESET = bit; break;
            case 5: TRISFSET = bit; break;
            case 6: TRISGSET = bit; break;
            default: break;
        }
    } else {           /* OUTPUT */
        switch (port) {
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

void epd_digitalWrite(uint8_t pin, uint8_t value) {
    uint32_t port = pin >> 5;
    uint32_t bit  = 1u << (pin & 0x1F);

    if (value) {
        switch (port) {
            case 1: LATBSET = bit; break;
            case 2: LATCSET = bit; break;
            case 3: LATDSET = bit; break;
            case 4: LATESET = bit; break;
            case 5: LATFSET = bit; break;
            case 6: LATGSET = bit; break;
            default: break;
        }
    } else {
        switch (port) {
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

uint8_t epd_digitalRead(uint8_t pin) {
    uint32_t port = pin >> 5;
    uint32_t bit  = 1u << (pin & 0x1F);

    switch (port) {
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
 * SPI4 ? PINES NATIVOS (sin PPS)
 *
 * En el PIC32MX795F512H/L los pines del SPI4 son FIJOS:
 *   SCK4 = RB14  (pin 29)
 *   SDO4 = RF5   (pin 32)   ? MOSI
 *   SDI4 = RF4   (pin 31)   ? MISO
 *   CS   = RB3   (pin 13)   ? GPIO manual
 *
 * La familia 5XX/6XX/7XX NO tiene CFGCONbits.IOLOCK, SDI4R, ni RPF5R.
 * Esos registros son exclusivos de la familia 1XX/2XX/3XX con PPS nuevo.
 * Solo se necesita configurar TRIS y habilitar el periférico SPI4.
 *============================================================================*/

void spi4_init(void) {
    // --- 1. Liberar Pines JTAG (RB13, RB14, etc.) ---
    // El bit JTAGEN en DDPCON está activado por defecto.
    // Al desactivarlo, los pines TDI (RB13), TDO, TMS y TCK (RB14) se liberan
    // y pueden ser usados como GPIO o por otros periféricos.
    DDPCON = 0x00;   // JTAGEN = 0

    // --- 2. Deshabilitar el ADC en los pines que se van a usar ---
    AD1PCFG = 0xFFFF; // Configura todos los pines como digitales

    // --- 3. Configurar la dirección de los pines (TRISx) ---
    // RB14 (SCK4) como salida
    TRISBCLR = (1 << 14);
    // RF5 (SDO4) como salida
    TRISFCLR = (1 << 5);
    // RF4 (SDI4) como entrada
    TRISFSET = (1 << 4);

    // --- 4. Configurar el Mapeo de Pines (PPS) ---
    // Desbloquear la configuración PPS
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    OSCCONCLR = (1 << 6); // Limpiar IOLOCK

    // Mapeo de SALIDAS usando punteros a direcciones absolutas
    // RPOR7 (0xBF80B34C) controla los pines RP14 y RP15
    volatile uint32_t *RPOR7 = (volatile uint32_t*)0xBF80B34C;
    // RPOR2 (0xBF80B338) controla los pines RP4 y RP5
    volatile uint32_t *RPOR2 = (volatile uint32_t*)0xBF80B338;

    // SCK4 a RB14 (RP14). RP14 está en RPOR7, bits 8-15. Valor de función = 3.
    *RPOR7 = (*RPOR7 & 0x00FF) | (3 << 8);
    // SDO4 a RF5 (RP5). RP5 está en RPOR2, bits 8-15. Valor de función = 4.
    *RPOR2 = (*RPOR2 & 0x00FF) | (4 << 8);

    // Mapeo de ENTRADA usando puntero a dirección absoluta
    // RPINR20 (0xBF80B3A0) controla la entrada SDI4
    volatile uint32_t *RPINR20 = (volatile uint32_t*)0xBF80B3A0;
    // SDI4 desde RF4 (RP4). Valor de entrada = 4.
    *RPINR20 = (*RPINR20 & 0xFFFFFF00) | 4;

    // Bloquear la configuración PPS
    SYSKEY = 0x00000000;

    // --- 5. Configurar el Periférico SPI4 como Master ---
    SPI4CON = 0;                // Reset del módulo SPI4
    SPI4BRG = 399;               // 80 MHz / (2 * (4+1)) = 8 MHz
    SPI4CONbits.MSTEN = 1;     // Habilitar modo Master
    SPI4CONbits.MODE16 = 0;    // Modo de 8 bits
    SPI4CONbits.CKP = 0;       // Clock Idle Low
    SPI4CONbits.CKE = 0;       // Datos cambian en flanco de subida
    SPI4CONbits.SMP = 0;       // Muestrear datos en el medio
    SPI4STATbits.SPIROV = 0;   // Limpiar bandera de overflow
    SPI4CONbits.ON = 1;        // Encender el módulo SPI4
}



 
void spi_init(void) {
    // Liberar JTAG (RB14 es TCK)
    DDPCON = 0x00;
    // Deshabilitar ADC en pines SPI
    AD1PCFG = 0xFFFF;
    // Configurar TRIS
    TRISBCLR = (1u << 14);  // RB14 SCK4 salida
    TRISFCLR = (1u << 5);   // RF5 SDO4 salida
    TRISFSET = (1u << 4);   // RF4 SDI4 entrada
    TRISBCLR = (1u << 3);   // RB3 CS salida
    LATBSET  = (1u << 3);   // CS inactivo
    // Configurar SPI4
    SPI4CON = 0;
    SPI4STAT = 0;
    SPI4BRG = 4;            // 8 MHz
    SPI4CONbits.MSTEN = 1;
    SPI4CONbits.MODE16 = 0;
    SPI4CONbits.CKP = 0;
    SPI4CONbits.CKE = 1;    // Modo 0 (necesario)
    SPI4CONbits.SMP = 0;
    SPI4STATbits.SPIROV = 0;
    SPI4CONbits.ON = 1;
 
}

uint8_t spi_transfer(uint8_t data) {
    while (!SPI4STATbits.SPITBE);  /* Esperar TX buffer vacío */
    SPI4BUF = data;
    while (!SPI4STATbits.SPIRBF);  /* Esperar RX buffer lleno */
    return (uint8_t)SPI4BUF;
}

// ============================================================================
// ENVÍO DE DATOS POR SPI4
// ============================================================================
void spi4_send(uint8_t data) {
    while (!SPI4STATbits.SPITBE); // Esperar a que el buffer de TX esté vacío
    SPI4BUF = data;               // Cargar el dato a enviar
    while (!SPI4STATbits.SPIRBF); // Esperar a que se reciba un dato
    volatile uint8_t dummy = SPI4BUF; // Leer para limpiar la bandera de RX
    (void)dummy;
}


/*==============================================================================
 * REGISTROS DE INICIALIZACIÓN
 *============================================================================*/
const uint8_t register_data_sm[6] = {
    0x00,   /* [0] usado como byte dummy/cero en varios comandos */
    0x0e,   /* [1] soft-reset data */
    0x19,   /* [2] temperatura activa (activar temp interna) */
    0x02,   /* [3] PSR byte 0 */
    0xcf,   /* [4] PSR byte 1 (pantallas pequeñas) */
    0x8d    /* [5] PSR byte 1 alternativo (medianas) */
};

const uint8_t register_data_mid[6] = {
    0x00,
    0x0e,
    0x19,
    0x02,
    0x0f,
    0x89
};

/*==============================================================================
 * CLASE EPD_Driver
 *============================================================================*/
EPD_Driver::EPD_Driver(eScreen_EPD_t screen, pins_t board) {
    spi_basic = board;
    pdi_size  = (uint16_t)(screen >> 8);

    switch (pdi_size) {
        case 0x15: width = 152; height = 152; break;
        case 0x21: width = 212; height = 104; break;
        case 0x26: width = 296; height = 152; break;
        case 0x27: width = 264; height = 176; break;
        case 0x28: width = 296; height = 128; break;
        case 0x37: width = 416; height = 240; break;
        case 0x41: width = 300; height = 400; break;
        case 0x43: width = 480; height = 176; break;
        default:   width = 0;   height = 0;   break;
    }

    /* BUG #5 CORREGIDO: fórmula correcta para frame buffer */
    image_data_size = ((uint32_t)width * (uint32_t)height + 7u) / 8u;

    memcpy(register_data, register_data_sm, sizeof(register_data_sm));
}

void EPD_Driver::COG_initial() {
    /* Configurar pines de control */
    epd_pinMode(spi_basic.panelBusy,  0);   /* BUSY  = entrada */
    epd_pinMode(spi_basic.panelDC,    1);   /* DC    = salida  */
    epd_pinMode(spi_basic.panelReset, 1);   /* RESET = salida  */
    epd_pinMode(spi_basic.panelCS,    1);   /* CS    = salida  */

    /* Estado inicial */
    epd_digitalWrite(spi_basic.panelDC,    1);
    epd_digitalWrite(spi_basic.panelReset, 1);
    epd_digitalWrite(spi_basic.panelCS,    1);

    if (spi_basic.panelON_EXT2 != NOT_CONNECTED) {
        epd_pinMode(spi_basic.panelON_EXT2,    1);
        epd_pinMode(spi_basic.panelSPI43_EXT2, 1);
        epd_digitalWrite(spi_basic.panelON_EXT2,    1);
        epd_digitalWrite(spi_basic.panelSPI43_EXT2, 0);
    }

    spi_init();
    epd_delayMs(5);

    /* Secuencia de reset hardware */
    _reset(1, 5, 10, 5, 1);

    /* Soft-reset y espera BUSY */
    _softReset();

    /* Configurar temperatura: 25°C */
    uint8_t temp = 25;
    _sendIndexData(0xE5, &temp, 1);                    /* Temperatura input */
    _sendIndexData(0xE0, &register_data[2], 1);        /* Activar temp interna */
    _sendIndexData(0x00, &register_data[3], 2);        /* PSR: bytes [3] y [4] */
}

void EPD_Driver::globalUpdate(const uint8_t* data1s, const uint8_t* data2s) {
    _sendIndexData(0x10, data1s, image_data_size);  /* Frame BW */
    _sendIndexData(0x13, data2s, image_data_size);  /* Frame Red / vacío */
    _DCDC_powerOn();
    _displayRefresh();
}

void EPD_Driver::COG_powerOff() {
    uint8_t dummy = 0x00;
    _sendIndexData(0x02, &dummy, 0);  /* Power off */

    uint32_t timeout = 5000;
    while (epd_digitalRead(spi_basic.panelBusy) != 1 && timeout > 0) {
        epd_delayMs(1);
        timeout--;
    }

    epd_digitalWrite(spi_basic.panelDC,    0);
    epd_digitalWrite(spi_basic.panelCS,    0);
    /* BUG #7 CORREGIDO: NO escribir en panelBusy (es entrada) */
    epd_delayMs(150);
    epd_digitalWrite(spi_basic.panelReset, 0);
}

/*
 * BUG #8 CORREGIDO: CS debe permanecer LOW durante todo el frame
 * (comando + datos). La versión original subía CS entre comando y datos,
 * lo que genera dos transacciones SPI separadas. El controlador UC8171
 * espera una única transacción continua: [índice][datos...] con CS bajo.
 */
void EPD_Driver::_sendIndexData(uint8_t index, const uint8_t* data, uint32_t len) {
    /* DC = LOW ? modo comando */
    epd_digitalWrite(spi_basic.panelDC, 0);
    epd_digitalWrite(spi_basic.panelCS, 0);
    spi_transfer(index);

    if (len > 0) {
        /* DC = HIGH ? modo datos (CS sigue bajo) */
        epd_digitalWrite(spi_basic.panelDC, 1);
        for (uint32_t i = 0; i < len; i++) {
            spi_transfer(data[i]);
        }
    }

    /* Finalizar transacción */
    epd_digitalWrite(spi_basic.panelCS, 1);
}

void EPD_Driver::_softReset() {
    /*
     * Comando 0x00 con dato 0x0e = soft-reset del UC8171.
     * register_data[1] = 0x0e (correcto).
     */
    _sendIndexData(0x00, &register_data[1], 1);

    uint32_t timeout = 5000;
    while (epd_digitalRead(spi_basic.panelBusy) != 1 && timeout > 0) {
        epd_delayMs(1);
        timeout--;
    }

    if (timeout == 0) {
        /* Error: parpadeo rápido de LED0 indicando fallo en BUSY */
        while (1) {
            LED0_ON();
            delayMs(150);
            LED0_OFF();
            delayMs(150);
        }
    }
}

void EPD_Driver::_displayRefresh() {
    uint8_t dummy = 0x00;
    _sendIndexData(0x12, &dummy, 0);  /* Display refresh ? sin datos */

    uint32_t timeout = 20000;  /* Hasta 20 s para refresh completo */
    while (epd_digitalRead(spi_basic.panelBusy) != 1 && timeout > 0) {
        epd_delayMs(1);
        timeout--;
    }
}

void EPD_Driver::_reset(uint32_t ms1, uint32_t ms2, uint32_t ms3,
                         uint32_t ms4, uint32_t ms5) {
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
    uint8_t dummy = 0x00;
    _sendIndexData(0x04, &dummy, 0);  /* Power on ? sin datos adicionales */

    uint32_t timeout = 5000;
    while (epd_digitalRead(spi_basic.panelBusy) != 1 && timeout > 0) {
        epd_delayMs(1);
        timeout--;
    }
}

/*==============================================================================
 * FUNCIONES DE TEST DE PINES
 * NOTA: Usar el esquema correcto de numeración (puerto*32 + bit):
 *   RB0=32, RB1=33, RB2=34, RB3=35
 *============================================================================*/
void test_pines_display(uint16_t repeticiones, uint16_t tiempo_ms) {
    epd_pinMode(32, 1);  /* RB0 */
    epd_pinMode(33, 1);  /* RB1 */
    epd_pinMode(34, 1);  /* RB2 */
    epd_pinMode(35, 1);  /* RB3 */

    epd_digitalWrite(32, 0);
    epd_digitalWrite(33, 0);
    epd_digitalWrite(34, 0);
    epd_digitalWrite(35, 0);

    for (uint16_t r = 0; r < repeticiones; r++) {
        epd_digitalWrite(32, 1); delayMs(tiempo_ms); epd_digitalWrite(32, 0); delayMs(tiempo_ms);
        epd_digitalWrite(33, 1); delayMs(tiempo_ms); epd_digitalWrite(33, 0); delayMs(tiempo_ms);
        epd_digitalWrite(34, 1); delayMs(tiempo_ms); epd_digitalWrite(34, 0); delayMs(tiempo_ms);
        epd_digitalWrite(35, 1); delayMs(tiempo_ms); epd_digitalWrite(35, 0); delayMs(tiempo_ms);
    }
}

void test_pines_simultaneos(uint16_t tiempo_ms) {
    epd_pinMode(32, 1); epd_pinMode(33, 1); epd_pinMode(34, 1); epd_pinMode(35, 1);
    epd_digitalWrite(32, 1); epd_digitalWrite(33, 1);
    epd_digitalWrite(34, 1); epd_digitalWrite(35, 1);
    delayMs(tiempo_ms);
    epd_digitalWrite(32, 0); epd_digitalWrite(33, 0);
    epd_digitalWrite(34, 0); epd_digitalWrite(35, 0);
    delayMs(tiempo_ms);
}

void test_pines_carrusel(uint16_t repeticiones, uint16_t tiempo_ms) {
    epd_pinMode(32, 1); epd_pinMode(33, 1); epd_pinMode(34, 1); epd_pinMode(35, 1);
    epd_digitalWrite(32, 0); epd_digitalWrite(33, 0);
    epd_digitalWrite(34, 0); epd_digitalWrite(35, 0);

    for (uint16_t r = 0; r < repeticiones; r++) {
        epd_digitalWrite(32, 1); delayMs(tiempo_ms); epd_digitalWrite(32, 0);
        epd_digitalWrite(33, 1); delayMs(tiempo_ms); epd_digitalWrite(33, 0);
        epd_digitalWrite(34, 1); delayMs(tiempo_ms); epd_digitalWrite(34, 0);
        epd_digitalWrite(35, 1); delayMs(tiempo_ms); epd_digitalWrite(35, 0);
    }
}


void EPD_Driver::fastUpdate(const uint8_t* prevBW, const uint8_t* nextBW) {
    // Configurar registros para fast update (según PDLS)
    uint8_t fast_psr0 = register_data[3] | 0x10;   // PSR0 bit 4 = 1
    uint8_t fast_psr1 = register_data[4] | 0x02;   // PSR1 bit 1 = 1
    _sendIndexData(0x00, &fast_psr0, 2);
    
    // Ajuste de Vcom (mejora la calidad del fast update)
    uint8_t vcom = 0x07;
    _sendIndexData(0x50, &vcom, 1);
    
    // Enviar frame anterior (0x10) y frame nuevo (0x13)
    _sendIndexData(0x10, prevBW, image_data_size);
    _sendIndexData(0x13, nextBW, image_data_size);
    
    _DCDC_powerOn();
    
    // Refresco rápido global (modo 0x01) ? sin datos adicionales
    uint8_t refresh_mode = 0x01;
    _sendIndexData(0x12, &refresh_mode, 1);
    
    while (epd_digitalRead(spi_basic.panelBusy) != 1);
    
    // Copiar el frame nuevo al anterior para la próxima actualización
    memcpy((void*)prevBW, (void*)nextBW, image_data_size);
}


/*
void EPD_Driver::fastUpdate(const uint8_t* data1s, const uint8_t* data2s) {
    _sendIndexData(0x10, data1s, image_data_size);
    _sendIndexData(0x13, data2s, image_data_size);
    _DCDC_powerOn();
    
    uint8_t fast_mode = 0x01;   // modo rápido completo (no parcial)
    _sendIndexData(0x12, &fast_mode, 1);
    
    while (epd_digitalRead(spi_basic.panelBusy) != 1);
}
*/

void EPD_Driver::setWindow(uint16_t x_start, uint16_t y_start, 
                           uint16_t x_end, uint16_t y_end) {
    // Convertir coordenadas a bytes (cada byte son 8 píxeles en X)
    uint16_t x_start_byte = x_start / 8;
    uint16_t x_end_byte   = (x_end - 1) / 8;
    uint16_t y_start_row  = y_start;
    uint16_t y_end_row    = y_end - 1;
    
    uint8_t x_data[4];
    x_data[0] = x_start_byte;
    x_data[1] = x_start_byte >> 8;
    x_data[2] = x_end_byte;
    x_data[3] = x_end_byte >> 8;
    _sendIndexData(0x44, x_data, 4);
    
    uint8_t y_data[4];
    y_data[0] = y_start_row;
    y_data[1] = y_start_row >> 8;
    y_data[2] = y_end_row;
    y_data[3] = y_end_row >> 8;
    _sendIndexData(0x45, y_data, 4);
}
 

void EPD_Driver::partialUpdate(const uint8_t* data1s, const uint8_t* data2s,
                               uint16_t x_start, uint16_t y_start,
                               uint16_t x_end, uint16_t y_end) {
    // Establecer ventana
    setWindow(x_start, y_start, x_end, y_end);
    
    // Enviar solo los datos de la ventana (necesitas calcular el tamaño)
    // Envía datos de la ventana con el comando 0x24 (BW) y 0x26 (Red)
    uint32_t bytes_per_row = ((x_end - x_start) + 7) / 8;
    uint32_t total_bytes = bytes_per_row * (y_end - y_start);
    
    // Extraer los datos de la región de los buffers originales (data1s, data2s)
    // Esta extracción depende de cómo almacenes tus datos (column-major).
    // Aquí asumimos que el buffer está en column-major (como en tu código).
    // Necesitarás una función que copie la región.
    
    // Por simplicidad, enviamos toda la pantalla pero con ventana activa,
    // el controlador solo actualizará esa zona.
    _sendIndexData(0x24, data1s, total_bytes);  // BW data for window
    _sendIndexData(0x26, data2s, total_bytes);  // Red data for window
    
    // Fast update dentro de la ventana
    uint8_t cmd = 0x12;
    _sendIndexData(0x22, &cmd, 1);
    _sendIndexData(0x20, &cmd, 0);
    
    while (epd_digitalRead(spi_basic.panelBusy) != 1);
}


void EPD_Driver::clearScreenFast(bool white) {
    uint8_t color = white ? 0x00 : 0xFF;
    memset(tempClearBuffer, color, image_data_size);
    fastUpdate(tempClearBuffer, tempClearBuffer);
}

void EPD_Driver::partialUpdateWindow(const uint8_t* dataBW, const uint8_t* dataRed,
                                     uint16_t x_start, uint16_t y_start,
                                     uint16_t x_end, uint16_t y_end) {
    setWindow(x_start, y_start, x_end, y_end);
    
    uint16_t w = x_end - x_start;
    uint16_t h = y_end - y_start;
    uint32_t bytes_per_row = (w + 7) / 8;
    uint32_t total_bytes = bytes_per_row * h;
    
    _sendIndexData(0x24, dataBW, total_bytes);   // imagen BW
    _sendIndexData(0x26, dataRed, total_bytes);  // imagen roja (0 si no se usa)
    
    // Comando de refresh con modo parcial (0x02 = fast partial)
    uint8_t refresh_mode = 0x02;
    _sendIndexData(0x12, &refresh_mode, 1);
    
    // Esperar a que termine
    while (epd_digitalRead(spi_basic.panelBusy) != 1);
}

// En EPD_Driver.cpp
void EPD_Driver::fastFullUpdate(const uint8_t* dataBW, const uint8_t* dataRed) {
    _sendIndexData(0x10, dataBW, image_data_size);
    _sendIndexData(0x13, dataRed, image_data_size);
    _DCDC_powerOn();
    uint8_t mode = 0x01;   // fast full refresh
    _sendIndexData(0x12, &mode, 1);
    while (epd_digitalRead(spi_basic.panelBusy) != 1);
}