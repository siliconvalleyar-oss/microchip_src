# PIC18F4550 - I2C Master Communication

**Descripción:** Demostración de comunicación I2C en modo Master con el PIC18F4550. Envía una trama I2C completa (START + dirección + datos + STOP) mientras anima LEDs en RD0-RD3.

## Características
- I2C Master implementado con módulo MSSP (100kHz)
- Pines: RB0=SDA, RB1=SCL
- LEDs en RD0-RD3 con animación secuencial
- Fosc = 4MHz (cristal HS)

## Periféricos Utilizados
- MSSP (I2C Master)
- GPIO (LEDs)

## Pines
| Pin | Función |
|-----|---------|
| RB0 | SDA |
| RB1 | SCL |
| RD0-RD3 | LEDs de salida |
