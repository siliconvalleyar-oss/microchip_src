#include "i2c_sw.h"

/* Macros open-drain correctas */
#define SCL_HIGH()  do { SCL_TRIS = 1; } while(0)
#define SCL_LOW()   do { SCL_LAT = 0; SCL_TRIS = 0; } while(0)
#define SDA_HIGH()  do { SDA_TRIS = 1; } while(0)
#define SDA_LOW()   do { SDA_LAT = 0; SDA_TRIS = 0; } while(0)
#define SDA_READ()  (SDA_PORT)

/* Velocidad ~50 kHz (10 us) más fiable */
#define I2C_DELAY()  __delay_us(1)

void I2C_SW_Init(void)
{
    SCL_LAT = 0;
    SDA_LAT = 0;
    SCL_HIGH();
    SDA_HIGH();
    I2C_DELAY();
}

void I2C_SW_Start(void)
{
    SDA_HIGH();
    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();
    SDA_LOW();
    I2C_DELAY();
    SCL_LOW();
    I2C_DELAY();
}

void I2C_SW_Stop(void)
{
    SDA_LOW();
    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();
    SDA_HIGH();
    I2C_DELAY();
}

uint8_t I2C_SW_WriteByte(uint8_t data)
{
    uint8_t i, ack;
    for (i = 0; i < 8; i++) {
        if (data & 0x80) SDA_HIGH();
        else             SDA_LOW();
        data <<= 1;
        I2C_DELAY();
        SCL_HIGH();
        I2C_DELAY();
        SCL_LOW();
        I2C_DELAY();
    }
    SDA_HIGH();
    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();
    ack = SDA_READ() ? 1 : 0;   /* 0=ACK, 1=NACK */
    SCL_LOW();
    I2C_DELAY();
    return ack;
}

uint8_t I2C_SW_ReadByte(uint8_t send_ack)
{
    uint8_t i, data = 0;
    SDA_HIGH();
    for (i = 0; i < 8; i++) {
        data <<= 1;
        I2C_DELAY();
        SCL_HIGH();
        I2C_DELAY();
        if (SDA_READ()) data |= 0x01;
        SCL_LOW();
        I2C_DELAY();
    }
    if (send_ack) SDA_LOW();
    else          SDA_HIGH();
    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();
    SCL_LOW();
    SDA_HIGH();
    I2C_DELAY();
    return data;
}

uint8_t I2C_SW_Write(uint8_t addr7, const uint8_t *buf, uint8_t len)
{
    uint8_t i;
    I2C_SW_Start();
    if (I2C_SW_WriteByte(addr7 << 1)) {
        I2C_SW_Stop();
        return 0;
    }
    for (i = 0; i < len; i++) {
        if (I2C_SW_WriteByte(buf[i])) {
            I2C_SW_Stop();
            return 0;
        }
    }
    I2C_SW_Stop();
    return 1;
}
