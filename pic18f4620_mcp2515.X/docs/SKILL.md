# MCP2515 CAN Controller Driver — Technical Skill

## Descripción

Driver para el controlador CAN MCP2515 de Microchip para PIC18F4620. Implementa comunicación SPI, inicialización del dispositivo, envío y recepción de mensajes CAN 2.0A (11-bit standard). Diseñado para lectura OBD-II sobre CAN 500 kbps.

## Hardware

### MCP2515
- Controlador CAN 2.0B (11-bit standard, 29-bit extended)
- Interface SPI (hasta 10 MHz)
- 2 buffers de recepción (RXB0, RXB1)
- 3 buffers de transmisión (TXB0, TXB1, TXB2)
- Filtros y máscaras programables
- Interrupciones configurables
- Requiere cristal de 8 MHz para 500 kbps

### Conexión PIC18F4620 -> MCP2515

| Señal | PIC18F4620 | MCP2515 | Función |
|-------|------------|---------|---------|
| SCK | RC3 | SCK | SPI Clock |
| SDI | RC4 | SO | SPI Data Input |
| SDO | RC5 | SI | SPI Data Output |
| CS | RA0 | CS | Chip Select (activo bajo) |
| INT | RB0/INT0 | INT | Interrupción (opcional) |
| CANH | - | TXCAN/RXCAN | Bus CAN alto |
| CANL | - | RXCAN/TXCAN | Bus CAN bajo |

## SPI Configuration

- Modo SPI 0,0 (CPOL=0, CPHA=0)
- Clock: Fosc/4 = 5 MHz @ 20 MHz
- SMP = 0 (muestreo a mitad del pulso)
- CKE = 1 (dato válido en flanco activo->inactivo)
- SSPM = 0b0000 (SPI Master, Fosc/4)

## API del Driver

### Inicialización

```c
void mcp2515_init(void);
```

Inicializa SPI, resetea el MCP2515, configura bitrate 500 kbps y modos.

### Acceso a Registros

```c
uint8_t mcp2515_read_register(uint8_t addr);
void mcp2515_write_register(uint8_t addr, uint8_t data);
void mcp2515_bit_modify(uint8_t addr, uint8_t mask, uint8_t data);
```

### Envío y Recepción

```c
void mcp2515_send_message(uint16_t id, const uint8_t *data, uint8_t len);
uint8_t mcp2515_receive_message(uint16_t *id, uint8_t *data, uint8_t *len);
```

## Instrucciones SPI

| Instrucción | Código | Descripción |
|-------------|--------|-------------|
| RESET | 0xC0 | Reset del dispositivo |
| READ | 0x03 | Leer registro |
| WRITE | 0x02 | Escribir registro |
| RTS_TX0 | 0x81 | Request To Send TXB0 |
| RTS_TX1 | 0x82 | Request To Send TXB1 |
| RTS_TX2 | 0x84 | Request To Send TXB2 |
| RTS_ALL | 0x87 | Request To Send todos |
| READ_STATUS | 0xA0 | Leer estado |
| BIT_MODIFY | 0x05 | Modificar bits de registro |
| READ_RXB0 | 0x90 | Leer buffer RXB0 |
| READ_RXB1 | 0x94 | Leer buffer RXB1 |

## Registros Principales

| Registro | Dirección | Función |
|----------|-----------|---------|
| CANSTAT | 0x0E | Estado del controlador |
| CANCTRL | 0x0F | Control de modo |
| CNF1 | 0x2A | Configuración bitrate 1 (BRP, SJW) |
| CNF2 | 0x29 | Configuración bitrate 2 (BTLMODE, SAM, SEG2PH, SEG1PH) |
| CNF3 | 0x28 | Configuración bitrate 3 (SEG2PH, SOF) |
| CANINTE | 0x2B | Habilitación interrupciones |
| CANINTF | 0x2C | Flags de interrupción |
| TXB0CTRL | 0x30 | Control buffer TXB0 |
| TXB0SIDH | 0x31 | ID alta TXB0 |
| TXB0SIDL | 0x32 | ID baja TXB0 |
| TXB0DLC | 0x35 | Longitud TXB0 |
| TXB0D0 | 0x36 | Dato 0 TXB0 |
| RXB0CTRL | 0x60 | Control buffer RXB0 |
| RXB0SIDH | 0x61 | ID alta RXB0 |
| RXB0SIDL | 0x62 | ID baja RXB0 |
| RXB0DLC | 0x65 | Longitud RXB0 |
| RXB0D0 | 0x66 | Dato 0 RXB0 |

## Modos de Operación

| Modo | Valor | Descripción |
|------|-------|-------------|
| Configuration | 0x80 | Modo configuración |
| Normal | 0x00 | Modo normal (operación) |
| Sleep | 0x20 | Modo bajo consumo |
| Listen Only | 0x60 | Solo escucha |
| Loopback | 0x40 | Loopback interno |

## Configuración de Bitrate

Para 500 kbps con cristal de 8 MHz:
- CNF1 = 0x00 (BRP=0, SJW=0)
- CNF2 = 0x90 (BTLMODE=1, SAM=0, SEG2PH=6, SEG1PH=0)
- CNF3 = 0x02 (SEG2PH=2)

Fórmula: CAN bitrate = Fosc / (2 * (BRP+1) * (1 + TSEG1 + TSEG2))

## Formato de Mensaje CAN

### Estándar (11-bit)

| Campo | Bits | Descripción |
|-------|------|-------------|
| SIDH | 8 | ID bits 10-3 |
| SIDL | 8 | ID bits 2-0 + RTR + IDE |
| DLC | 4 | Longitud de datos |
| D0-D7 | 8 | Datos |

### ID construcción

