# Changelog

## v1.5 (current)
- **Pac-Man mejorado** sobre v1.4
- Power pellets en 4 esquinas que ponen a los fantasmas en modo asustado
- Fantasmas asustados: cuerpo azul, ojos en X, boca ondulada, huyen (timer 150 frames)
- Pantalla de título profesional (sprites escalados 3×, barra de progreso)
- Pantalla Game Over con high score y "NEW RECORD!"
- Pantalla Level Clear con transición animada, estrellas y bonus por nivel
- Niveles con velocidad progresiva (`pacSpeed`/`ghostSpeed` por `level`)
- High Score de sesión (H:)
- Sprites escalados: `draw_pacman_scaled()`, `draw_ghost_scaled()`
- Paredes con bordes direccionales (`draw_wall_cell`)
- Popup de puntuación (`draw_score_popup`)
- Círculo relleno `fill_circle()` por scanlines (sin flicker)
- Laberinto reducido a 9×9
- Rendimiento de pantallas optimizado (menos frames de animación)

## v1.4
- Cambio de Space Shooter a Pac-Man
- Laberinto 10×10 celdas 24×24
- 4 fantasmas con IA de persecución (rojo, rosa, cyan, naranja)
- Pac-Man con animación de boca
- Borrado de sprites por reconstrucción de celdas
- Sistema de sonido (`Sound.h`, `Sound.cpp`)
- Pantalla de título "PACMAN / PIC32MX795"

## v1.3
- Refactor en módulos: `HardwareProfile.h`, `Graphics.h`, `GameEngine.h`
- Namespace `GameEngine` con `init()`/`gameLoop()`
- `PhysicsComponents.h` para sistema de físicas
- Código más mantenible y extensible

## v1.2
- 5 tipos de enemigos nave con colores distintos (rojo, verde, azul, naranja, púrpura)
- `draw_ship_palette()` — renderiza la nave con cualquier paleta
- Movimiento y disparo automático (pruebas sin botones)
- Selector `#define USE_BUTTONS`

## v1.1
- Space Shooter avanzado
- Refactor en namespaces `Graphics::` y `Game::`
- Sprite de nave 16×24 detallado
- Frames de invencibilidad con parpadeo
- Hasta 8 enemigos y 5 balas simultáneas

## v1.0
- Driver ST7789 base vía SPI4 (init, píxeles, rectángulos, líneas, círculos, texto)
- Fuente 5×7 con 95 caracteres imprimibles
- Juego Space Shooter básico como demostración
- PLL 20× → 80MHz, RPP para remappable pins
