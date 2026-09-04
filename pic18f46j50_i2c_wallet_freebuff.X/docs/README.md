# PIC18F46J50 - Crypto Wallet (FreeBuff OLED, Trezor-like)

**Descripción:** Cartera cripto estilo Trezor con display OLED SSD1306 y navegación por 2 botones (RB4=Move, RB5=Enter). Versión sin USB, solo interfaz I2C para OLED.

## Características
- OLED SSD1306 128×32px (I2C por software)
- Navegación: RB4 (Move/Scroll), RB5 (Enter/Select)
- Menú wallet con init y update loop
- Fosc = 16MHz (INTOSC)

## Periféricos Utilizados
- I2C (bit-bang), GPIO

## Archivos
| Archivo | Descripción |
|---------|-------------|
| main.c | Inicialización y loop |
| wallet.c/h | Lógica de cartera |
| oled.c/h | Driver OLED |
| i2c.c/h | I2C por software |
| hardware_cfg.h | Configuración pines |
