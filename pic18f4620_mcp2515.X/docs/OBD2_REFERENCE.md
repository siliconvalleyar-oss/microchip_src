# OBD-II Reference — PID and Protocol Documentation

## CAN IDs ISO 15765-4

| ID | Función |
|----|---------|
| 0x7DF | Request broadcast (functional) |
| 0x7E8 | Response ECU #1 |
| 0x7E9 | Response ECU #2 |

---

## PIDs Modo 01 — Datos en Tiempo Real

| PID | Nombre | Bytes | Fórmula | Unidad |
|-----|--------|-------|---------|--------|
| 0x0C | RPM | 2 | (A*256 + B) / 4 | rpm |
| 0x0D | Velocidad | 1 | A | km/h |
| 0x05 | Temp. Refrigerante | 1 | A - 40 | °C |
| 0x04 | Carga Motor | 1 | A * 100 / 255 | % |
| 0x11 | Posición Acelerador | 1 | A * 100 / 255 | % |
| 0x0B | Presión Admisión | 1 | A | kPa |
| 0x0F | Temp. Admisión | 1 | A - 40 | °C |
| 0x0E | Avance Encendido | 1 | (A / 2) - 64 | ° |
| 0x10 | MAF | 2 | (A*256 + B) / 100 | g/s |
| 0x2F | Nivel Combustible | 1 | A * 100 / 255 | % |
| 0x0A | Presión Combustible | 1 | A * 3 | kPa |
| 0x33 | Presión Barométrica | 1 | A | kPa |

---

## Fuel Trim

| PID | Nombre | Bytes | Fórmula | Unidad |
|-----|--------|-------|---------|--------|
| 0x06 | STFT B1 | 1 | ((A - 128) * 100) / 128 | % |
| 0x08 | STFT B2 | 1 | ((A - 128) * 100) / 128 | % |
| 0x07 | LTFT B1 | 1 | ((A - 128) * 100) / 128 | % |
| 0x09 | LTFT B2 | 1 | ((A - 128) * 100) / 128 | % |

---

## Sensores O2

| PID | Banco/Sensor | Bytes | Voltaje | Trim |
|-----|-------------|-------|---------|------|
| 0x14 | B1S1 | 2 | A * 0.005 | ((B-128)*100)/128 % |
| 0x15 | B1S2 | 2 | A * 0.005 | ((B-128)*100)/128 % |
| 0x16 | B1S3 | 2 | A * 0.005 | ((B-128)*100)/128 % |
| 0x17 | B1S4 | 2 | A * 0.005 | ((B-128)*100)/128 % |
| 0x18 | B2S1 | 2 | A * 0.005 | ((B-128)*100)/128 % |
| 0x19 | B2S2 | 2 | A * 0.005 | ((B-128)*100)/128 % |
| 0x1A | B2S3 | 2 | A * 0.005 | ((B-128)*100)/128 % |
| 0x1B | B2S4 | 2 | A * 0.005 | ((B-128)*100)/128 % |

---

## OBD-II Modes

| Modo | Nombre | Descripción |
|------|--------|-------------|
| 01 | Datos en vivo | PIDs de datos actuales del vehículo |
| 02 | Freeze Frame | Datos capturados en el momento del DTC |
| 03 | Leer DTCs | Códigos de error almacenados |
| 04 | Borrar DTCs | Limpiar códigos almacenados |
| 05 | O2 Monitor | Resultados de test de sensores O2 |
| 06 | On-Board Monitoring | Resultados de test continuos |
| 07 | Leer DTCs pendientes | Códigos pendientes |
| 08 | Test especial | Control de actuadores |
| 09 | Info vehículo | VIN, CALID, CVN |
| 0A | Leer DTCs permanentes | Códigos permanentes |

---

## Formato de Request/Response ISO 15765-4

### Single Frame (SF) — 1-7 bytes

```
Byte 0: PCI = 0x00 | length (0x01-0x07)
Byte 1..N: Data payload
```

### First Frame (FF) — >7 bytes

```
Byte 0: PCI = 0x10 | length high nibble
Byte 1: Length low byte
Byte 2..7: First data bytes (6 bytes)
```

### Flow Control (FC) — Enviado por el scanner

```
Byte 0: PCI = 0x30
Byte 1: Block Size (0 = enviar todos los CF sin pausa)
Byte 2: STmin (0 = sin separación mínima)
Byte 3..7: Reserved
```

### Consecutive Frame (CF)

```
Byte 0: PCI = 0x20 | sequence number (0-15)
Byte 1..7: Data payload (7 bytes)
```

---

## Codigos DTC

| Tipo | Rango | Significado |
|------|-------|-------------|
| P | P0000-P3FFF | Powertrain (motor/transmisión) |
| C | C0000-C3FFF | Chassis |
| B | B0000-B3FFF | Body |
| U | U0000-U3FFF | Network |

Formato: `P/C/B/U` + `0-3` + `0-3` + `0-F` + `0-F`

---

## PIDs GM Modo 22 (UDS)

| PID | Nombre | Bytes | Fórmula | Unidad |
|-----|--------|-------|---------|--------|
| B100 | Odómetro | 4 | raw32 / 10 | km |
| 01B4 | Temp. Catalizador | 2 | raw16 * 0.1 - 40 | °C |
| 1180 | Presión Combustible | 2 | raw16 * 4 | kPa |
| 01A9 | Torque Motor | 2 | raw16 * 0.5 - 848 | Nm |
| 01A1 | Voltaje ECU | 2 | raw16 * 0.001 | V |

---

## AT Commands (ELM327)

| Comando | Descripción |
|---------|-------------|
| ATZ | Reset adapter |
| ATE0 | Echo off |
| ATL0 | Linefeed off |
| ATS0 | Remove spaces |
| ATSP0 | Auto-detect protocol |
| ATSP6 | ISO 15765-4 CAN 11-bit 500 kbps |
| ATDP | Display protocol |
| ATSH 7DF | Set header |
| ATCRA 7E8 | Set response address |
| ATFC SH 7E0 | Set flow control |
| ATFC SD 30 00 00 | Flow control data |
| ATFC SM 1 | Flow control mode |

---

## Standards

- SAE J1979 / ISO 15031-5 — PIDs de datos
- ISO 14229 (UDS) — Diagnóstico unificado
- ISO 15765-4 (CAN) — CAN bus para diagnóstico
- ISO 27145-2 (WWH-OBD) — Diagnóstico mundial

---

## References

- MCP2515 Datasheet: DS20001801
- PIC18F4620 Datasheet: DS39626D
- ISO 15765-4: Road vehicles — CAN
- SAE J1962: OBD-II connector
- SAE J1979: E/E diagnostic test modes
- ISO 15031-5: Road vehicles — Diagnostic communication
