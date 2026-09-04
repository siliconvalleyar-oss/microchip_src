# Architecture — PIC18F4620 MCP2515 OBD-II Reader

## System Overview

```
┌─────────────────────────────────────────────┐
│              PIC18F4620 (20 MHz)             │
│                                              │
│  ┌────────────┐    SPI    ┌──────────────┐  │
│  │  main.c    │──────────▶│  MCP2515     │  │
│  │            │           │  (CAN 500k)  │  │
│  │ - UART     │           └──────┬───────┘  │
│  │ - OBD-II   │                  │           │
│  │ - PID poll │           ┌──────▼───────┐  │
│  └────────────┘           │  SN65HVD230  │  │
│                           │ (Transceiver)│  │
│                           └──────┬───────┘  │
│                                  │           │
│                            CANH/CANL        │
│                                  │           │
│                           ┌──────▼───────┐  │
│                           │  OBD-II      │  │
│                           │  Connector   │  │
│                           └──────────────┘  │
│                                              │
│  ┌────────────┐                              │
│  │   UART     │──────────────────────────────┼──▶ PC Terminal
│  │  (9600)    │                              │
│  └────────────┘                              │
│                                              │
└──────────────────────────────────────────────┘
```

## Software Architecture

### Layers

```
Application Layer
├── main.c
│   ├── UART_Init() - Serial debug output
│   ├── OBD2_RequestPID() - Send CAN request
│   └── OBD2_WaitResponse() - Receive and parse
│
Hardware Abstraction Layer
├── mcp2515.c
│   ├── SPI_Init() - MSSP module configuration
│   ├── SPI_Transfer() - Byte-level SPI
│   ├── MCP_WriteReg() - MCP2515 register write
│   ├── MCP_ReadReg() - MCP2515 register read
│   ├── MCP_BitModify() - MCP2515 bit modify
│   ├── mcp2515_send_message() - CAN transmit
│   └── mcp2515_receive_message() - CAN receive
│
Hardware Layer
├── PIC18F4620 MSSP (SPI Master)
├── PIC18F4620 EUSART (UART)
└── MCP2515 CAN Controller
```

## Data Flow

### Request-Response Pattern

```
1. main() calls OBD2_RequestPID(PID_ENGINE_RPM)
   └── mcp2515_send_message(0x7DF, {0x02, 0x01, 0x0C, ...}, 8)
       └── Load TXB0 with ID and data
       └── Issue RTS command
           └── MCP2515 transmits CAN frame on bus

2. ECU receives 0x7DF, processes request
   └── ECU transmits response on 0x7E8
       └── MCP2515 receives into RXB0
           └── Sets CANINTF bit 0

3. OBD2_WaitResponse() polls mcp2515_receive_message()
   └── Checks CANINTF bit 0
       └── Reads RXB0SIDH/RXB0SIDL → id
       └── Reads RXB0DLC → len
       └── Reads RXB0D0..D7 → data
       └── Clears CANINTF bit 0
   └── Validates id == 0x7E8, data[1] == 0x41, data[2] == pid
   └── Returns A, B data bytes

4. main() decodes A, B
   └── RPM = (A * 256 + B) / 4
   └── printf("RPM: %u rpm\r\n", rpm)
```

## Memory Map

### PIC18F4620 RAM Usage

| Variable | Location | Size |
|----------|----------|------|
| Stack | - | ~64 bytes |
| UART buffers | - | Minimal |
| CAN data[8] | Stack | 8 bytes |
| A, B variables | Stack | 2 bytes |

Total RAM usage: < 100 bytes (very lightweight)

### MCP2515 Registers Used

| Register | Address | Usage |
|----------|---------|-------|
| CANCTRL | 0x0F | Mode control |
| CANSTAT | 0x0E | Status read |
| CNF1 | 0x2A | Bitrate config |
| CNF2 | 0x29 | Bitrate config |
| CNF3 | 0x28 | Bitrate config |
| CANINTE | 0x2B | RXB0 interrupt enable |
| CANINTF | 0x2C | RXB0 interrupt flag |
| TXB0CTRL | 0x30 | TXB0 status |
| TXB0SIDH | 0x31 | TXB0 ID high |
| TXB0SIDL | 0x32 | TXB0 ID low |
| TXB0DLC | 0x35 | TXB0 length |
| TXB0D0-D7 | 0x36-0x3D | TXB0 data |
| RXB0CTRL | 0x60 | RXB0 control |
| RXB0SIDH | 0x61 | RXB0 ID high |
| RXB0SIDL | 0x62 | RXB0 ID low |
| RXB0DLC | 0x65 | RXB0 length |
| RXB0D0-D7 | 0x66-0x6D | RXB0 data |
| RXM0SIDH | 0x20 | Mask 0 high |
| RXM0SIDL | 0x21 | Mask 0 low |

## Timing

### Poll Loop Timing

```
OBD2_WaitResponse():
  - timeout = 500 iterations
  - Each iteration: ~2ms (mcp2515_receive_message + __delay_ms(2))
  - Total timeout: ~1 second per PID request

main() loop:
  - Request RPM + wait: ~1s
  - Request Speed + wait: ~1s
  - Request Temp + wait: ~1s
  - Request Load + wait: ~1s
  - Request Throttle + wait: ~1s
  - Delay: 500ms
  - Total cycle: ~5.5 seconds
```

### SPI Timing

```
SPI clock: Fosc/4 = 5 MHz @ 20 MHz
Single byte: ~2.2 µs (8 bits / 5 MHz + overhead)
MCP2515 register access: ~10 µs (4 bytes: instruction + address + data + CS)
```

## Comparison with C++ Reference

| Feature | C++ (Raspberry Pi) | C (PIC18F4620) |
|---------|-------------------|----------------|
| SPI | /dev/spidev via ioctl | MSSP hardware |
| Threading | std::thread | None (polling) |
| Memory | Heap allocation | Static only |
| Display | SSD1306 I2C/SPI | UART debug |
| ISO-TP | Full multi-frame | Single-frame only |
| Config | JSON file | Hardcoded |
| Logging | File + console | UART printf |
| Error handling | Exceptions | Return codes |

## Future Extensions

1. **Interrupt-driven RX**: Use RB0/INT0 for immediate CAN receive
2. **Multi-frame ISO-TP**: Add buffer and Flow Control support for VIN/CALID
3. **Display Support**: Add SSD1306 via SPI for standalone operation
4. **EEPROM Storage**: Store configuration and DTC history
5. **Sleep Mode**: Low-power mode between polls
6. **Multiple PIDs per Request**: Send multiple PIDs in one CAN frame
