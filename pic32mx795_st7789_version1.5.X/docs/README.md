# PIC32MX795 - Pac-Man Mejorado (v1.5)

**Descripción:** Versión mejorada del Pac-Man para display ST7789 (240×240) en PIC32MX795F512H. Incluye power pellets, fantasmas asustados, pantallas de título profesionales, niveles y más.

## Características
- **Power Pellets**: Puntos grandes en 4 esquinas que asustan a los fantasmas
- **Fantasmas asustados**: Cambian de color y huyen de Pac-Man (modo timer)
- **Pantalla de título profesional**: Título animado con Pac-Man a escala 3×
- **Pantalla Game Over**: Mensaje con puntuación y high score
- **Pantalla Level Clear**: Transición entre niveles
- **Niveles**: La velocidad aumenta con cada nivel (`level`)
- **High Score**: Récord de sesión persistente
- **Sprites escalados**: `draw_pacman_scaled()` y `draw_ghost_scaled()` para título
- **Paredes con estilo**: Celdas de pared con bordes direccionales (`draw_wall_cell`)
- **Popup de puntuación**: Animación al comer puntos
- **Círculo relleno**: `fill_circle()` para sprites más suaves
- **Sonido**: beep, eat, ghost, death, start

## Novedades v1.5 vs v1.4
| Característica | v1.4 | v1.5 |
|---------------|------|------|
| Power Pellets | ❌ | ✅ (4 esquinas) |
| Fantasmas asustados | ❌ | ✅ (timer + color azul) |
| Pantalla título | Texto simple | ✅ Profesional con sprites |
| Game Over | Texto simple | ✅ Con high score |
| Level Clear | ❌ | ✅ Transición animada |
| Niveles | ❌ | ✅ Velocidad progresiva |
| High Score | ❌ | ✅ Récord de sesión |
| Sprites escalados | ❌ | ✅ 1×, 3× y más |
| Paredes direccionales | ❌ | ✅ draw_wall_cell() |
| Popup puntuación | ❌ | ✅ Animación |
| Círculo relleno | ❌ | ✅ fill_circle() |

## Archivos del proyecto
| Archivo | Propósito |
|---------|-----------|
| `HardwareProfile.h` | Configuración de pines, sistema, colores |
| `Graphics.h` | Funciones gráficas con sprites escalados |
| `fonts.h` | Definiciones de fuentes |
| `GameEngine.h` | Motor de juego con niveles y high score |
| `Sound.h` / `Sound.cpp` | Sistema de sonido |
| `main.cpp` | Punto de entrada |

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
