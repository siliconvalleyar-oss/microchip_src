# PIC32MX795F512H - E-Paper Display Driver

**Descripción:** Driver base para pantallas E-Paper de Pervasive Displays usando PIC32MX795F512H (64 pines). Soporta múltiples tamaños de pantalla con frame buffers calculados automáticamente.

## Características
- Soporte para pantallas: 1.54", 2.13", 2.66", 2.71", 2.87", 3.70", 4.17", 4.37"
- SPI a 8MHz (SCK=RB14, SDO=RF5, SDI=RF4)
- Pines configurables via estructura pins_t
- Frame buffers calculados para cada tamaño
- Sistema a 80MHz (cristal HS 8MHz + PLL x20)
- Registros de inicialización para pantallas pequeñas/medianas

## Periféricos Utilizados
- SPI4 (maestro), GPIO, Timer
