#include <xc.h>
#include <stdint.h>
#include "sdcard.h"
#include "spi.h"



#define SD_CS LATB3

void SD_Select()
{
    SD_CS = 0;
}

void SD_Deselect()
{
    SD_CS = 1;
    SPI_Write(0xFF);
}

uint8_t SD_Init()
{
    uint8_t i;

    SD_Deselect();

    for(i=0;i<10;i++)
        SPI_Write(0xFF);

    SD_Select();

    SPI_Write(0x40); // CMD0
    SPI_Write(0x00);
    SPI_Write(0x00);
    SPI_Write(0x00);
    SPI_Write(0x00);
    SPI_Write(0x95);

    for(i=0;i<10;i++)
    {
        if(SPI_Write(0xFF)==0x01)
            return 1;
    }

    return 0;
}

void SD_ReadLogo(uint8_t *buffer)
{
    uint16_t i;

    SD_Select();

    /* aquí debería ir la lectura real del sector
       (simplificado para ejemplo) */

    for(i=0;i<1024;i++)
    {
        buffer[i] = SPI_Write(0xFF);
    }

    SD_Deselect();
}




uint8_t SD_ReadByte(void)
{
    return SPI_Write(0xFF);
}
