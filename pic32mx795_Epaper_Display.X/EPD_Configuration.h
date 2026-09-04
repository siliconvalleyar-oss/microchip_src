/**
 * @file    EPD_Configuration.h
 * @brief   Configuración de pantallas E-Paper Pervasive Displays
 * @author  Leo
 * @date    2026-04-03
 * @version 2.0
 * 
 * @details Este archivo contiene las definiciones de todas las pantallas
 *          soportadas, tamaños de frame buffer y estructura de pines.
 * 
 * @note    Proyecto para PIC32MX795 con XC32 v5.00 (sin plib.h)
 * @note    SPI: SCK=RB14, SDO=RF5, SDI=RF4
 * @see     https://www.pervasivedisplays.com/products/
 */

#ifndef EPD_CONFIGURATION_H
#define EPD_CONFIGURATION_H

#include <stdint.h>


#define SYS_FREQ        80000000UL      /**< Frecuencia del sistema 80 MHz */
#define PB_FREQ         80000000UL      /**< Frecuencia del periférico 80 MHz */
#define EPD_SPI_CLOCK   8000000UL       /**< Velocidad SPI 8 MHz */


/*==============================================================================
 * 1. PANTALLAS SOPORTADAS
 *============================================================================*/

typedef uint32_t eScreen_EPD_t;

#define eScreen_EPD_154     0x1509      /**< 1.54" 152x152 */
#define eScreen_EPD_213     0x2100      /**< 2.13" 212x104 */
#define eScreen_EPD_266     0x2600      /**< 2.66" 296x152 */
#define eScreen_EPD_271     0x2700      /**< 2.71" 264x176 */
#define eScreen_EPD_287     0x2800      /**< 2.87" 296x128 */
#define eScreen_EPD_370     0x3700      /**< 3.70" 416x240 */
#define eScreen_EPD_417     0x4100      /**< 4.17" 300x400 */
#define eScreen_EPD_437     0x430C      /**< 4.37" 480x176 */

/*==============================================================================
 * 2. TAMAÑOS DE FRAME BUFFER (bytes)
 *============================================================================*/

#define frameSize_EPD_154   2888        /**< 152 * 152 / 8 */
#define frameSize_EPD_213   2756        /**< 212 * 104 / 8 */
#define frameSize_EPD_266   5624        /**< 296 * 152 / 8 */
#define frameSize_EPD_271   5808        /**< 264 * 176 / 8 */
#define frameSize_EPD_287   4736        /**< 296 * 128 / 8 */
#define frameSize_EPD_370   12480       /**< 416 * 240 / 8 */
#define frameSize_EPD_417   15000       /**< 300 * 400 / 8 */
#define frameSize_EPD_437   10560       /**< 480 * 176 / 8 */

/*==============================================================================
 * 3. CONFIGURACIÓN DE PINES
 *============================================================================*/

#define NOT_CONNECTED       0xFF        /**< Pin no conectado */

/**
 * @brief Estructura para configuración de pines del display
 * 
 * Los valores son números de pin según el mapeo:
 * - Puertos: 0-15 = PORTA, 16-31 = PORTB, 32-47 = PORTC
 * - 48-63 = PORTD, 64-79 = PORTE, 80-95 = PORTF, 96-111 = PORTG
 * 
 * @example pin 48 = RD0, pin 49 = RD1, pin 50 = RD2, pin 51 = RD3
 */
struct pins_t {
    uint8_t panelBusy;      /**< Pin BUSY (entrada) - recomendado RD0 */
    uint8_t panelDC;        /**< Pin Data/Command (salida) - recomendado RD1 */
    uint8_t panelReset;     /**< Pin RESET (salida) - recomendado RD2 */
    uint8_t panelCS;        /**< Pin Chip Select (salida) - recomendado RD3 */
    uint8_t panelON_EXT2;   /**< Pin adicional (opcional) */
    uint8_t panelSPI43_EXT2;/**< Pin SPI43 (opcional) */
    uint8_t flashCS;        /**< Pin CS para flash (opcional) */
};

/*==============================================================================
 * 4. REGISTROS DE INICIALIZACIÓN
 *============================================================================*/

/**
 * @brief Registros para pantallas pequeñas/medianas (1.54" a 4.37")
 * 
 * Datos: soft-reset, temperature, active temp, PSR0, PSR1
 */
extern const uint8_t register_data_sm[6];

/**
 * @brief Registros para pantallas medianas (4.2")
 */
extern const uint8_t register_data_mid[6];

#endif /* EPD_CONFIGURATION_H */
