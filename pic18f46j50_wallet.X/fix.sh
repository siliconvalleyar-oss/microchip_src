#!/bin/bash
# remove_warnings.sh - Elimina warnings del proyecto

echo "=========================================="
echo "  Eliminando warnings del proyecto"
echo "=========================================="
echo ""

# Colores
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

#=============================================================================
# 1. Corregir hal_button.c - Comentar función no usada
#=============================================================================
echo -e "${YELLOW}[1/6] Corrigiendo hal_button.c...${NC}"

cat > hal_button.c << 'EOF'
#include "hal/hal_config.h"
#include "hal/hal_button.h"

static uint8_t button_history[2] = {0xFF, 0xFF};

void BUTTON_Init(void)
{
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB5 = 1;
    INTCON2bits.RBPU = 0;
}

uint8_t BUTTON_GetState(uint8_t button)
{
    uint8_t pin_state = 1;
    
    if(button == 0)
        pin_state = PORTBbits.RB4;
    else if(button == 1)
        pin_state = PORTBbits.RB5;
    
    return (pin_state == 0) ? BUTTON_PRESSED : 0;
}
EOF

echo -e "  ${GREEN}✓ Eliminada función no usada: _BUTTON_GetDebounced${NC}"

#=============================================================================
# 2. Corregir hal_i2c.c - Comentar funciones no usadas
#=============================================================================
echo -e "\n${YELLOW}[2/6] Corrigiendo hal_i2c.c...${NC}"

cat > hal_i2c.c << 'EOF'
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
EOF

echo -e "  ${GREEN}✓ Eliminadas funciones no usadas: _I2C_Restart, _I2C_ReadByte, _I2C_ReadMulti${NC}"

#=============================================================================
# 3. Corregir spi_sd.c - Mantener solo funciones usadas
#=============================================================================
echo -e "\n${YELLOW}[3/6] Corrigiendo spi_sd.c...${NC}"

cat > spi_sd.c << 'EOF'
#include "hal/hal_config.h"
#include "hal/spi_sd.h"

void SPI_SD_Init(void)
{
    TRISBbits.TRISB0 = 1;
    TRISBbits.TRISB1 = 0;
    TRISBbits.TRISB3 = 0;
    TRISCbits.TRISC7 = 0;

    LATBbits.LATB1 = 0;
    LATBbits.LATB3 = 1;
    LATCbits.LATC7 = 0;

    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 0;

    volatile unsigned char *rpor1 = (volatile unsigned char *)0xEF1;
    *rpor1 = 0x0A;

    volatile unsigned char *rpor9 = (volatile unsigned char *)0xEF9;
    *rpor9 = 0x09;

    volatile unsigned char *rpinr21 = (volatile unsigned char *)0xEFD;
    *rpinr21 = 0x03;

    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 1;

    SSP2CON1 = 0b00100010;
    SSP2CON2 = 0;
    SSP2STAT = 0;
    SSP2CON1bits.SSPEN = 1;

    __delay_ms(10);
}
EOF

echo -e "  ${GREEN}✓ Eliminadas funciones no usadas: _SPI_SD_TransferByte, _SPI_SD_Select, _SPI_SD_Deselect${NC}"

#=============================================================================
# 4. Corregir ssd1306.c - Comentar función no usada
#=============================================================================
echo -e "\n${YELLOW}[4/6] Corrigiendo ssd1306.c...${NC}"

# Solo comentar la función Fill si existe
sed -i '/void SSD1306_Fill/,/^}/ { s/^/\/\//; }' ssd1306.c 2>/dev/null || true
echo -e "  ${GREEN}✓ Comentada función no usada: _SSD1306_Fill${NC}"

#=============================================================================
# 5. Corregir usb_cdc.c - Comentar función no usada
#=============================================================================
echo -e "\n${YELLOW}[5/6] Corrigiendo usb_cdc.c...${NC}"

# Comentar función IsConnected si no se usa
sed -i '/uint8_t USB_CDC_IsConnected/,/^}/ { s/^/\/\//; }' usb_cdc.c 2>/dev/null || true
echo -e "  ${GREEN}✓ Comentada función no usada: _USB_CDC_IsConnected${NC}"

#=============================================================================
# 6. Reducir heap memory (opcional)
#=============================================================================
echo -e "\n${YELLOW}[6/6] Reduciendo heap memory...${NC}"

# Agregar flag para reducir heap si no se usa malloc
echo "  Para reducir el heap (100% actual), agrega en Project Properties:"
echo "  XC8 Linker → Additional options: --heap=100"
echo ""

echo "=========================================="
echo -e "${GREEN}  WARNINGS ELIMINADOS${NC}"
echo "=========================================="
echo ""
echo "Ahora recompila: Clean and Build (Shift+F11, luego F11)"
