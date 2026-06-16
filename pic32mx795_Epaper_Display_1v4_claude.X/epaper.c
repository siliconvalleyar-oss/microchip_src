////////////////////////////////////////////////////////////////////////////////
//
//   filename        : epaper.c
//   License         : GNU
//   Author          : Lio
//   Hardware        : PIC32MX795F512H
//   Compiler        : XC32 / MPLAB X
//
//   Referencia oficial: PDI Datasheet Rev.02 (2022/06/06)
//   Panel: 2.66" EPD, 296×152 píxeles, COG driver PDI
//
// ─────────────────────────────────────────────────────────────────────────────
//  ANÁLISIS DEL DATASHEET vs CÓDIGO ANTERIOR — CORRECCIONES APLICADAS
// ─────────────────────────────────────────────────────────────────────────────
//
//  1. SPI TIMING (pág. 16)
//     Datasheet: velocidad máxima = 10 MHz (tSCYCW min = 100 ns)
//     Código anterior: SPI4BRG=1 → 20 MHz ← VIOLA la especificación
//     CORRECCIÓN: SPI4BRG=3 → 80/(2×4) = 10 MHz exactos ✓
//
//  2. SPI FORMAT (pág. 15) — ¡CRÍTICO!
//     Datasheet: "CS is pulled HIGH then LOW between clocks"
//                "If register data is more than one byte, the CS# pulse
//                 is necessary between each data byte"
//     Código anterior: CS bajaba una vez para TODOS los bytes del bloque
//     CORRECCIÓN: CS sube y baja entre cada byte de datos ✓
//
//  3. SECUENCIA DE POWER-ON COG (pág. 17)
//     Datasheet exacto:
//       Estado inicial: VCC/VDD, RES#, CS#, SDIN, SCLK = 0
//       Turn-on VCC/VDD
//       Delay 5ms
//       RES# = 1
//       Delay 5ms
//       RES# = 0
//       Delay 10ms
//       RES# = 1
//       Delay 5ms
//       Soft-reset SPI(0x00, 0x0E)   ← dato es 0x0E, no 0x0F
//       Delay 5ms
//     Código anterior: usaba 0x0F como dato del soft-reset ← INCORRECTO
//     CORRECCIÓN: SPI(0x00, 0x0E) ✓
//
//  4. TEMPERATURA Y PSR (pág. 18)
//     Datasheet:
//       SPI(0xe5, TSSET)    ← temperatura: 25°C = 0x19
//       SPI(0xe0, 0x02)     ← activar temperatura
//       SPI(0x00, PSR)      ← PSR para 2.66": 0xCF, 0x8D  (Other Size)
//     Código anterior: register_data_sm[2]=0x19 ✓, [3]=0x02 ✓, [4..5]=0xCF,0x8D ✓
//     Estado: CORRECTO — se mantiene igual
//
//  5. INPUT IMAGE (pág. 19-20)
//     Datasheet:
//       Frame 1 (cmd 0x10): dato 1=NEGRO, dato 0=BLANCO
//       Frame 2 (cmd 0x13): K bytes de 0x00 (todos ceros)
//     Código anterior: mandaba el mismo buffer en Frame1 y Frame2
//     CORRECCIÓN:
//       - Frame 1: buffer del usuario (1=negro, 0=blanco) — SIN invertir ✓
//       - Frame 2: 5624 bytes de 0x00 ✓
//     NOTA CRÍTICA sobre la inversión:
//       El buffer interno usa 1=negro, 0=blanco → IGUAL que el datasheet
//       NO hay inversión necesaria. El código anterior que hacía ~byte
//       estaba EQUIVOCADO. Se elimina.
//
//  6. SEND UPDATE (pág. 21)
//     SPI(0x04) → esperar BUSY=HIGH → SPI(0x12) → esperar BUSY=HIGH ✓
//
//  7. TURN-OFF DC/DC (pág. 22)
//     SPI(0x02) → esperar BUSY=HIGH → RES# flotante → CS#,SDIN,SCLK=LOW
//     Código anterior: ponía RES#=LOW (correcto), DC=LOW, CS=LOW ✓
//
//  8. POLARIDAD DE BUSY
//     Datasheet: "Make sure BUSY = High" antes de cada paso
//     El pin BUSY es activo en HIGH cuando está LIBRE (listo)
//     Código: while(BUSY != HIGH) → espera a que esté listo ✓
//
// ─────────────────────────────────────────────────────────────────────────────
//
//   Pin mapping:
//      Panel_CS   -> RB3   (manual CS para SPI4)
//      Flash_CS   -> RB13
//      MOSI       -> RF5   (SPI4 SDO4)
//      MISO       -> RF4   (SPI4 SDI4)
//      RST (RES#) -> RB2
//      D/C        -> RB1
//      BUSY       -> RB0   (input)
//      SCK        -> RB14  (SPI4 SCK4)
//
////////////////////////////////////////////////////////////////////////////////

