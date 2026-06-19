# PIC32MX795F512H - E-Paper Display v1.2 (Pin Test)

**Descripción:** Segunda versión del driver para pantallas E-Paper Pervasive Displays. Mantiene las mismas funciones de test de pines que v1.1 con mejoras incrementales en el driver EPD.

## Características
- Mismas funciones test_all_pins, test_pin_output, test_pin_input que v1.1
- Prueba de pines: RB0-RB3, RB13-RB14, RF4-RF5
- Driver EPD con configuración mejorada
- Sistema a 80MHz

## Periféricos Utilizados
- SPI4, GPIO

## Archivos
| Archivo | Descripción |
|---------|-------------|
| test.c | Funciones de test |
| EPD_Driver.h/c | Driver EPD |
| EPD_Graphics.h | Primitivas gráficas |
| EPD_Configuration.h | Configuración |
