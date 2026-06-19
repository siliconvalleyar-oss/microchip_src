# PIC18F4620 - SSD1306 OLED Display (I2C Software)

**Descripción:** Controlador para display OLED SSD1306 de 128×32 píxeles usando I2C por software (bit-banging). Muestra texto en las 4 líneas del display.

## Características
- I2C bit-bang por software (pines configurables en i2c_sw.h)
- SSD1306 128×32 píxeles
- Funciones: PutStr, FillScreen, Update, Clear
- Fosc = 4MHz

## Periféricos Utilizados
- GPIO (I2C bit-bang)

## Archivos
| Archivo | Descripción |
|---------|-------------|
| main.c | Inicialización y texto de prueba |
| i2c_sw.c/h | I2C por software |
| oled.c/h | Driver SSD1306 |
| config.h | Configuración de pines |
