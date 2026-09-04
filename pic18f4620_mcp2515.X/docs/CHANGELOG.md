# Changelog — pic18f4620_mcp2515

All notable changes to this project will be documented in this file.
The format is based on Keep a Changelog, and this project adheres to Semantic Versioning.

## [1.0.0] - 2026-09-04

### Added
- Initial project structure for PIC18F4620 + MCP2515 OBD-II CAN reader
- `mcp2515.c/h` — Low-level MCP2515 SPI driver for PIC18F4620
  - SPI Mode 0,0 initialization (Fosc/4 = 5 MHz @ 20 MHz crystal)
  - MCP2515 reset, register read/write, bit-modify primitives
  - 500 kbps CAN bitrate configuration (8 MHz MCP2515 crystal)
  - Single-frame CAN send/receive on TXB0/RXB0
  - RXB0 rollover disabled; RX mask configured to accept all IDs
  - CAN interrupt enable for RXB0 message received
- `main.c` — OBD-II Mode 01 reader application
  - UART1 @ 9600 baud, 8N1, redirected `printf()` via `putch()`
  - Periodic polling of PIDs: RPM (0x0C), speed (0x0D), coolant temp (0x05), engine load (0x04), throttle (0x11)
  - Blocking receive timeout (~1 s) with `__delay_ms(2)` polling loop
  - Human-readable output via UART
- `docs/SKILL.md` — Technical skill documentation for the project

### Hardware
- PIC18F4620 @ 20 MHz HS oscillator
- MCP2515 @ 8 MHz crystal
- SPI on RC3 (SCK), RC4 (SDI), RC5 (SDO)
- CS on RA0 (active-low chip select)
- INT on RB0/INT0 (optional, not used in current code)
- UART on RC6 (TX), RC7 (RX)

### Known Limitations
- Only TXB0 used for transmission; TXB1/TXB2 unused
- Only RXB0 used for reception; RXB1 available for future expansion
- No ISO-TP multi-frame support (single-frame only, max 8 bytes)
- No interrupt-driven CAN reception; polling-based
- No error handling/reporting for CAN bus errors
- No filter/mask configuration beyond "accept all"

### References
- Learned from `tmp/mcp2515_integral/scanner/autel_scanner/` (C++ Raspberry Pi scanner)
- Learned from `tmp/mcp2515_integral/scanner/reader/` (Raspberry Pi OBD-II reader)
- MCP2515 register map and SPI commands from Microchip DS20001801
