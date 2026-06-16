/**
 * @file    EPD_Configuration.h
 * @brief   Configuración de pantallas E-Paper Pervasive Displays
 */

#ifndef EPD_CONFIGURATION_H
#define EPD_CONFIGURATION_H

#include <stdint.h>

#define SYS_FREQ        80000000UL
#define PB_FREQ         80000000UL
#define EPD_SPI_CLOCK   8000000UL

/*==============================================================================
 * 1. PANTALLAS SOPORTADAS
 *============================================================================*/

typedef uint32_t eScreen_EPD_t;

#define eScreen_EPD_154     0x1509
#define eScreen_EPD_213     0x2100
#define eScreen_EPD_266     0x2600      // 2.66" 296x152
#define eScreen_EPD_271     0x2700
#define eScreen_EPD_287     0x2800
#define eScreen_EPD_370     0x3700
#define eScreen_EPD_417     0x4100
#define eScreen_EPD_437     0x430C

/*==============================================================================
 * 2. TAMAÑOS DE FRAME BUFFER (bytes)
 *============================================================================*/

#define frameSize_EPD_154   2888
#define frameSize_EPD_213   2756
#define frameSize_EPD_266   5624        // 296 * 152 / 8
#define frameSize_EPD_271   5808
#define frameSize_EPD_287   4736
#define frameSize_EPD_370   12480
#define frameSize_EPD_417   15000
#define frameSize_EPD_437   10560

/*==============================================================================
 * 3. CONFIGURACIÓN DE PINES
 *============================================================================*/

#define NOT_CONNECTED       0xFF

struct pins_t {
    uint8_t panelBusy;
    uint8_t panelDC;
    uint8_t panelReset;
    uint8_t panelCS;
    uint8_t panelON_EXT2;
    uint8_t panelSPI43_EXT2;
    uint8_t flashCS;
};

/*==============================================================================
 * 4. REGISTROS DE INICIALIZACIÓN
 *============================================================================*/

extern const uint8_t register_data_sm[6];
extern const uint8_t register_data_mid[6];

#endif /* EPD_CONFIGURATION_H */
