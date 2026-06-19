# PIC18F46J50 - LED Sequencer + OLED Display

**Descripción:** Secuenciador de LEDs con control por pulsadores y display OLED informativo. Los LEDs en RD0-RD3 hacen un barrido con dirección y velocidad ajustables.

## Características
- 4 LEDs en secuencia (RD0-RD3)
- 3 botones: UP (velocidad +), DOWN (velocidad -), DIR (cambiar dirección)
- OLED SSD1306 128×32px muestra velocidad y dirección
- I2C en RB0 (SDA) y RB1 (SCL) usando MSSP2
- Velocidad: 50-500ms, paso 50ms
- Frecuencia sistema: 8MHz

## Periféricos Utilizados
- MSSP2 (I2C), GPIO, OLED SSD1306

## Pines
| Pin | Función |
|-----|---------|
| RB0 | SDA (OLED) |
| RB1 | SCL (OLED) |
| RB4/RB5/RB6 | Botones UP/DOWN/DIR |
| RD0-RD3 | LEDs salida |
