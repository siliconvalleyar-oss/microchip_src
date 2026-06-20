#include "hal/hal_config.h"
#include "hal/hal_i2c.h"

void I2C_Init(void)
{
    TRISDbits.TRISD0 = 1;
    TRISDbits.TRISD1 = 1;

    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 0;

    volatile unsigned char *rpor6 = (volatile unsigned char *)0xEF6;
    *rpor6 = 0x1E;
    *(rpor6 + 1) = 0x1D;

    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 1;

    SSP2CON1 = 0b00101000;
    SSP2CON2 = 0;
    SSP2ADD = 7;
    SSP2CON1bits.SSPEN = 1;
}

I2C_Status I2C_Start(void)
{
    uint16_t timeout = 10000;
    while(SSP2CON2bits.SEN) if(--timeout == 0) return I2C_TIMEOUT;
    SSP2CON2bits.SEN = 1;
    timeout = 10000;
    while(SSP2CON2bits.SEN) if(--timeout == 0) return I2C_TIMEOUT;
    return I2C_OK;
}

I2C_Status I2C_Stop(void)
{
    uint16_t timeout = 10000;
    SSP2CON2bits.PEN = 1;
    timeout = 10000;
    while(SSP2CON2bits.PEN) if(--timeout == 0) return I2C_TIMEOUT;
    return I2C_OK;
}

I2C_Status I2C_WriteByte(uint8_t data)
{
    uint16_t timeout = 10000;
    SSP2BUF = data;
    while(SSP2STATbits.BF) if(--timeout == 0) return I2C_TIMEOUT;
    if(SSP2CON2bits.ACKSTAT) return I2C_NACK;
    return I2C_OK;
}

void I2C_WriteMulti(uint8_t addr, uint8_t* data, uint8_t len)
{
    I2C_Start();
    I2C_WriteByte((uint8_t)(addr << 1));
    for(uint8_t i = 0; i < len; i++) I2C_WriteByte(data[i]);
    I2C_Stop();
}
