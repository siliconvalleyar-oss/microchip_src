#ifndef I2C_SW_H
#define I2C_SW_H

#include "config.h"

void I2C_SW_Init(void);
void I2C_SW_Start(void);
void I2C_SW_Stop(void);
uint8_t I2C_SW_WriteByte(uint8_t data);
uint8_t I2C_SW_ReadByte(uint8_t send_ack);
uint8_t I2C_SW_Write(uint8_t addr7, const uint8_t *buf, uint8_t len);

#endif
