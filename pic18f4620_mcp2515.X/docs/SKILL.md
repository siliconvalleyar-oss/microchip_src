# MCP2515 CAN Controller Driver

## Descripción

Driver para el controlador CAN MCP2515 de Microchip. Implementa comunicación SPI, inicialización del dispositivo, envío y recepción de mensajes CAN 2.0B.

## Hardware

### MCP2515
- Controlador CAN 2.0B (11 bits)
- Interface SPI (hasta 10 MHz)
- 2 buffers de recepción (RXB0, RXB1)
- 3 buffers de transmisión (TXB0, TXB1, TXB2)
- Filtros y máscaras programables
- Interrupciones configurables

### Conexión PIC18F4620 -> MCP2515

| Señal | PIC18F4620 | MCP2515 | Función |
|-------|------------|---------|---------|
| SCK | RC3 | SCK | SPI Clock |
| SDI | RC4 | SO | SPI Data Input |
| SDO | RC5 | SI | SPI Data Output |
| CS | RA0 | CS | Chip Select (activo bajo) |
| INT | RB0/INT0 | INT | Interrupción (opcional) |
| CANH | - | CANH | Bus CAN alto |
| CANL | - | CANL | Bus CAN bajo |

## SPI Configuration

- Modo SPI 0,0 (CPOL=0, CPHA=0)
- Clock: Fosc/4 = 5 MHz @ 20 MHz
- SMP = 0 (muestreo a mitad del pulso)
- CKE = 1 (dato válido en flanco activo->inactivo)

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
| RTS | 0x80 | Request To Send (TXB0/1/2) |
| READ_STATUS | 0xA0 | Leer estado |
| BIT_MODIFY | 0x05 | Modificar bits de registro |
| READ_RXB0 | 0x90 | Leer buffer RXB0 |
| READ_RXB1 | 0x94 | Leer buffer RXB1 |

## Registros Principales

| Registro | Dirección | Función |
|----------|-----------|---------|
| CANSTAT | 0x0E | Estado del controlador |
| CANCTRL | 0x0F | Control de modo |
| CNF1 | 0x2A | Configuración bitrate 1 |
| CNF2 | 0x29 | Configuración bitrate 2 |
| CNF3 | 0x28 | Configuración bitrate 3 |
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
- CNF2 = 0x90 (BTLMODE=1, SAM=0, SEG2PH=6)
- CNF3 = 0x02 (SEG2PH=2)

Fórmula: CAN bitrate = Fosc / (2 * BRP * (1 + TSEG1 + TSEG2))

## Formato de Mensaje CAN

### Estándar (11 bits)

| Campo | Bits | Descripción |
|-------|------|-------------|
| SIDH | 8 | ID bits 10-3 |
| SIDL | 8 | ID bits 2-0 + RTR + IDE |
| DLC | 4 | Longitud de datos |
| D0-D7 | 8 | Datos |

### ID construcción

```c
id = ((uint16_t)sidh << 3) | (sidl >> 5);
```

## OBD-II Protocol

### PIDs Modo 01

| PID | Descripción | Fórmula |
|-----|-------------|---------|
| 0x0C | RPM motor | (A*256+B)/4 |
| 0x0D | Velocidad | A (km/h) |
| 0x05 | Temperatura | A-40 (°C) |
| 0x04 | Carga motor | A*100/255 (%) |
| 0x11 | Acelerador | A*100/255 (%) |

### IDs CAN

| ID | Función |
|----|---------|
| 0x7DF | Request broadcast |
| 0x7E8 | Response ECU #1 |
| 0x7E9 | Response ECU #2 |

## Ejemplo de Uso

```c
#include "mcp2515.h"

void main(void) {
    mcp2515_init();

    // Enviar mensaje
    uint8_t data[] = {0x02, 0x01, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00};
    mcp2515_send_message(0x7DF, data, 8);

    // Recibir mensaje
    uint16_t id;
    uint8_t data[8];
    uint8_t len;
    if (mcp2515_receive_message(&id, data, &len)) {
        // Procesar mensaje
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

### Problema: Mensajes no se envían

**Solución**: Verificar:
1. MCP2515 en modo normal (CANSTAT & 0xE0 == 0x00)
2. Buffer TXB0 vacío (TXB0CTRL bit 3 = 0)
3. Terminación 120 ohm en CANH/CANL

### Problema: No se reciben mensajes

**Solución**: Verificar:
1. Filtros configurados correctamente
2. CANINTF bit 0 = 1 (mensaje en RXB0)
3. Modo normal operativo

## Referencias

- [MCP2515 Datasheet](https://www.microchip.com/wwwproducts/en/MCP2515)
- [ISO 15765-4](https://www.iso.org/standard/66346.html)
- [OBD-II PIDs](https://en.wikipedia.org/wiki/OBD-II_PIDs)
