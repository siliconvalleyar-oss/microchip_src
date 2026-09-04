/**
 * @file mcp2515.h
 * @brief Driver para MCP2515 - CAN Controller
 *
 * SPI interface for MCP2515 CAN bus controller
 * PIC18F4620 @ 20MHz
 */

#ifndef MCP2515_H
#define MCP2515_H

#include <stdint.h>

// ============================================================================
//  SPI INSTRUCTIONS (DS20001801)
// ============================================================================

#define MCP_RESET       0xC0
#define MCP_READ        0x03
#define MCP_WRITE       0x02
#define MCP_RTS         0x80   // RTS = 0x80 | (1<<n) n=0,1,2 (TXB0,1,2)
#define MCP_READ_STATUS 0xA0
#define MCP_BIT_MODIFY  0x05
#define MCP_READ_RXB0   0x90
#define MCP_READ_RXB1   0x94

// ============================================================================
//  REGISTER MAP
// ============================================================================

#define CANSTAT   0x0E
#define CANCTRL   0x0F
#define CNF1      0x2A
#define CNF2      0x29
#define CNF3      0x28
#define CANINTE   0x2B
#define CANINTF   0x2C
#define TXB0CTRL  0x30
#define TXB0SIDH  0x31
#define TXB0SIDL  0x32
#define TXB0DLC   0x35
#define TXB0D0    0x36
#define RXB0CTRL  0x60
#define RXB0SIDH  0x61
#define RXB0SIDL  0x62
#define RXB0DLC  0x65
#define RXB0D0    0x66
#define RXF0SIDH  0x00
#define RXF0SIDL  0x01
#define RXM0SIDH  0x20
#define RXM0SIDL  0x21

// ============================================================================
//  OPERATION MODES (CANCTRL)
// ============================================================================

#define MODE_CONFIG   0x80
#define MODE_NORMAL   0x00
#define MODE_MASK     0xE0

// ============================================================================
//  PUBLIC API
// ============================================================================

void mcp2515_init(void);
void mcp2515_reset(void);

uint8_t mcp2515_read_register(uint8_t addr);
void mcp2515_write_register(uint8_t addr, uint8_t data);
void mcp2515_bit_modify(uint8_t addr, uint8_t mask, uint8_t data);

void mcp2515_send_message(uint16_t id, const uint8_t *data, uint8_t len);
uint8_t mcp2515_receive_message(uint16_t *id, uint8_t *data, uint8_t *len);

#endif /* MCP2515_H */
