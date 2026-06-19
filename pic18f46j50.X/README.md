# PIC18F46J50 - OLED Display + EEPROM

**Descripción:** Demostración de display OLED con interfaz I2C y almacenamiento en EEPROM externa usando PIC18F46J50. Muestra logo, escribe y lee texto de EEPROM, y anima LEDs en RD0-RD3.

## Características
- I2C para OLED SSD1306
- I2C para EEPROM externa
- Escritura/lectura de string en EEPROM
- Display de logo bitmap
- LEDs animados en RD0-RD3
- Fosc = 4MHz

## Periféricos Utilizados
- MSSP (I2C), GPIO, EEPROM externa

## Archivos
| Archivo | Descripción |
|---------|-------------|
| main_p18f46j50.c | Programa principal |
| main_p18f4550_oled.c | Versión alternativa para PIC18F4550 |
| oled.c/h | Driver OLED SSD1306 |
| fonts.c/h | Fuentes bitmap |
| image.c/h | Imágenes bitmap |
| sdcard.c/h | SD card (WIP) |
| spi.c/h | SPI |
| eprom.c/h | EEPROM externa |