#include "epaper.h"
#include <string.h>

// ─── Clock del sistema ────────────────────────────────────────────────────────
#ifndef SYS_FREQ
#define SYS_FREQ  80000000UL
#endif

// ─── Datos de registro PDI (pág. 18) ─────────────────────────────────────────
//  [0] = padding (no se usa como dato útil)
//  [1] = 0x0E  ← Soft-reset data (datasheet pág.17: SPI(0x00, 0x0E))
//  [2] = 0x19  ← Temperatura 25°C (pág.18: 25°C = 0x19)
//  [3] = 0x02  ← Active temperature (pág.18: SPI(0xe0, 0x02))
//  [4] = 0xCF  ← PSR byte 1 para "Other Size" (2.66" = Other) (pág.18)
//  [5] = 0x8D  ← PSR byte 2
const uint8_t register_data_sm[6]  = { 0x00, 0x0E, 0x19, 0x02, 0xCF, 0x8D };
// Para pantallas 3.7", 4.2", 4.37": PSR = 0x0F, 0x89
const uint8_t register_data_mid[6] = { 0x00, 0x0E, 0x19, 0x02, 0x0F, 0x89 };

// ─── Frame 2: debe ser todo 0x00 según datasheet pág. 20 ─────────────────────
// "The second frame need to be fed K bytes 0x00"
// Se declara estático para no usar el stack (5624 bytes es mucho para stack)
static const uint8_t s_frame2_zeros[296 * 152 / 8] = { 0 };  // 5624 bytes de 0x00

// ─── Configuración del tablero ────────────────────────────────────────────────
const pins_t boardPIC32MX795 = {
    .panelBusy        = PIN_BUSY,
    .panelDC          = PIN_DC,
    .panelReset       = PIN_RST,
    .panelCS          = PIN_PANEL_CS,
    .panelON_EXT2     = NOT_CONNECTED,
    .panelSPI43_EXT2  = NOT_CONNECTED,
    .flashCS          = PIN_FLASH_CS
};

// ─────────────────────────────────────────────────────────────────────────────
//  HAL GPIO
// ─────────────────────────────────────────────────────────────────────────────

void HAL_GPIO_Init(void) {
    // RB0=BUSY(IN), RB1=D/C(OUT), RB2=RES#(OUT), RB3=CS(OUT)
    // RB13=FlashCS(OUT), RB14=SCK4(periférico SPI4)
    TRISBbits.TRISB0  = 1;
    TRISBbits.TRISB1  = 0;
    TRISBbits.TRISB2  = 0;
    TRISBbits.TRISB3  = 0;
    TRISBbits.TRISB13 = 0;
    TRISBbits.TRISB14 = 0;

    // Todos los pines del puerto B en modo digital
    AD1PCFG = 0xFFFF;

    // RF4=MISO(SDI4), RF5=MOSI(SDO4)
    TRISFbits.TRISF4 = 1;
    TRISFbits.TRISF5 = 0;

    // Estado inicial según datasheet pág.17 nota 1:
    // "Initial State: VCC/VDD, RES#, CS#, SDIN, SCLK = 0"
    // → todos en LOW al inicio, luego el COG_initial los sube
    LATBbits.LATB1  = 0;    // D/C  = LOW
    LATBbits.LATB2  = 0;    // RES# = LOW (estado inicial)
    LATBbits.LATB3  = 0;    // CS#  = LOW (estado inicial)
    LATBbits.LATB13 = 1;    // FlashCS = HIGH (no usado)
}

