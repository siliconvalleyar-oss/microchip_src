# Learnings — from MCP2515 Integral Scanner Projects

This document captures lessons learned from studying the C++ scanner implementations in `tmp/mcp2515_integral/scanner/`.

## From scanner/autel_scanner (Raspberry Pi CAN + OLED)

### Architecture Patterns

1. **Singleton Config**: `Config::instance()` pattern for global configuration
   - Auto-detects config directory from executable path
   - Falls back to `config/` in current directory
   - Persists to JSON with pretty-print

2. **EventLog Singleton**: Thread-safe logging with mutex
   - Levels: TRACE, DEBUG, INFO, WARN, ERROR
   - Environment variables: AUTEL_LOG_LEVEL, AUTEL_LOG_FILE
   - Falls back from `/var/log/` to local directory

3. **Dependency Injection**: Scanner subsystems receive dependencies via `setDependencies()`
   - OBD2 -> MCP2515
   - LiveData -> OBD2
   - ActiveTest -> OBD2
   - Menu -> Display, OBD2, DTCManager, LiveData, ActiveTest

4. **RAII**: All hardware classes use constructor/destructor for init/cleanup
   - `~MCP2515()` calls `cleanup()`
   - `~SPI()` closes file descriptor

### CAN/OBD-II Patterns

1. **ISO-TP Multi-frame**: Full implementation in `OBD2::receiveISO15765()`
   - Single Frame (0x00), First Frame (0x10), Flow Control (0x30), Consecutive Frame (0x20)
   - Sequence validation, timeout handling
   - Buffer overflow protection

2. **Response ID Matching**: Accepts 0x7E8, 0x7E9, 0x7EA (ECU #1, #2, #3)
   ```cpp
   if (msg.id == responseId_ || msg.id == responseId_ + 1 || msg.id == responseId_ + 2)
   ```

3. **EventLog for Diagnostics**: Every CAN operation is logged
   - TX/RX with byte counts
   - Timeout warnings
   - Error flags (TEC, REC, EFLG)

4. **Headless Mode**: Display is optional; scanner works without OLED
   - SSD1306 init failure logs error but returns success
   - Display pointer null-checked before every operation

### Menu System

1. **Tree Structure**: Recursive `MenuItem` with children vector
2. **Keyboard Input**: Non-blocking `select()` with 100ms timeout
3. **Escape Sequences**: Full ANSI/VT100 support for arrow keys
4. **Console + Display**: Dual output (terminal + OLED)

---

## From scanner/reader (Bluetooth ELM327 + OLED SPI)

### Threading Model

1. **Producer-Consumer**: OBD poll thread writes to `VehicleData` mutex
   - Main thread reads for display
   - Keyboard thread is separate from poll thread

2. **Atomic Flags**: `g_running` for clean shutdown
   - Signal handlers set flag
   - All threads check flag in loops

3. **Terminal Raw Mode**: `TermRaw` RAII wrapper for non-blocking keyboard
   - Sets ICANON + ECHO off
   - Sets O_NONBLOCK on stdin
   - Restores on destruction

### ELM327 Communication

1. **Init Sequence**: ATZ, ATE0, ATL0, ATS0, ATSP0, ATAT1, ATST20
2. **Response Parsing**: Strip CR/LF/spaces/>, uppercase, split by 2-char bytes
3. **GM Mode 22**: Custom header setting (AT SH 7E0) with flow control
4. **Timeout Handling**: `select()` with 600ms timeout on socket

### Display Architecture

1. **Page-based**: 7 pages (MAIN, ADMISSION, FUEL_TRIM, O2_SENSOR, GM_DATA, DTC, DEBUG_BT)
2. **Auto-rotate**: Configurable interval, can be toggled
3. **CSV Logging**: Separate logger thread-safe class

---

## Ported to PIC18F4620 C Implementation

### Key Differences

| Aspect | C++ Raspberry Pi | PIC18F4620 C |
|--------|------------------|--------------|
| Memory | Heap (new/delete) | Static/stack only |
| Threads | std::thread | None (polling) |
| SPI | /dev/spidev (bcm2835) | MSSP hardware module |
| CAN | SocketCAN or MCP2515 SPI | MCP2515 SPI direct |
| Display | SSD1306 OLED | UART debug only |
| Config | JSON file | Hardcoded defaults |
| Logging | File + stdout | UART printf |

### Lessons Applied

1. **Minimal Dependencies**: No dynamic allocation in PIC code
2. **Blocking Receive**: Simpler than interrupt-driven for this application
3. **Single TX Buffer**: TXB0 only; sufficient for request-response pattern
4. **Polling Loop**: 2ms poll interval in `OBD2_WaitResponse()`
5. **No ISO-TP Yet**: Multi-frame requires more RAM; single-frame for initial version

### Potential Improvements (from C++ reference)

1. **Add RXB1 with rollover**: For higher throughput
2. **Add filters**: Reduce CPU load filtering in software
3. **Add interrupt-driven RX**: INT0 on RB0 for immediate response
4. **Add timeout per PID**: Some PIDs take longer than others
5. **Add error counters**: TEC/REC monitoring for bus diagnostics
6. **Add retry logic**: Re-send on NACK or timeout
7. **Add VIN support**: Multi-frame Mode 09 PID 02

## References

- `tmp/mcp2515_integral/scanner/autel_scanner/src/scanner/obd2.cpp` — ISO-TP implementation
- `tmp/mcp2515_integral/scanner/autel_scanner/src/hardware/mcp2515.cpp` — MCP2515 C++ driver
- `tmp/mcp2515_integral/scanner/reader/src/elm327.cpp` — ELM327 BT communication
- `tmp/mcp2515_integral/docs/obd2-elm327-pid-reference/OBD2_Complete_PID_Reference_EN.md` — Full PID reference
