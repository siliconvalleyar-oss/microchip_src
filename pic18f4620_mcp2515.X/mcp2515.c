/**
 * @file mcp2515.c
 * @brief Driver para MCP2515 CAN Controller
 *
 * SPI interface for MCP2515 CAN bus controller
 * PIC18F4620 @ 20MHz
 */

#include "mcp2515.h"
#include <xc.h>
#include <stdint.h>

// ============================================================================
//  PIN DEFINITIONS
// ============================================================================

#define CS_TRIS   TRISAbits.TRISA0
#define CS_PIN    LATAbits.LATA0
#define CS_LOW()  (CS_PIN = 0)
#define CS_HIGH() (CS_PIN = 1)

// ============================================================================
//  SPI FUNCTIONS
// ============================================================================

static void SPI_Init(void) {
    TRISCbits.TRISC3 = 0;   // SCK salida
    TRISCbits.TRISC4 = 1;   // SDI entrada
    TRISCbits.TRISC5 = 0;   // SDO salida
    CS_TRIS = 0;
    CS_HIGH();

    SSPSTATbits.SMP = 0;    // Muestreo a la mitad del pulso
    SSPSTATbits.CKE = 1;    // Dato valido en flanco activo->inactivo
    SSPCON1bits.CKP = 0;    // Reloj en reposo bajo (Modo SPI 0,0)
    SSPCON1bits.SSPM = 0b0000; // Reloj = Fosc/4 (5 MHz @ 20 MHz)
    SSPCON1bits.SSPEN = 1;  // Habilita el modulo MSSP en modo SPI
}

static uint8_t SPI_Transfer(uint8_t data) {
    SSPBUF = data;
    while (!SSPSTATbits.BF);   // Espera a que termine la transferencia
    return SSPBUF;
}

// ============================================================================
//  MCP2515 LOW-LEVEL FUNCTIONS
// ============================================================================

static void MCP_WriteReg(uint8_t addr, uint8_t data) {
    CS_LOW();
    SPI_Transfer(MCP_WRITE);
    SPI_Transfer(addr);
    SPI_Transfer(data);
    CS_HIGH();
}

static uint8_t MCP_ReadReg(uint8_t addr) {
    uint8_t val;
    CS_LOW();
    SPI_Transfer(MCP_READ);
    SPI_Transfer(addr);
    val = SPI_Transfer(0x00);
    CS_HIGH();
    return val;
}

static void MCP_BitModify(uint8_t addr, uint8_t mask, uint8_t data) {
    CS_LOW();
    SPI_Transfer(MCP_BIT_MODIFY);
    SPI_Transfer(addr);
    SPI_Transfer(mask);
    SPI_Transfer(data);
    CS_HIGH();
}

static void MCP_Reset(void) {
    CS_LOW();
    SPI_Transfer(MCP_RESET);
    CS_HIGH();
    __delay_ms(10);
}

// ============================================================================
//  PUBLIC API
// ============================================================================

void mcp2515_init(void) {
    SPI_Init();
    MCP_Reset();

    // Entrar en modo configuracion
    MCP_BitModify(CANCTRL, MODE_MASK, MODE_CONFIG);
    while ((mcp2515_read_register(CANSTAT) & MODE_MASK) != MODE_CONFIG);

    // Configurar bitrate 500 kbps (8 MHz cristal)
    MCP_WriteReg(CNF1, 0x00);
    MCP_WriteReg(CNF2, 0x90);
    MCP_WriteReg(CNF3, 0x02);

    // Mascaras de RXB0 a 0 -> acepta cualquier ID
    MCP_WriteReg(RXM0SIDH, 0x00);
    MCP_WriteReg(RXM0SIDL, 0x00);

    // RXB0CTRL: recibir cualquier mensaje, rollover a RXB1 si RXB0 lleno
    MCP_WriteReg(RXB0CTRL, 0x64);

    // Habilitar interrupcion de mensaje recibido en RXB0
    MCP_WriteReg(CANINTE, 0x01);

    // Pasar a modo normal
    MCP_BitModify(CANCTRL, MODE_MASK, MODE_NORMAL);
    while ((mcp2515_read_register(CANSTAT) & MODE_MASK) != MODE_NORMAL);
}

void mcp2515_reset(void) {
    MCP_Reset();
}

uint8_t mcp2515_read_register(uint8_t addr) {
    return MCP_ReadReg(addr);
}

void mcp2515_write_register(uint8_t addr, uint8_t data) {
    MCP_WriteReg(addr, data);
}

void mcp2515_bit_modify(uint8_t addr, uint8_t mask, uint8_t data) {
    MCP_BitModify(addr, mask, data);
}

void mcp2515_send_message(uint16_t id, const uint8_t *data, uint8_t len) {
    if (len > 8) len = 8;

    MCP_WriteReg(TXB0SIDH, (uint8_t)(id >> 3));
    MCP_WriteReg(TXB0SIDL, (uint8_t)((id & 0x07) << 5));
    MCP_WriteReg(TXB0DLC, len);

    for (uint8_t i = 0; i < len; i++) {
        MCP_WriteReg(TXB0D0 + i, data[i]);
    }

    // Request To Send del buffer TXB0
    CS_LOW();
    SPI_Transfer(MCP_RTS | 0x01);
    CS_HIGH();
}

uint8_t mcp2515_receive_message(uint16_t *id, uint8_t *data, uint8_t *len) {
    if (!(mcp2515_read_register(CANINTF) & 0x01)) {
        return 0;   // No hay mensaje nuevo en RXB0
    }

    uint8_t sidh = mcp2515_read_register(RXB0SIDH);
    uint8_t sidl = mcp2515_read_register(RXB0SIDL);
    *id = ((uint16_t)sidh << 3) | (sidl >> 5);

    uint8_t dlc = mcp2515_read_register(RXB0DLC) & 0x0F;
    *len = dlc;
    for (uint8_t i = 0; i < dlc; i++) {
        data[i] = mcp2515_read_register(RXB0D0 + i);
    }

    // Limpia el flag de interrupcion RXB0 para liberar el buffer
    mcp2515_bit_modify(CANINTF, 0x01, 0x00);
    return 1;
}
