# API Reference — MCP2515 Driver for PIC18F4620

## Public Functions

### mcp2515_init

```c
void mcp2515_init(void);
```

Initializes the SPI module, resets the MCP2515, and configures it for 500 kbps CAN bus operation.

**Sequence:**
1. Initialize SPI (Mode 0,0, Fosc/4, 5 MHz)
2. Reset MCP2515
3. Enter Configuration mode
4. Set CNF1/CNF2/CNF3 for 500 kbps @ 8 MHz crystal
5. Configure RXB0 mask to accept all messages
6. Set RXB0CTRL for rollover (0x64 = BUKT + BUKT2)
7. Enable RXB0 interrupt (CANINTE bit 0)
8. Enter Normal mode

---

### mcp2515_reset

```c
void mcp2515_reset(void);
```

Resets the MCP2515 via SPI RESET command. Delays 10 ms after reset.

---

### mcp2515_read_register

```c
uint8_t mcp2515_read_register(uint8_t addr);
```

Reads a single MCP2515 register at the given address.

**Parameters:**
- `addr` — Register address (0x00 - 0x7F)

**Returns:** Register value (8-bit)

---

### mcp2515_write_register

```c
void mcp2515_write_register(uint8_t addr, uint8_t data);
```

Writes a single byte to a MCP2515 register.

**Parameters:**
- `addr` — Register address
- `data` — Value to write

---

### mcp2515_bit_modify

```c
void mcp2515_bit_modify(uint8_t addr, uint8_t mask, uint8_t data);
```

Modifies specific bits in a MCP2515 register without affecting other bits.

**Parameters:**
- `addr` — Register address
- `mask` — Bit mask (1 = modify, 0 = keep)
- `data` — New bit values

---

### mcp2515_send_message

```c
void mcp2515_send_message(uint16_t id, const uint8_t *data, uint8_t len);
```

Sends a CAN 2.0A (11-bit standard) message using TXB0.

**Parameters:**
- `id` — 11-bit CAN identifier (0x000 - 0x7FF)
- `data` — Pointer to data buffer (max 8 bytes)
- `len` — Data length (0-8, truncated to 8 if larger)

**Behavior:**
- Loads TXB0SIDH/TXB0SIDL with ID
- Loads TXB0DLC with length
- Copies data bytes to TXB0D0..TXB0D7
- Issues RTS command for TXB0

**Return:** void (no error reporting in current implementation)

---

### mcp2515_receive_message

```c
uint8_t mcp2515_receive_message(uint16_t *id, uint8_t *data, uint8_t *len);
```

Receives a CAN message from RXB0 if available.

**Parameters:**
- `id` — Output: received CAN ID
- `data` — Output: received data bytes
- `len` — Output: received data length (DLC)

**Returns:** 1 if message received, 0 if no message available

**Behavior:**
- Checks CANINTF bit 0 (RXB0 interrupt flag)
- Reads SIDH/SIDL to extract 11-bit ID
- Reads DLC for length
- Reads data bytes from RXB0D0..RXB0D7
- Clears CANINTF bit 0 after read

---

## SPI Instructions

| Instruction | Code | Description |
|-------------|------|-------------|
| RESET | 0xC0 | Software reset |
| READ | 0x03 | Read register |
| WRITE | 0x02 | Write register |
| RTS_TX0 | 0x81 | Request to send TXB0 |
| RTS_TX1 | 0x82 | Request to send TXB1 |
| RTS_TX2 | 0x84 | Request to send TXB2 |
| RTS_ALL | 0x87 | Request to send all TX buffers |
| READ_STATUS | 0xA0 | Read status (TXREQ, RX flags) |
| BIT_MODIFY | 0x05 | Modify bits in register |
| READ_RXB0 | 0x90 | Read RXB0 directly |
| READ_RXB1 | 0x94 | Read RXB1 directly |

## Register Map

### Configuration Registers

| Register | Address | Description |
|----------|---------|-------------|
| CANSTAT | 0x0E | CAN controller status (mode bits 7:5) |
| CANCTRL | 0x0F | CAN control register (mode bits 7:5) |
| CNF1 | 0x2A | Bit timing config 1 (BRP, SJW) |
| CNF2 | 0x29 | Bit timing config 2 (BTLMODE, SAM, SEG2PH, SEG1PH) |
| CNF3 | 0x28 | Bit timing config 3 (SEG2PH, SOF) |
| CANINTE | 0x2B | Interrupt enable register |
| CANINTF | 0x2C | Interrupt flag register |

