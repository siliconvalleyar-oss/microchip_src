# PIC32MX795 - Space Shooter Refactorizado (v1.3)

**Descripción:** Cuarta versión del Space Shooter, refactorizada con separación en módulos (HardwareProfile, Graphics, GameEngine). Arquitectura más limpia y preparada para escalar.

## Características
- **Arquitectura modular**: `HardwareProfile.h`, `Graphics.h`, `GameEngine.h`
- Motor de juego separado de la capa gráfica
- `GameEngine` namespace con init/gameLoop
- Sistema de físicas (`PhysicsComponents.h`)
- Sprite de nave 16×24 completo
- Control por botones o automático (`#define USE_BUTTONS`)

## Novedades v1.3 vs v1.2
- Código fuente dividido en headers separados
- `GameEngine::init()` y `GameEngine::gameLoop()` como API pública
- `PhysicsComponents.h` para sistema de físicas
- Código más mantenible y extensible

## Archivos del proyecto
| Archivo | Propósito |
|---------|-----------|
| `HardwareProfile.h` | Configuración de pines, sistema, colores, delays |
| `Graphics.h` | Funciones gráficas (píxeles, rect, círculos, texto, sprites) |
| `GameEngine.h` | Motor de juego (init, update, collisions, loop) |
| `PhysicsComponents.h` | Componentes físicos (movimiento, colisiones) |
| `main.cpp` | Punto de entrada, init display, GameEngine |

## Pines
| Pin | Señal |
|-----|-------|
| RB0 | BL (Backlight) |
| RB1 | DC (Data/Command) |
| RB2 | RST (Reset) |
| RB6 | SCK4 (SPI Clock) |
| RF5 | SDO4 (SPI MOSI) |
| RF4 | SDI4 (SPI MISO) |
