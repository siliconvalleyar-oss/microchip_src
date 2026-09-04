# PIC18F4620 - MRF24J40 Wireless Transceiver (DeepSeek)

**Descripción:** Comunicación inalámbrica IEEE 802.15.4 usando MRF24J40 con interrupción externa INT1 (RB1). Dos proyectos en uno: TX (envía texto o datos) y RX (recibe y muestra estadísticas LQI/RSSI). Incluye OLED, USART y test SPI.

## Características
- Interrupción INT1 (RB1) para eventos del MRF24J40 (flanco de bajada)
- Modo TX: envía texto "Hola PIC18F4620" o 100 bytes de datos cada 2s
- Modo RX: muestra en OLED #recibidos, LQI, RSSI
- Comandos USART: s=stats, t=texto, d=datos, c=clear
- Botones RB4 (TX texto), RB5 (TX datos)
- LEDs: ACT, TX, RX, ERR
- Test SPI con lectura de registro SOFTRST
- USART 19200 baud

## Periféricos Utilizados
- SPI, USART, I2C (bit-bang), INT1, GPIO, MRF24J40

## Pines
| Pin | Función |
|-----|---------|
| RB1 | INT1 (MRF24J40 IRQ) |
| RB4/RB5 | Botones |
| RD0-RD3 | LEDs ACT/TX/RX/ERR |
