# Installation

## Prerequisitos
- MPLAB X IDE (con soporte para PIC32MX)
- Compilador XC32
- Programador Pickit/ICD compatible
- PIC32MX795F512H con display ST7789 (240×240) y speaker PWM

## Compilar

### Vía MPLAB X
1. Abrir MPLAB X IDE.
2. File → Open Project → seleccionar la carpeta `pic32mx795_st7789.X`.
3. Seleccionar configuración y PIC32MX795F512H.
4. Clean and Build.

### Nota sobre la ruta del driver
`HardwareProfile.h` incluye `../../ST7789_common/st7789_driver.h` (driver común fuera del proyecto). Si el árbol de MPLAB no lo encuentra, agrega `ST7789_common` a las rutas de include o copia el driver al proyecto.

## Programar

### Vía MPLAB IPE
1. Abrir MPLAB IPE.
2. Dispositivo: PIC32MX795F512H.
3. Herramienta: Pickit/ICD.
4. Cargar el `.hex` de `dist/<config>/production/`.
5. Programar.

## Primer arranque
1. Alimentar la placa (3.3V).
2. El firmware hace SPI init, enciende el backlight y muestra la pantalla de título.
3. Inicia la partida automáticamente (Pac-Man).
