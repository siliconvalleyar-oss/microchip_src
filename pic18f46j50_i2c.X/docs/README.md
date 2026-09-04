# PIC18F46J50 - I2C OLED with Buttons

**Descripción:** Control de display OLED SSD1306 vía I2C por software con entrada de botones. Muestra texto y contador en OLED, controlado por pulsadores RB4 y RB5.

## Características
- I2C por software (bit-banging)
- OLED SSD1306 128×32px
- 2 botones: SW1 (incrementa LED), SW2 (decrementa LED)
- LEDs en RD0-RD3 (1 de 4 activo)
- Muestra contador en OLED
- Fosc = 4MHz

## Periféricos Utilizados
- GPIO, I2C (bit-bang), OLED SSD1306

## Archivos
| Archivo | Descripción |
|---------|-------------|
| main.c | Lógica principal |
| hardware_cfg.h | Configuración de pines |
| i2c.c/h | I2C bit-bang |
| oled.c/h | Driver OLED |
| fonts.h | Definiciones de fuentes |
