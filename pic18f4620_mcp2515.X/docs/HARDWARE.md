# Hardware Documentation — PIC18F4620 + MCP2515 OBD-II Reader

## Esquematico de Conexion

### PIC18F4620 -> MCP2515 (SPI)

| Señal PIC18F4620 | Pin PIC | Señal MCP2515 | Pin MCP2515 | Función |
|------------------|---------|---------------|-------------|---------|
| SCK              | RC3     | SCK           | Pin 18      | SPI Clock (salida PIC) |
| SDI              | RC4     | SO            | Pin 21      | SPI Data Input (entrada PIC) |
| SDO              | RC5     | SI            | Pin 19      | SPI Data Output (salida PIC) |
| CS               | RA0     | CS            | Pin 17      | Chip Select (activo bajo) |
| INT              | RB0     | INT           | Pin 16      | Interrupcion (opcional) |
| VCC              | -       | VCC           | Pin 1       | 5V alimentacion |
| GND              | -       | GND           | Pin 8       | Tierra |

### MCP2515 -> Transceptor CAN (SN65HVD230) -> OBD-II

| MCP2515 | SN65HVD230 | OBD-II Pin | Función |
|---------|------------|------------|---------|
| TXCAN   | TXD        | Pin 6      | CAN High (transmisión) |
| RXCAN   | RXD        | Pin 14     | CAN Low (recepción) |
| VCC     | VCC        | Pin 16     | 5V alimentación |
| GND     | GND        | Pin 4      | Chassis GND |
| CANH    | CANH       | Pin 6      | Bus CAN alto |
| CANL    | CANL       | Pin 14     | Bus CAN bajo |

### OBD-II Connector (Pinout SAE J1962)

```
        ┌─────────────┐
        │    OBD-II   │
        │  Connector  │
        └─────────────┘
         1  2  3  4  5
         6  7  8  9 10
        11 12 13 14 15
        16 ──────── (reserved)
```

| Pin | Función | Color tipico |
|-----|---------|--------------|
| 4   | Chassis Ground | Negro |
| 6   | CAN High (CANH) | Amarillo |
| 14  | CAN Low (CANL) | Verde |
| 16  | Battery (+12V) | Rojo |

## Conexion Completa

```
PIC18F4620 (20 MHz)
     │
     ├── SPI ──── MCP2515 ─── SN65HVD230 ─── OBD-II (Pin 6/14)
     │             (8 MHz)     (CAN Transceiver)
     │
     ├── UART ──── USB-TTL ──── PC Terminal (debug)
     │
     └── Power ──── 5V regulator ──── MCP2515 + SN65HVD230
```

## Lista de Componentes

1. PIC18F4620 microcontroller
2. MCP2515 CAN Bus Module (con cristal de 8 MHz)
3. SN65HVD230 CAN Transceiver
4. OBD-II Connector (macho, SAE J1962)
5. Cristal 20 MHz para PIC18F4620
6. Cristal 8 MHz para MCP2515
7. Condensadores de desacople 100nF en VCC de cada IC
8. Regulador 5V (si se alimenta desde USB o 12V)
9. Cables jumper / protoboard

## Configuracion de Bits MCP18F4620 (XC8)

```c
#pragma config OSC     = HS        // Oscilador HS (cristal 20 MHz)
#pragma config WDT     = OFF       // Watchdog apagado
#pragma config LVP     = OFF       // Bajo voltaje de programacion apagado
#pragma config PBADEN  = OFF       // PORTB<4:0> como digital al reset
#pragma config MCLRE   = ON        // MCLR habilitado
#pragma config DEBUG   = OFF
#pragma config PWRT    = ON
```

## Notas

- El MCP2515 requiere un transceptor CAN (SN65HVD230) para conversión de niveles y aislamiento
- La alimentacion del modulo MCP2515 debe ser 5V (no 3.3V)
- El display OLED del proyecto no esta implementado en esta version PIC; se usa UART para debug
- El cristal del MCP2515 debe ser de 8 MHz para la configuracion de bitrate implementada
- Terminación de 120 ohm en CANH/CANL segun ISO 11898 (en el conector OBD-II del vehiculo)
