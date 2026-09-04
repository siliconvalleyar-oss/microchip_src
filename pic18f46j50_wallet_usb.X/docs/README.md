# PIC18F46J50 - Crypto Wallet with USB CDC + OLED

**Descripción:** Cartera cripto (Trezor-like) con display OLED SSD1306, interfaz USB CDC y navegación por menú con botones RB4 (Move) y RB5 (Enter). Envía "Hola mundo" por USB al iniciar.

## Características
- USB CDC con PLL a 48MHz desde INTOSC 16MHz
- OLED SSD1306 128×32px (I2C)
- 2 botones: RB4 (Move/Scroll), RB5 (Enter/Select)
- Menú wallet navegable
- Fosc = 16MHz + PLL

## Periféricos Utilizados
- USB (CDC), I2C (bit-bang), GPIO, OLED SSD1306

## Archivos
| Archivo | Descripción |
|---------|-------------|
| main.c | Inicialización y loop principal |
| wallet.c/h | Lógica de wallet |
| oled.c/h | Driver OLED SSD1306 |
| i2c.c/h | I2C por software |
| hardware_cfg.h | Configuración de pines |
| usb/ | Stack USB CDC |