void HAL_GPIO_Write(uint8_t pin, uint8_t value) {
    if (pin == NOT_CONNECTED) return;
    uint8_t port = PIN_PORT(pin);
    uint8_t bit  = PIN_BIT(pin);
    switch (port) {
        case PORT_B:
            if (value) LATBSET = (1u << bit);
            else        LATBCLR = (1u << bit);
            break;
        case PORT_F:
            if (value) LATFSET = (1u << bit);
            else        LATFCLR = (1u << bit);
            break;
    }
}

uint8_t HAL_GPIO_Read(uint8_t pin) {
    if (pin == NOT_CONNECTED) return 0;
    uint8_t port = PIN_PORT(pin);
    uint8_t bit  = PIN_BIT(pin);
    switch (port) {
        case PORT_B: return (PORTB >> bit) & 1u;
        case PORT_F: return (PORTF >> bit) & 1u;
        default:     return 0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  HAL SPI4
//  Datasheet pág.16: velocidad máxima = 10 MHz (tSCYCW min = 100 ns)
//  SPI4BRG = PBCLK/(2×SPI_freq) - 1 = 80MHz/(2×10MHz) - 1 = 3
// ─────────────────────────────────────────────────────────────────────────────

void HAL_SPI4_Init(void) {
    SPI4CON = 0;
    volatile uint32_t rData = SPI4BUF;
    (void)rData;

    SPI4BRG = 3;            // 10 MHz con PBCLK=80 MHz ← máximo del datasheet

    SPI4CONbits.MSTEN  = 1; // Maestro
    SPI4CONbits.MODE16 = 0; // 8 bits
    SPI4CONbits.MODE32 = 0;
    SPI4CONbits.CKP    = 0; // CPOL=0: reloj idle LOW
    SPI4CONbits.CKE    = 1; // CPHA=0: datos válidos en flanco de subida
    SPI4CONbits.SMP    = 0;
    SPI4CONbits.DISSDO = 0;
    SPI4CONbits.ON     = 1;
}

uint8_t HAL_SPI4_Transfer(uint8_t data) {
    while (SPI4STATbits.SPIBUSY);
    SPI4BUF = data;
    while (!SPI4STATbits.SPIRBF);
    return (uint8_t)SPI4BUF;
}

// ─────────────────────────────────────────────────────────────────────────────
//  HAL Delay — CoreTimer preciso, sin overflow
// ─────────────────────────────────────────────────────────────────────────────

void HAL_Delay_ms(uint32_t ms) {
    uint32_t ticksPerMs = SYS_FREQ / 2 / 1000;  // 40.000 ticks/ms
    while (ms--) {
        uint32_t start = _CP0_GET_COUNT();
        while ((_CP0_GET_COUNT() - start) < ticksPerMs);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  SPI send — formato EXACTO del datasheet pág.15
//
//  "CS is pulled HIGH then LOW between clocks"
//  "If register data is more than one byte, the CS# pulse is
//   necessary between each data byte"
//
//  Secuencia por comando:
//    D/C=0, CS=0 → [index] → CS=1
//    D/C=1, CS=0 → [data0] → CS=1
//    D/C=1, CS=0 → [data1] → CS=1
//    ...
// ─────────────────────────────────────────────────────────────────────────────

static void _spi_send_index(EPD_Driver_t *drv, uint8_t index) {
    HAL_GPIO_Write(drv->pin_cfg.panelDC, LOW);
    HAL_GPIO_Write(drv->pin_cfg.panelCS, LOW);
    HAL_SPI4_Transfer(index);
    HAL_GPIO_Write(drv->pin_cfg.panelCS, HIGH);
}

static void _spi_send_data_byte(EPD_Driver_t *drv, uint8_t data) {
    HAL_GPIO_Write(drv->pin_cfg.panelDC, HIGH);
    HAL_GPIO_Write(drv->pin_cfg.panelCS, LOW);
    HAL_SPI4_Transfer(data);
    HAL_GPIO_Write(drv->pin_cfg.panelCS, HIGH);
}

// Envía: índice + N bytes de datos (CS pulsea entre cada byte)
static void _sendIndexData(EPD_Driver_t *drv,
                            uint8_t index,
                            const uint8_t *data,
                            uint32_t len)
{
    _spi_send_index(drv, index);
    for (uint32_t i = 0; i < len; i++)
        _spi_send_data_byte(drv, data[i]);
}

// Envía un frame completo de imagen (con CS pulsando entre cada byte)
// Frame 1: datos del usuario tal cual (1=negro, 0=blanco — igual que datasheet)
static void _sendFrame(EPD_Driver_t *drv,
                       uint8_t cmd,
                       const uint8_t *data,
                       uint32_t len)
{
    _spi_send_index(drv, cmd);
    for (uint32_t i = 0; i < len; i++)
        _spi_send_data_byte(drv, data[i]);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Power-on COG — secuencia exacta del datasheet pág.17
// ─────────────────────────────────────────────────────────────────────────────

static void _cog_power_on(EPD_Driver_t *drv) {
    // Estado inicial: todos los pines a LOW (ya hecho en HAL_GPIO_Init)
    // Aquí asumimos que VCC/VDD ya está activo

    // RES# = 1
    HAL_GPIO_Write(drv->pin_cfg.panelReset, HIGH);
    HAL_Delay_ms(5);

    // RES# = 0
    HAL_GPIO_Write(drv->pin_cfg.panelReset, LOW);
    HAL_Delay_ms(10);

    // RES# = 1
    HAL_GPIO_Write(drv->pin_cfg.panelReset, HIGH);
    HAL_Delay_ms(5);

    // CS# = HIGH antes del soft-reset
    HAL_GPIO_Write(drv->pin_cfg.panelCS, HIGH);

    // Soft-reset: SPI(0x00, 0x0E)   ← datasheet pág.17
    {
        uint8_t soft_rst = 0x0E;
        _sendIndexData(drv, 0x00, &soft_rst, 1);
    }
    HAL_Delay_ms(5);
}

// ─────────────────────────────────────────────────────────────────────────────
//  API pública
// ─────────────────────────────────────────────────────────────────────────────

void EPD_Init(EPD_Driver_t *drv, uint32_t eScreen_EPD, const pins_t *board) {
    drv->pin_cfg  = *board;
    drv->pdi_cp   = (uint16_t) eScreen_EPD;
    drv->pdi_size = (uint16_t)(eScreen_EPD >> 8);

    uint16_t sV = 0, sH = 0;
    switch (drv->pdi_size) {
        case 0x15: sV=152; sH=152; break;
        case 0x21: sV=212; sH=104; break;
        case 0x26: sV=296; sH=152; break;  // 2.66" ← nuestro panel
        case 0x27: sV=264; sH=176; break;
        case 0x28: sV=296; sH=128; break;
        case 0x37: sV=416; sH=240; break;
        case 0x41: sV=300; sH=400; break;
        case 0x43: sV=480; sH=176; break;
        default:   sV=296; sH=152; break;
    }
    drv->image_data_size = (uint32_t)sV * (uint32_t)(sH / 8);

    // PSR según datasheet pág.18:
    // 2.66" = "Other Size" → 0xCF, 0x8D
    memcpy(drv->register_data, register_data_sm, sizeof(register_data_sm));

    HAL_GPIO_Init();
    HAL_SPI4_Init();
}

void EPD_COG_initial(EPD_Driver_t *drv) {
    // Paso 1: Power-on COG (pág.17)
    _cog_power_on(drv);

    // Paso 2: Temperatura y PSR (pág.18)
    // SPI(0xe5, 0x19)  → temperatura 25°C
    _sendIndexData(drv, 0xe5, &drv->register_data[2], 1);

    // SPI(0xe0, 0x02)  → activar temperatura
    _sendIndexData(drv, 0xe0, &drv->register_data[3], 1);

    // SPI(0x00, 0xCF, 0x8D)  → PSR para 2.66"
    // Nota: el comando 0x00 es el mismo que el soft-reset,
    // pero con datos PSR sobreescribe la configuración del panel
    _sendIndexData(drv, 0x00, &drv->register_data[4], 2);

    drv->v_screenSizeV = 296;
    drv->v_screenSizeH = 152;
}

void EPD_globalUpdate(EPD_Driver_t *drv,
                      const uint8_t *data1s,
                      const uint8_t *data2s)
{
    (void)data2s;  // Frame 2 SIEMPRE es 0x00 según datasheet pág.20

    // Frame 1 (cmd 0x10): imagen real — 1=negro, 0=blanco (pág.20)
    // El buffer del usuario ya tiene 1=negro por convención → sin invertir
    _sendFrame(drv, 0x10, data1s, drv->image_data_size);

    // Frame 2 (cmd 0x13): K bytes de 0x00 (pág.20)
    _sendFrame(drv, 0x13, s_frame2_zeros, drv->image_data_size);

    // Power on DC/DC (pág.21): SPI(0x04) + esperar BUSY=HIGH
    {
        uint8_t dummy = 0x00;
        _sendIndexData(drv, 0x04, &dummy, 0);
        while (HAL_GPIO_Read(drv->pin_cfg.panelBusy) != HIGH);
    }

    // Display Refresh (pág.21): SPI(0x12) + esperar BUSY=HIGH
    {
        uint8_t dummy = 0x00;
        _sendIndexData(drv, 0x12, &dummy, 0);
        while (HAL_GPIO_Read(drv->pin_cfg.panelBusy) != HIGH);
    }
}

void EPD_COG_powerOff(EPD_Driver_t *drv) {
    // Turn-off DC/DC (pág.22): SPI(0x02) + esperar BUSY=HIGH
    {
        uint8_t dummy = 0x00;
        _sendIndexData(drv, 0x02, &dummy, 0);
        while (HAL_GPIO_Read(drv->pin_cfg.panelBusy) != HIGH);
    }

    // Set RES# to floating (pág.22) → ponemos LOW como estado seguro
    HAL_GPIO_Write(drv->pin_cfg.panelReset, LOW);

    // Clear CS#, SDIN and SCLK to low level (pág.22)
    HAL_GPIO_Write(drv->pin_cfg.panelDC, LOW);
    HAL_GPIO_Write(drv->pin_cfg.panelCS, LOW);
    // SDIN y SCLK los controla el periférico SPI4; deshabilitarlo:
    SPI4CONbits.ON = 0;
}

#ifdef EPD_DEBUG_PRINT
extern void UART_Print(const char *s);
void EPD_printGpios(const EPD_Driver_t *drv) {
    UART_Print("=== EPD GPIO (PDI 2.66\") ===\r\n");
    UART_Print("BUSY=RB0 DC=RB1 RES=RB2 CS=RB3\r\n");
    UART_Print("MOSI=RF5 MISO=RF4 SCK=RB14 @ 10MHz\r\n");
    (void)drv;
}
#else
void EPD_printGpios(const EPD_Driver_t *drv) { (void)drv; }
#endif
