/**
 * @file    EPD_Driver.h
 * @brief   Driver principal para E-Paper Pervasive Displays
 * @author  Leo
 * @date    2026-04-03
 * @version 2.0
 * 
 * @details Este driver implementa el control de displays E-Paper
 *          mediante SPI nativo, sin dependencia de plib.h.
 *          Compatible con PIC32MX795F512L y XC32 v5.00.
 * 
 * @note    SPI configurado con PPS: SCK=RB14, SDO=RF5, SDI=RF4
 */

#ifndef EPD_DRIVER_H
#define EPD_DRIVER_H

#include <stdint.h>
#include "EPD_Configuration.h"



//static inline uint32_t ReadCoreTimer(void) ;
void delayMs(uint32_t ms) ;
void test_pines_display(uint16_t repeticiones, uint16_t tiempo_ms);
void test_pines_simultaneos(uint16_t tiempo_ms);
void test_pines_carrusel(uint16_t repeticiones, uint16_t tiempo_ms) ;


void epd_pinMode(uint8_t pin, uint8_t mode);
void epd_digitalWrite(uint8_t pin, uint8_t value) ;
uint8_t epd_digitalRead(uint8_t pin);

void spi_init(void);
void spi4_init(void);
void spi4_send(uint8_t data) ;




/**
 * @class EPD_Driver
 * @brief Clase principal para control del display E-Paper
 * 
 * Implementa la inicialización del COG (Chip On Glass), actualización
 * global de pantalla y apagado del controlador.
 */
class EPD_Driver {
public:
    /**
     * @brief Constructor del driver
     * @param screen Tipo de pantalla (eScreen_EPD_xxx)
     * @param board  Configuración de pines
     */
    EPD_Driver(eScreen_EPD_t screen, pins_t board);
    
    /**
     * @brief Inicializa el controlador COG del display
     * @note Configura pines, SPI y envía secuencia de inicialización
     */
    void COG_initial(void);
    
    /**
     * @brief Apaga el controlador COG y DC/DC
     */
    void COG_powerOff(void);
    
    /**
     * @brief Realiza una actualización global de pantalla
     * @param data1s Primer frame (blanco/negro)
     * @param data2s Segundo frame (rojo/0x00 para monocromo)
     */
    void globalUpdate(const uint8_t* data1s, const uint8_t* data2s);
    
    /**
     * @brief Obtiene el tamaño del frame buffer
     * @return Tamaño en bytes
     */
    uint32_t getFrameSize(void) const { return image_data_size; }
    
    /**
     * @brief Obtiene el ancho de la pantalla
     * @return Ancho en píxeles
     */
    uint16_t getWidth(void) const { return width; }
    
    /**
     * @brief Obtiene el alto de la pantalla
     * @return Alto en píxeles
     */
    uint16_t getHeight(void) const { return height; }
    
    

    
    // En EPD_Driver.h
void fastUpdate(const uint8_t* data1s, const uint8_t* data2s);
void partialUpdate(const uint8_t* data1s, const uint8_t* data2s, 
                   uint16_t x_start, uint16_t y_start, 
                   uint16_t x_end, uint16_t y_end);
void setWindow(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);
void clearScreenFast(bool white);

uint8_t getBusyPin() { return epd_digitalRead(spi_basic.panelBusy); }


    /**
     * @brief Envía datos por SPI con protocolo index/data
     * @param index Registro de destino
     * @param data  Puntero a los datos
     * @param len   Longitud de los datos
     */
    void _sendIndexData(uint8_t index, const uint8_t* data, uint32_t len);
    void _DCDC_powerOn(void);    

private:
    static uint8_t tempClearBuffer[frameSize_EPD_266]; // 5624 bytes 
    


    /**
     * @brief Soft-reset del COG
     */
    void _softReset(void);
    
    /**
     * @brief Comando de refresco de pantalla
     */
    void _displayRefresh(void);
    
    /**
     * @brief Hard-reset del COG con delays programados
     */
    void _reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5);
    
    /**
     * @brief Enciende el DC/DC converter
     */

    
    pins_t spi_basic;           /**< Configuración de pines */
    uint16_t pdi_size;          /**< Tamaño de pantalla (código) */
    uint32_t image_data_size;   /**< Tamaño del frame buffer */
    uint16_t width;             /**< Ancho en píxeles */
    uint16_t height;            /**< Alto en píxeles */
    uint8_t register_data[6];   /**< Registros de inicialización */
};



#endif /* EPD_DRIVER_H */
