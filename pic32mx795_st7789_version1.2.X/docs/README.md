# PIC32MX795 - Space Shooter con Enemigos Coloreados (v1.2)

**Descripción:** Tercera versión del Space Shooter para display ST7789 (240×240). Introduce múltiples tipos de enemigos con forma de nave y colores distintos, movimiento automático y paleta de colores configurable.

## Características
- **5 tipos de enemigos**: Rojo, Verde, Azul, Naranja y Púrpura
- Enemigos con forma de nave (misma silueta que el jugador) y paleta de colores
- `draw_ship_palette()` — función genérica que renderiza la nave con cualquier paleta
- Movimiento automático de la nave (oscilación izquierda-derecha)
- Disparo automático cada ~20 frames
- Modo botones opcional (`#define USE_BUTTONS`)
- Optimización de redibujado (solo lo que cambia)
- Presentación opcional

## Novedades v1.2 vs v1.1
- Enemigos con forma de nave (antes rectángulos simples)
- Múltiples tipos/colores de enemigos seleccionados aleatoriamente
- `draw_ship_palette()` con 6 colores parametrizables
- Movimiento y disparo automático para pruebas sin botones
- Selector `#define USE_BUTTONS` para alternar control

## Pines
| Pin | Señal |
|-----|-------|
| RB0 | BL (Backlight) |
| RB1 | DC (Data/Command) |
| RB2 | RST (Reset) |
| RB6 | SCK4 (SPI Clock) |
| RF5 | SDO4 (SPI MOSI) |
| RF4 | SDI4 (SPI MISO) |
| RB7-RB9 | Botones (opcional) |
