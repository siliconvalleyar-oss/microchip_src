# Workflow — PIC18F4620 MCP2515 Development

## Build Instructions

```bash
# From project root
make

# Clean
make clean
```

## Programming

```bash
# Using PICkit 3/4 with MPLAB X IDE
# Or command line with pk2cmd / pk4cmd
```

## Debugging

1. Connect UART to PC (9600 baud, 8N1)
2. Monitor output for:
   - MCP2515 initialization status
   - CAN bus communication results
   - OBD-II PID responses
3. Use MPLAB X debugger for step-through debugging

## Development Cycle

1. Modify source code in `src/` or project root
2. Build with `make`
3. Program PIC18F4620 with PICkit
4. Open serial monitor (9600 baud) to view debug output
5. Connect OBD-II cable to vehicle
6. Verify CAN communication with scanner tool

## Git Workflow

```bash
# Branch naming
git checkout -b feature/<feature-name>

# Conventional commits
git commit -m "feat: add new PID support"
git commit -m "fix: correct CAN ID encoding"
git commit -m "docs: update API reference"
git commit -m "chore: bump version"
```

## Versioning

- Follow Semantic Versioning
- Tags must match VERSION file
- Patch cycle: 0-9, then bump minor
- Every push must have a tag

## Hardware Testing

1. Verify SPI signals with oscilloscope (SCK, MOSI, MISO, CS)
2. Verify CAN bus with oscilloscope or CAN analyzer
3. Test with known-good OBD-II vehicle
4. Verify UART output with serial terminal

## Troubleshooting

### MCP2515 not initializing
- Check SPI connections (SCK, SDI, SDO, CS)
- Verify 8 MHz crystal on MCP2515
- Check power supply (5V)
- Verify MCP2515 CS pin toggling

### CAN messages not sending
- Verify MCP2515 in Normal mode (CANSTAT & 0xE0 == 0x00)
- Check TXB0CTRL bit 3 (TXREQ) clears after send
- Verify 120 ohm termination on CANH/CANL
- Check transceiver (SN65HVD230) connections

### CAN messages not receiving
- Check CANINTF bit 0 for RXB0 interrupt
- Verify filters/masks configured to accept IDs
- Check CAN bus is active with other device

### OBD-II no response
- Verify vehicle OBD-II port is powered (Pin 16 = +12V)
- Check CAN bus termination
- Try different vehicle (some vehicles use CAN FD)
- Verify 500 kbps bitrate matches vehicle

## References

- tmp/mcp2515_integral/scanner/autel_scanner/ — C++ scanner reference implementation
- tmp/mcp2515_integral/scanner/reader/ — Bluetooth ELM327 reader reference
- tmp/mcp2515_integral/docs/obd2-elm327-pid-reference/ — Complete PID reference
