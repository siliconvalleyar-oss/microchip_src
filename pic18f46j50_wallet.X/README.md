# PIC18F46J50 - USB CDC Device (LED Control)

**Descripción:** Dispositivo USB CDC (Virtual Serial Port) que recibe comandos por USB para controlar LEDs. Implementa el stack USB MLA de Microchip con comunicación clase CDC.

## Características
- USB CDC (Virtual COM Port) a 48MHz con PLL
- Comandos: LED0_ON, LED0_OFF, LED1_ON, LED1_OFF, HELP
- 2 botones físicos que alternan LEDs
- PLL desde cristal de 12MHz
- Envía banner de inicio al conectar

## Periféricos Utilizados
- USB (CDC), GPIO, Timer

## Archivos
| Archivo | Descripción |
|---------|-------------|
| main.c | Aplicación principal |
| usb_cdc.c/h | Capa de abstracción USB CDC |
| usb_descriptors.c | Descriptores USB |
| hal/ | Capa HAL (LED, botón, I2C, SPI, SD) |
| usb_mla/ | Stack USB MLA de Microchip |
