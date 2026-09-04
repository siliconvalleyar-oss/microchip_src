# PIC32MX795 - Pac-Man v1.4

**Descripción:** Primera versión del juego Pac-Man para display ST7789 (240×240) en PIC32MX795F512H. Laberinto 10×10, 4 fantasmas con IA básica, sonido y puntuación.

## Características
- Laberinto 10×10 celdas (24×24 píxeles cada una)
- Pac-Man con animación de boca (abierta/cerrada)
- 4 fantasmas con colores distintos (rojo, rosa, cyan, naranja)
- IA básica de fantasmas (persecución directa hacia Pac-Man)
- Puntos pequeños (10 pts) distribuidos por el laberinto
- HUD con vidas y puntuación
- Borrado inteligente: reconstruye celdas del laberinto al mover sprites
- Sonido: inicio, comer punto, muerte
- Pantalla de título "PACMAN / PIC32MX795"
- Movimiento automático para pruebas (`rand() % 4`)
- Frame rate ~25 FPS

## Novedades vs Space Shooter v1.3
- Juego completamente nuevo (Pac-Man en lugar de Space Shooter)
- Laberinto con paredes, puntos, sprites de 20×20
- 4 fantasmas con IA de persecución
- Sistema de sonido (`Sound.h`, `Sound.cpp`)
- Animación de boca de Pac-Man
- Borrado de sprites por reconstrucción de celdas

## Archivos del proyecto
| Archivo | Propósito |
|---------|-----------|
| `HardwareProfile.h` | Configuración de pines, sistema, colores |
| `Graphics.h` | Funciones gráficas con sprites de Pac-Man y fantasmas |
| `GameEngine.h` | Motor de juego (laberinto, Pac-Man, fantasmas, colisiones) |
| `GameEngine.cpp` | Implementación completa del motor de juego |
| `Sound.h` / `Sound.cpp` | Sistema de sonido básico |
| `main.cpp` | Punto de entrada, init display, sonido, game loop |

## Pines
| Pin | Señal |
|-----|-------|
| RB0 | BL (Backlight) |
| RB1 | DC (Data/Command) |
| RB2 | RST (Reset) |
| RB6 | SCK4 (SPI Clock) |
| RF5 | SDO4 (SPI MOSI) |
| RF4 | SDI4 (SPI MISO) |
| RB12 | Speaker (PWM sound) |
