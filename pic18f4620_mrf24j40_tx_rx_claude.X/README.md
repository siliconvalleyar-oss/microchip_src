# PIC18F4620 - MRF24J40 Wireless Transceiver (Claude)

**Descripción:** Comunicación inalámbrica IEEE 802.15.4 usando módulo MRF24J40 con PIC18F4620. Soporta modos TX y RX seleccionables por compilación. Incluye OLED, USART para debug, LEDs indicadores y botones.

## Características
- Modo TX: transmite mensajes cada 2s con contador, botones para TX manual y cambio de destino
- Modo RX: recibe paquetes y los muestra en OLED + USART
- OLED SSD1306 para display de estado
- USART 9600 baud para debug
- LEDs indicadores de actividad TX/RX/error
- Botones RB4 (TX manual/limpiar), RB5 (cambiar destino/ACK)
- PAN ID configurable, canal configurable

## Periféricos Utilizados
- SPI, USART (EUSART), I2C (bit-bang), GPIO, MRF24J40

## Pines
| Pin | Función |
|-----|---------|
| RB4/RB5 | Botones |
| RD0-RD3 | LEDs |
| SPI | MRF24J40 |
| I2C | OLED |
| RC6/RC7 | USART debug |
