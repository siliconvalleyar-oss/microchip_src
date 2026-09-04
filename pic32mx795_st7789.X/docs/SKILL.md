# SKILL: ST7789 Games - PIC32MX795

## Descripción
Conjunto de juegos (Space Shooter → Pac-Man) para display ST7789 (240×240) vía SPI4 en PIC32MX795F512H. Incluye driver gráfico, sprites escalados, motor de juego modular y sonido PWM.

## Configuración de Hardware
- **MCU:** PIC32MX795F512H (3.3V)
- **Display:** ST7789 240×240
- **Speaker:** PWM en RB12
- **PLL:** 20× multiplicador → 80MHz

## Pin Mapping
| Función | Pin |
|---------|-----|
| BL (Backlight) | RB0 |
| DC (Data/Command) | RB1 |
| RST (Reset) | RB2 |
| SCK4 (SPI Clock) | RB6 |
| SDO4 (SPI MOSI) | RF5 |
| SDI4 (SPI MISO) | RF4 |
| Speaker | RB12 |
| Botón izquierda | RB7 (opcional) |
| Botón derecha | RB8 (opcional) |
| Botón arriba | RB9 (opcional) |
| Botón abajo | RB10 (opcional) |

## Periféricos
- **SPI4**: Modo 3, Master, 80MHz, remappable (RP6=SCK4, RP5=SDO4, RP4=SDI4)
- **Core Timer**: `_CP0_GET_COUNT()` para delays y `lcg_srand()` como semilla
- **PWM**: speaker generado por software (onda cuadrada con `delay_us`)
- **Memory**: estática, sin heap

## Cómo dibujar sprites (anti-flicker)
1. Borrar el bounding box completo con `fill_rect` (una sola transacción SPI).
2. Dibujar el cuerpo (círculo relleno por scanlines para Pac-Man; cúpula + cuerpo + faldón dentado para ghost).
3. Dibujar la boca / ojos encima.

## Cómo reconstruir el fondo al mover sprites
- `redraw_cells_in_rect(x,y,w,h)`: recorre las celdas del laberinto solapadas con el rectángulo y las redibuja según su valor (pared/punto/power) para no dejar rastro.

## Cómo añadir un power pellet
- Marcar la celda con `3` en el laberinto.
- Al comerlo (celda `==3`): `score+=50`, `frightenTimer=150`, `ghostFrightened[i]=true` para los 4 fantasmas, `sound_power_pellet()`.

## Trucos de colores
- `COLOR565(r,g,b)` convierte RGB565.
- Fantasma asustado: `COLOR565(0,0,200)` azul.
- Paredes `WALL_COLOR=COLOR565(0,0,140)` con borde `WALL_EDGE=COLOR565(30,80,255)`.
- Punto `DOT_COLOR=COLOR565(255,220,100)`, power pellet blanco.

## Sonido
- `sound_beep(freq_hz, duration_ms)`: onda cuadrada bloqueante.
- Melodías: `start`, `eat`, `power_pellet`, `ghost` (descendente), `death`, `level_up`, `extra_life`.

## Notas
- El bucle principal `game_loop()` está en estado intermedio de refactor (ver docs/ARCHITECTURE.md).
- Dependencia externa: `ST7789_common/st7789_driver.h`.
