# TODO — Future Work for PIC18F4620 MCP2515 OBD-II Reader

## High Priority

### CAN Improvements
- [ ] Implement interrupt-driven CAN receive (RB0/INT0)
- [ ] Add RXB1 with rollover for higher throughput
- [ ] Add CAN filters/masks to reduce software filtering
- [ ] Implement CAN error counters (TEC/REC) monitoring
- [ ] Add retry logic for failed transmissions

### OBD-II Protocol
- [ ] Implement ISO-TP multi-frame support (VIN, CALID)
  - [ ] Flow Control frame generation
  - [ ] Consecutive Frame handling with sequence validation
  - [ ] Buffer for multi-frame responses (64 bytes)
- [ ] Add Mode 03 DTC read with proper parsing
- [ ] Add Mode 04 DTC clear
- [ ] Add Mode 09 Vehicle Info (VIN, CALID, CVN)
- [ ] Add Mode 22 extended PIDs (GM-specific)

### Display
- [ ] Add SSD1306 OLED driver via SPI
- [ ] Implement display pages (similar to C++ reader)
- [ ] Add menu system for PID selection

## Medium Priority

### Code Quality
- [ ] Add unit tests for PID decoding functions
- [ ] Add CAN bus error handling and reporting
- [ ] Implement configurable PID list (EEPROM/Flash)
- [ ] Add watchdog timer support
- [ ] Add low-power sleep mode between polls

### Features
- [ ] Add SD card logging (CSV format)
- [ ] Add Bluetooth module (HC-05) for wireless data
- [ ] Add real-time clock (MCP7940N or similar)
- [ ] Add data buffering for offline analysis
- [ ] Implement GM Mode 22 extended commands

### Documentation
- [ ] Create schematic in KiCad/Eagle
- [ ] Add PCB layout files
- [ ] Write assembly guide
- [ ] Create 3D-printed enclosure design

## Low Priority

### Advanced Features
- [ ] Add CAN FD support (requires MCP2517FD or similar)
- [ ] Add J1939 support (heavy-duty vehicles)
- [ ] Add K-Line support (ISO 9141-2, KWP2000)
- [ ] Add LIN bus support
- [ ] Add multiple ECUs support (broadcast mode)

### Optimization
- [ ] Optimize SPI timing for faster bitrate
- [ ] Implement DMA for SPI transfers
- [ ] Reduce code size for smaller PIC variants
- [ ] Add bootloader support

## Reference Implementation Status

| Feature | Status | Notes |
|---------|--------|-------|
| SPI MCP2515 driver | ✅ Complete | TXB0/RXB0 only |
| 500 kbps bitrate | ✅ Complete | 8 MHz crystal |
| UART debug output | ✅ Complete | 9600 baud |
| OBD-II Mode 01 | ✅ Partial | 5 PIDs implemented |
| ISO-TP multi-frame | ❌ Not started | Needs RXB1 + buffer |
| Display | ❌ Not started | SSD1306 planned |
| Logging | ❌ Not started | SD card or EEPROM |
| Error handling | ❌ Minimal | Basic timeout only |

## References

- C++ Scanner: `tmp/mcp2515_integral/scanner/autel_scanner/`
- C++ Reader: `tmp/mcp2515_integral/scanner/reader/`
- PID Reference: `tmp/mcp2515_integral/docs/obd2-elm327-pid-reference/`
- MCP2515 Datasheet: DS20001801
- PIC18F4620 Datasheet: DS39626D