### TXB0 Registers

| Register | Address | Description |
|----------|---------|-------------|
| TXB0CTRL | 0x30 | TXB0 control (TXREQ, TXERR, MLOA) |
| TXB0SIDH | 0x31 | TXB0 standard ID high byte |
| TXB0SIDL | 0x32 | TXB0 standard ID low byte |
| TXB0DLC | 0x35 | TXB0 data length code |
| TXB0D0 - TXB0D7 | 0x36 - 0x3D | TXB0 data bytes |

### RXB0 Registers

| Register | Address | Description |
|----------|---------|-------------|
| RXB0CTRL | 0x60 | RXB0 control (RXRE, RXM, BUKT) |
| RXB0SIDH | 0x61 | RXB0 standard ID high byte |
| RXB0SIDL | 0x62 | RXB0 standard ID low byte |
| RXB0DLC | 0x65 | RXB0 data length code |
| RXB0D0 - RXB0D7 | 0x66 - 0x6D | RXB0 data bytes |

### Filter/Mask Registers

| Register | Address | Description |
|----------|---------|-------------|
| RXF0SIDH | 0x00 | Filter 0 SIDH |
| RXF0SIDL | 0x01 | Filter 0 SIDL |
| RXM0SIDH | 0x20 | Mask 0 SIDH |
| RXM0SIDL | 0x21 | Mask 0 SIDL |

---

## Operation Modes

| Mode | Value | Description |
|------|-------|-------------|
| Configuration | 0x80 | Register configuration mode |
| Normal | 0x00 | Normal operation |
| Sleep | 0x20 | Low-power sleep mode |
| Listen Only | 0x60 | Receive-only, no ACK |
| Loopback | 0x40 | Internal loopback |

Mode is set via CANCTRL bits 7:5, masked with 0xE0.

---

## Bitrate Configuration (500 kbps, 8 MHz crystal)

| Register | Value | Description |
|----------|-------|-------------|
| CNF1 | 0x00 | BRP = 0, SJW = 0 |
| CNF2 | 0x90 | BTLMODE = 1, SAM = 0, SEG2PH = 6, SEG1PH = 0 |
| CNF3 | 0x02 | SEG2PH = 2, WAKFIL = 0 |

Bitrate formula: `Fosc / (2 * (BRP + 1) * (1 + TSEG1 + TSEG2))`

For 8 MHz crystal, 500 kbps: BRP=0, TSEG1=0, TSEG2=2

---

## ID Encoding

### Standard (11-bit) ID

```
SIDH = id[10:3]
SIDL = id[2:0] << 5
```

Decoding:
```c
id = ((uint16_t)sidh << 3) | (sidl >> 5);
```

### Extended (29-bit) ID

```
SIDH = id[28:21]
SIDL = id[20:18] | 0x08 | id[17:16]
EID8 = id[15:8]
EID0 = id[7:0]
```

---

## OBD-II Constants

```c
#define OBD2_REQUEST_ID   0x7DF   // Functional broadcast request
#define OBD2_RESPONSE_ID  0x7E8   // ECU #1 response
#define OBD2_RESPONSE_ID2 0x7E9   // ECU #2 response
```

## PID Constants

```c
#define PID_ENGINE_LOAD    0x04
#define PID_COOLANT_TEMP   0x05
#define PID_ENGINE_RPM     0x0C
#define PID_VEHICLE_SPEED  0x0D
#define PID_THROTTLE_POS   0x11
```

---

## SPI Configuration (PIC18F4620)

```c
SSPSTATbits.SMP = 0;    // Input sampled at middle of data output time
SSPSTATbits.CKE = 1;    // Transmit occurs on transition from active to idle clock state
SSPCON1bits.CKP = 0;    // Clock polarity: idle state is low (SPI Mode 0,0)
SSPCON1bits.SSPM = 0b0000; // SPI Master mode, clock = Fosc/4
SSPCON1bits.SSPEN = 1;  // Enable serial port
```

| Parameter | Value |
|-----------|-------|
| Mode | 0,0 (CPOL=0, CPHA=0) |
| Clock | Fosc/4 = 5 MHz @ 20 MHz |
| SMP | 0 |
| CKE | 1 |
