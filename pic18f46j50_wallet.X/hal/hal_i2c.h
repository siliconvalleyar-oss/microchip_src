#ifndef HAL_I2C_H
#define HAL_I2C_H

#include "hal_config.h"

typedef enum {
    I2C_OK = 0,
    I2C_BUSY,
    I2C_TIMEOUT,
    I2C_NACK
} I2C_Status;

void I2C_Init(void);
I2C_Status I2C_Start(void);
I2C_Status I2C_Stop(void);
I2C_Status I2C_Restart(void);
I2C_Status I2C_WriteByte(uint8_t data);
uint8_t I2C_ReadByte(uint8_t ack);
void I2C_WriteMulti(uint8_t addr, uint8_t* data, uint8_t len);
void I2C_ReadMulti(uint8_t addr, uint8_t* data, uint8_t len);

#endif
