////////////////////////////////////////////////////////////////////////////////
//
//   filename        : epaper.h
//   License         : GNU
//   Author          : Lio
//   Ported to       : PIC32MX795F512H - 64 pines
//   Compiler        : XC32 / MPLAB X
//   Dependencies    : plib.h / xc.h
//
//   Pin mapping:
//      Panel_CS   -> RB3   (SPI4 CS  - ECSM)
//      Flash_CS   -> RB13  (FCSM)
//      MOSI       -> RF5   (SPI4 SDO4)
//      MISO       -> RF4   (SPI4 SDI4)
//      RST        -> RB2
//      D/C        -> RB1
//      BUSY       -> RB0   (input)
//      SCK        -> RB14  (SPI4 SCK4)
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ─── Nivel lógico ────────────────────────────────────────────────────────────
#ifndef HIGH
#define HIGH 1
#endif
#ifndef LOW
#define LOW  0
#endif

// ─── Sentinel "no conectado" ──────────────────────────────────────────────────
#define NOT_CONNECTED 0xFF

// ─── Tipos de pantalla E-Paper ────────────────────────────────────────────────
#define eScreen_EPD_154  ((uint32_t)0x1509)
#define eScreen_EPD_213  ((uint32_t)0x2100)
#define eScreen_EPD_266  ((uint32_t)0x2600)
#define eScreen_EPD_271  ((uint32_t)0x2700)
#define eScreen_EPD_287  ((uint32_t)0x2800)
#define eScreen_EPD_370  ((uint32_t)0x3700)
#define eScreen_EPD_417  ((uint32_t)0x4100)
#define eScreen_EPD_437  ((uint32_t)0x430C)

// ─── Mapa de pines (codificado como puerto<<8 | bit) ──────────────────────────
//   Puerto B => 0xB0 | bit
//   Puerto F => 0xF0 | bit
#define PIN_PORT(p)  ((p) >> 4)
#define PIN_BIT(p)   ((p) & 0x0F)

#define MAKE_PIN(port, bit)  (((port) << 4) | (bit))

// Puertos disponibles
#define PORT_B  0xB
#define PORT_F  0xF

// Pines concretos del diseño
#define PIN_PANEL_CS    MAKE_PIN(PORT_B, 3)   // RB3  - SPI CS
#define PIN_FLASH_CS    MAKE_PIN(PORT_B, 13)  // RB13 - Flash CS
#define PIN_RST         MAKE_PIN(PORT_B, 2)   // RB2  - Reset
#define PIN_DC          MAKE_PIN(PORT_B, 1)   // RB1  - D/C
#define PIN_BUSY        MAKE_PIN(PORT_B, 0)   // RB0  - BUSY (input)
// MOSI/MISO/SCK son controlados por el periférico SPI4, no por GPIO directo

// ─── Datos de registro ────────────────────────────────────────────────────────
extern const uint8_t register_data_mid[6];
extern const uint8_t register_data_sm[6];

// ─── Estructura de configuración de pines ────────────────────────────────────
typedef struct {
    uint8_t panelBusy;        // PIN_BUSY  (o NOT_CONNECTED)
    uint8_t panelDC;          // PIN_DC
    uint8_t panelReset;       // PIN_RST
    uint8_t panelCS;          // PIN_PANEL_CS
    uint8_t panelON_EXT2;     // NOT_CONNECTED en este diseño
    uint8_t panelSPI43_EXT2;  // NOT_CONNECTED en este diseño
    uint8_t flashCS;          // PIN_FLASH_CS
} pins_t;

// ─── Tablero por defecto ──────────────────────────────────────────────────────
extern const pins_t boardPIC32MX795;

// ─── Prototipos de bajo nivel (HAL) ──────────────────────────────────────────
void     HAL_GPIO_Init    (void);
void     HAL_GPIO_Write   (uint8_t pin, uint8_t value);
uint8_t  HAL_GPIO_Read    (uint8_t pin);
void     HAL_SPI4_Init    (void);
uint8_t  HAL_SPI4_Transfer(uint8_t data);
void     HAL_Delay_ms     (uint32_t ms);

// ─── Driver EPD ──────────────────────────────────────────────────────────────
typedef struct {
    pins_t   pin_cfg;
    uint16_t pdi_size;
    uint16_t pdi_cp;
    uint32_t image_data_size;
    uint8_t  register_data[6];
    uint16_t v_screenSizeV;
    uint16_t v_screenSizeH;
} EPD_Driver_t;

void    EPD_Init          (EPD_Driver_t *drv, uint32_t eScreen_EPD, const pins_t *board);
void    EPD_COG_initial   (EPD_Driver_t *drv);
void    EPD_COG_powerOff  (EPD_Driver_t *drv);
void    EPD_globalUpdate  (EPD_Driver_t *drv,
                           const uint8_t *data1s,
                           const uint8_t *data2s);
void    EPD_printGpios    (const EPD_Driver_t *drv);
