# Deployment

## Producción / Programación

El firmware se compila en MPLAB X y se graba en el PIC32MX795F512H.

### Pasos
1. Compilar el proyecto en MPLAB X (Clean and Build).
2. Generar el `.hex` (normalmente en `dist/<config>/production/`).
3. Abrir MPLAB IPE, seleccionar dispositivo **PIC32MX795F512H**.
4. Conectar el programador (Pickit/ICD) al header ICSP del PCB.
5. Programar y verificar checksum.

### Conexión ICSP (típica)
- MCLR
- PGD (RB7 / SDATA)
- PGC (RB6 / SCLK)
- VDD (3.3V)
- VSS (GND)

## Dependencia externa
- El proyecto incluye `HardwareProfile.h` con `#include "../../ST7789_common/st7789_driver.h"`, que reside fuera de la carpeta del proyecto (directorio compartido `ST7789_common`). Asegúrate de que la ruta relativa esté disponible en el árbol de MPLAB X al compilar.

## Notas de hardware
- MCU: **PIC32MX795F512H** (3.3V)
- Display **ST7789** de 240×240, vía **SPI4**
- Speaker PWM en **RB12**
- Botones opcionales (compilar con `-DUSE_BUTTONS`)
  - RB7=izquierda, RB8=derecha, RB9=arriba, RB10=abajo

## Reset / reinicio
- El mensaje "PRESS RESET" en Game Over indica que para reiniciar se presiona el reset físico del microcontrolador.
