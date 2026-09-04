# PIC18F4620 + MCP2515 - Lector OBD-II CAN

**Descripción:** Lector de parámetros OBD-II mediante bus CAN usando PIC18F4620 y controlador MCP2515. Lee RPM, velocidad, temperatura del motor, carga y posición del acelerador desde la ECU del vehículo.

## Características
- Protocolo CAN 2.0B (11 bits) a 500 kbps
- MCP2515 controlador CAN via SPI
- OBD-II PIDs: RPM, velocidad, temperatura, carga motor, acelerador
- Salida por UART 9600 baudios
- Polling de mensajes CAN
- Fosc = 20MHz

## Periféricos Utilizados
- SPI (MSSP)
- UART (EUSART)
- GPIO
- MCP2515 CAN Controller

## Pines
| Pin | Función |
|-----|---------|
| RC3 | SCK (SPI clock) |
| RC4 | SDI (SPI data input) |
| RC5 | SDO (SPI data output) |
| RA0 | CS (Chip Select, active low) |
| RC6 | TX (UART) |
| RC7 | RX (UART) |

## Archivos
| Archivo | Descripción |
|---------|-------------|
| `main.c` | Aplicación principal OBD-II |
| `mcp2515.c` | Driver MCP2515 |
| `mcp2515.h` | API pública del driver |
| `docs/SKILL.md` | Documentación técnica |

## Uso
1. Conectar MCP2515 al PIC18F4620 via SPI
2. Conectar CANH/CANL al conector OBD-II (pines 6 y 14)
3. Conectar UART a PC via USB-TTL
4. Compilar y grabar
5. Abrir terminal serie a 9600 baudios

## Protocolo OBD-II
- ID request: 0x7DF (broadcast)
- ID response: 0x7E8 (ECU #1)
- Modo 01: Datos en tiempo real
- PIDs soportados:
  - 0x0C: RPM motor
  - 0x0D: Velocidad vehículo
  - 0x05: Temperatura refrigerante
  - 0x04: Carga motor
  - 0x11: Posición acelerador
