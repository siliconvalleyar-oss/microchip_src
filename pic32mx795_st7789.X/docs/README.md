# PIC32MX795 - ST7789 Pac-Man / Space Shooter (v1.5)

**Descripción:** Proyecto para display ST7789 (240×240) vía SPI4 en el PIC32MX795F512H. Comenzó como un driver gráfico base con un Space Shooter como demo (v1.0-v1.3), evolucionó a Pac-Man (v1.4) y culminó en un Pac-Man mejorado con power pellets, fantasmas asustados y pantallas profesionales (v1.5).

## Características (v1.5)
- **Power Pellets**: puntos grandes en 4 esquinas que ponen a los fantasmas en modo asustado
- **Fantasmas asustados**: cuerpo azul, ojos en X y boca ondulada; huyen de Pac-Man (timer de 150 frames)
- **Pantalla de título profesional**: título escalado 3×, pacman 3×, 4 fantasmas y barra de progreso
- **Pantalla Game Over**: mensaje descendente, parpadeo de puntuación, "NEW RECORD!" y prompt de reset
- **Pantalla Level Clear**: parpadeo, estrellas, texto con rebote y bonus por nivel
- **Niveles**: la velocidad de Pac-Man y fantasmas aumenta con el nivel (`level`)
- **High Score**: récord de sesión (H:) persistente mientras esté encendido
- **Sprites escalados**: `draw_pacman_scaled()` y `draw_ghost_scaled()` (1×, 3×)
- **Anti-flicker**: borrado del bounding box de una vez (1 transacción SPI) antes de redibujar
- **Paredes direccionales**: `draw_wall_cell()` con bordes biselados según vecinos
- **Popup de puntuación**: `draw_score_popup()` al comer un fantasma asustado
- **Círculo relleno**: `fill_circle()` por scanlines (sin parpadeo)
- **Sonido**: beep, eat, power pellet, ghost, death, start, level up, extra life

## Historia de evolución (aprendido de las versiones en tmp/)
| Versión | Contenido |
|---------|-----------|
| v1.0 | Driver ST7789 base + Space Shooter con botones (RB7/RB8/RB9) |
| v1.1 | Space Shooter avanzado: namespaces `Graphics`/`Game`, sprite nave 16×24, invencibilidad |
| v1.2 | Enemigos tipo nave con 5 colores, `draw_ship_palette()`, movimiento/disparo automático |
| v1.3 | Refactor en módulos: `HardwareProfile`, `Graphics`, `GameEngine`, físicas |
| v1.4 | Cambio a Pac-Man: laberinto 10×10, 4 fantasmas con IA, sonido |
| v1.5 | Pac-Man mejorado: power pellets, fantasmas asustados, pantallas profesionales, niveles |

## Archivos del proyecto
| Archivo | Propósito |
|---------|-----------|
| `HardwareProfile.h` | Pines, configuración del sistema, colores, y dimensiones del laberinto (9×9) |
| `Graphics.h` / `Graphics.cpp` | Primitivas gráficas, sprites y dibujo del laberinto |
| `GameEngine.h` / `GameEngine.cpp` | Motor de juego completo |
| `Sound.h` / `Sound.cpp` | Sistema de sonido PWM |
| `main.cpp` | Punto de entrada |
| `docs/` | Documentación técnica (este repo) |

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

## Nota sobre código actual
El archivo `GameEngine.cpp` (v1.5) contiene la lógica completa de actualización de Pac-Man y fantasmas (`update_pacman`, `update_ghosts`, `check_collisions`), el HUD, y las pantallas profesional/título/level-clear/game-over. El bucle `game_loop()` quedó en estado intermedio de refactor (termina en "Resto del código..."): las pantallas y el resto de módulos están completos, pero hay que reensamblar el bucle principal para integrar `update_pacman()`/`update_ghosts()`/`check_collisions()` en el flujo definitivo.
