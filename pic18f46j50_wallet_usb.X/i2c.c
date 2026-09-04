/*
 * File:   i2c.c
 * Author: optimus
 *
 * Created on 2 de abril de 2026, 17:07
 */
#include <xc.h>
#include "i2c.h"
#include "hardware_cfg.h"

//----------------------------------------
// DELAY PARA I2C (100kHz -> 5us por bit)
//----------------------------------------
void I2C_Delay(void)
{
    __delay_us(5);
}

//----------------------------------------
// INICIALIZACIÓN DEL SISTEMA
//----------------------------------------

//----------------------------------------
// I2C POR SOFTWARE (BIT-BANGING)
//----------------------------------------
void I2C_Init(void)
{
    // Configurar pines como salidas inicialmente
    SDA_OUTPUT();
    SCL_OUTPUT();
    
    // Estado inicial: líneas altas
    SDA_HIGH();
    SCL_HIGH();
    
    I2C_Delay();
}

void I2C_Start(void)
{
    // Configurar SDA como salida
    SDA_OUTPUT();
    
    // SDA y SCL altos
    SDA_HIGH();
    SCL_HIGH();
    I2C_Delay();
    
    // Condición START: SDA baja mientras SCL está alto
    SDA_LOW();
    I2C_Delay();
    
    // SCL baja para comenzar la transmisión
    SCL_LOW();
    I2C_Delay();
}

void I2C_Stop(void)
{
    // Configurar SDA como salida
    SDA_OUTPUT();
    
    // SCL baja, SDA baja
    SCL_LOW();
    SDA_LOW();
    I2C_Delay();
    
    // SCL alta
    SCL_HIGH();
    I2C_Delay();
    
    // Condición STOP: SDA alta mientras SCL está alto
    SDA_HIGH();
    I2C_Delay();
}

void I2C_Write(uint8_t data)
{
    // Enviar 8 bits
    for(uint8_t i = 0; i < 8; i++)
    {
        // Poner bit en SDA
        if(data & 0x80)
            SDA_HIGH();
        else
            SDA_LOW();
        
        I2C_Delay();
        
        // Generar pulso de reloj
        SCL_HIGH();
        I2C_Delay();
        SCL_LOW();
        I2C_Delay();
        
        data <<= 1;
    }
    
    // Configurar SDA como entrada para leer ACK
    SDA_INPUT();
    I2C_Delay();
    
    // Generar noveno pulso de reloj
    SCL_HIGH();
    I2C_Delay();
    
    // Leer ACK (0 = ACK, 1 = NACK)
    uint8_t ack = SDA_READ();
    
    SCL_LOW();
    I2C_Delay();
    
    // Volver SDA a salida
    SDA_OUTPUT();
}

uint8_t I2C_Read(uint8_t ack)
{
    uint8_t data = 0;
    
    // Configurar SDA como entrada
    SDA_INPUT();
    
    // Leer 8 bits
    for(uint8_t i = 0; i < 8; i++)
    {
        data <<= 1;
        
        SCL_HIGH();
        I2C_Delay();
        
        if(SDA_READ())
            data |= 0x01;
        
        SCL_LOW();
        I2C_Delay();
    }
    
    // Configurar SDA como salida para enviar ACK/NACK
    SDA_OUTPUT();
    
    // Enviar ACK o NACK
    if(ack)
        SDA_HIGH();  // NACK
    else
        SDA_LOW();   // ACK
    
    I2C_Delay();
    
    // Generar pulso de reloj para ACK
    SCL_HIGH();
    I2C_Delay();
    SCL_LOW();
    I2C_Delay();
    
    // Dejar SDA alta
    SDA_HIGH();
    
    return data;
}
