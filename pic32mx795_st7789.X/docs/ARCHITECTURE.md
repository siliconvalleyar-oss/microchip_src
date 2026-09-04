# Architecture

## Overview

Juego para display ST7789 (240×240) en PIC32MX795F512H. Se comunica por SPI4 a 80MHz y usa el Core Timer para temporización. Todo el dibujo es por "dirty rectangle": se borra el bounding box anterior en una sola transacción SPI y se redibuja encima (anti-flicker).

## Ejecución

```
Power ON
  └─ Configurar pines display (BL/DC/RST) como salida
  └─ spi4_init()
  └─ init_display()
  └─ lcg_srand(_CP0_GET_COUNT())  → semilla para rand()
  └─ sound_init()
  └─ GameEngine::game_loop()
       ├─ save_template()          → copia laberinto inicial a plantilla
       ├─ show_title_professional()
       ├─ init_game()              → restaura laberinto desde plantilla, reubica sprites
       ├─ draw_maze()
       ├─ draw_status()
       └─ (bucle de juego — ver nota)
```

## Módulos

- **HardwareProfile.h**: pines del display y del speaker (RB12), colores `COLOR565`, y dimensiones del juego: laberinto `MAZE_COLS × MAZE_ROWS` = 9×9, celdas 24×24, offsets `MAZE_OX/MAZE_OY` = 12, tamaños de sprites (`PACMAN_SIZE`/`GHOST_SIZE` = 18).
- **Graphics**: envuelve el driver `st7789_driver.h` (`fill_screen`, `fill_rect`, `draw_pixel`, `draw_hline/vline`, `draw_circle`, `draw_char/string`) y dibuja los sprites de juego y el laberinto.
- **GameEngine**: estado global (laberinto, posiciones de sprites, puntuación, vidas, nivel) y lógica (`update_pacman`, `update_ghosts`, `check_collisions`, `draw_status`, pantallas).
- **Sound**: generación de tonos PWM con `delay_us()` (bloqueante).

## Laberinto 9×9

Cada celda puede ser:
- `0` = pasillo vacío
- `1` = pared
- `2` = punto normal (10 pts / 50 pts)
- `3` = power pellet (asusta a los fantasmas)

`draw_wall_cell()` recibe las flags `top/bot/lft/rgt` (si el vecino adyacente también es pared) para dibujar bordes biselados solo en las transiciones pared→pasillo.

## Sistema de movimiento (anti-flicker)

1. Guardar la posición previa del sprite.
2. Verificar colisión con paredes en las 4 esquinas del sprite (`canMovePac`).
3. Mover sprite / elegir dirección del fantasma.
4. `redraw_cells_in_rect(previa)` → reconstruye las celdas del laberinto solapadas con la posición previa.
5. `Graphics::draw_pacman/draw_ghost` → borra el bounding box de una vez y dibuja.

## Fantasmas

- 4 fantasmas: rojo, rosa, cyan, naranja `(RED, PINK, CYAN, ORANGE)`.
- IA: persecución directa hacia Pac-Man, con giro aleatorio ocasional.
- Modo asustado: `ghostFrightened[i]=true` y `frightenTimer=150` frames; cuerpo azul, ojos en X, huyen de Pac-Man y van más lentos.
- Comer un fantasma asustado: +200 pts y efecto `draw_score_popup`.

## Colisiones

- `check_collisions()` compara cada fantasma con Pac-Man (distancia < `PACMAN_SIZE-4`).
- Fantasma asustado → se come (+200, reaparición en centro).
- Fantasma normal → pierde una vida, reposiciona todo, y a 0 vidas marca `gameOver`.

## HUD

- `draw_status()` muestra: mini pac-mans (una por vida, máx 5), `S:<score> H:<highscore> L<level>`.
- Solo redibuja si cambian score/vidas/nivel (cache `ls/ll/llv`).

## Sonido (PWM en RB12)

- `sound_beep(freq_hz, duration_ms)` genera onda cuadrada bloqueante con `delay_us`.
- Melodías: `sound_start`, `sound_eat` (frecuencia creciente), `sound_power_pellet`, `sound_ghost` (descendente), `sound_death` (descendente largo), `sound_level_up`, `sound_extra_life`.

## Nota: bucle de juego

`game_loop()` en `GameEngine.cpp` quedó en estado intermedio de refactor: configura el estado inicial y dibuja el laberinto, pero termina con el comentario *"Resto del código..."* antes de integrar `update_pacman()`/`update_ghosts()`/`check_collisions()` en un `for(;;)` de frames. El resto de los módulos (gráficos, lógica, sonido, pantallas) está completo.