```c
// Enviar
uint8_t sidh = (id >> 3) & 0xFF;
uint8_t sidl = (id & 0x07) << 5;

// Recibir
id = ((uint16_t)sidh << 3) | (sidl >> 5);
```

## OBD-II Protocol

### PIDs Modo 01

| PID | Descripción | Fórmula | Unidad |
|-----|-------------|---------|--------|
| 0x0C | RPM motor | (A*256+B)/4 | rpm |
| 0x0D | Velocidad | A | km/h |
| 0x05 | Temperatura refrigerante | A-40 | °C |
| 0x04 | Carga motor | A*100/255 | % |
| 0x11 | Posición acelerador | A*100/255 | % |
| 0x0B | Presión admisión | A | kPa |
| 0x0F | Temperatura admisión | A-40 | °C |
| 0x0E | Avance encendido | A/2-64 | ° |
| 0x10 | MAF | (A*256+B)/100 | g/s |
| 0x2F | Nivel combustible | A*100/255 | % |

### IDs CAN

| ID | Función |
|----|---------|
| 0x7DF | Request broadcast (functional) |
| 0x7E8 | Response ECU #1 |
| 0x7E9 | Response ECU #2 |

### Formato Request ISO-TP (Single Frame)

```
Byte 0: 0x02 (PCI: 2 bytes additional)
Byte 1: 0x01 (Mode 01)
Byte 2: PID
Byte 3-7: 0x00 padding
```

### Formato Response ISO-TP (Single Frame)

```
Byte 0: 0x41 (Mode 01 + 0x40 = response)
Byte 1: PID
Byte 2: Data A
Byte 3: Data B (optional)
```

## ISO-TP Multi-frame (Future)

Para respuestas >7 bytes (VIN, CALID):

### First Frame (FF)
```
Byte 0: 0x10 | (len >> 8)
Byte 1: len & 0xFF
Byte 2-7: First 6 data bytes
```

### Flow Control (FC) — Scanner → ECU
```
Byte 0: 0x30 (PCI Flow Control)
Byte 1: Block Size (0 = send all CF without pause)
Byte 2: STmin (0 = no minimum separation)
Byte 3-7: Reserved
```

### Consecutive Frame (CF)
```
Byte 0: 0x20 | sequence number (0-15, wraps)
Byte 1-7: Data payload (7 bytes)
```

## Ejemplo de Uso

```c
#include "mcp2515.h"

void main(void) {
    mcp2515_init();

    // Enviar request PID
    uint8_t data[] = {0x02, 0x01, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00};
    mcp2515_send_message(0x7DF, data, 8);

    // Recibir respuesta
    uint16_t id;
    uint8_t data[8];
    uint8_t len;
    if (mcp2515_receive_message(&id, data, &len)) {
        if (id == 0x7E8 && data[1] == 0x41 && data[2] == 0x0C) {
            uint16_t rpm = ((uint16_t)data[3] * 256 + data[4]) / 4;
            printf("RPM: %u\r\n", rpm);
        }
    }
}
```

## Troubleshooting

### Problema: MCP2515 no responde

**Solución**: Verificar:
1. Conexiones SPI (SCK, SDI, SDO)
2. CS en nivel alto cuando no se usa
3. Alimentación 5V estable
4. Cristal de 8 MHz conectado
5. MCP2515 no en modo Configuration al inicializar

### Problema: Mensajes no se envían

**Solución**: Verificar:
1. MCP2515 en modo normal (CANSTAT & 0xE0 == 0x00)
2. Buffer TXB0 vacío (TXB0CTRL bit 3 = 0)
3. Terminación 120 ohm en CANH/CANL
4. Transceiver SN65HVD230 alimentado y conectado

### Problema: No se reciben mensajes

**Solución**: Verificar:
1. CANINTF bit 0 = 1 (mensaje en RXB0)
2. Filtros configurados correctamente (aceptan ID)
3. Modo normal operativo
4. Bus CAN activo (otro dispositivo transmitiendo)

### Problema: OBD-II sin respuesta del vehiculo

**Solución**: Verificar:
1. Conector OBD-II alimentado (Pin 16 = +12V)
2. CAN bus presente en vehiculo (ISO 15765-4)
3. Bitrate 500 kbps coincide con vehiculo
4. Terminacion correcta (120 ohm en ambos extremos)

## Lessons from C++ Reference

### scanner/autel_scanner (Raspberry Pi)
- Response ID matching accepts 0x7E8, 0x7E9, 0x7EA (multiple ECUs)
- Full ISO-TP multi-frame implementation for VIN/CALID
- EventLog pattern for diagnostics (TEC, REC, EFLG)
- Headless mode: display is optional, scanner continues without OLED

### scanner/reader (Raspberry Pi)
- ELM327 init sequence: ATZ, ATE0, ATL0, ATS0, ATSP0, ATAT1, ATST20
- GM Mode 22: custom header (AT SH 7E0) with flow control
- Response parsing: strip CR/LF/spaces/>, uppercase, split by 2-char bytes

## References

- [MCP2515 Datasheet](https://www.microchip.com/wwwproducts/en/MCP2515)
- [PIC18F4620 Datasheet](https://www.microchip.com/wwwproducts/en/PIC18F4620)
- [ISO 15765-4](https://www.iso.org/standard/66346.html)
- [OBD-II PIDs](https://en.wikipedia.org/wiki/OBD-II_PIDs)
- [SAE J1979](https://www.sae.org/standards/content/j1979/)
- `tmp/mcp2515_integral/scanner/autel_scanner/` — C++ scanner reference
- `tmp/mcp2515_integral/scanner/reader/` — C++ OBD-II reader reference
- `tmp/mcp2515_integral/docs/obd2-elm327-pid-reference/` — Complete PID reference
