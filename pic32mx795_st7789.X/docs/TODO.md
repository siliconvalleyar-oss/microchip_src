# TODO

## Alta prioridad
- [ ] Completar `game_loop()` de `GameEngine.cpp`: integrar `update_pacman()`, `update_ghosts()` y `check_collisions()` en el bucle de frames (termina en "Resto del código...").
- [ ] Verificar el comportamiento del high score en modo reinicio (el `game_loop()` actual resetea `highScore=0` al iniciar).
- [ ] Validar el flag `USE_BUTTONS` si se desea control manual (actualmente el juego es automático).

## Media prioridad
- [ ] Revisar los valores de `delay_us()`/`delay_ms()` en `Sound.cpp` para el reloj real (Core Timer).
- [ ] Añadir `fonts.h` al proyecto si `draw_char`/`draw_string` dependen de `font5x7` externo.
- [ ] Confirmar framework de frames per second (condiciones de sincronización del bucle de juego).

## Baja prioridad
- [ ] Añadir diagrama de pines al apartado de hardware de esta documentación.
- [ ] Expandir a laberintos más grandes o niveles con layout distinto.
- [ ] Documentar la evolución completa driver→shooter→pacman en un histórico más detallado.

## Problemas conocidos
- `game_loop()` incompleto: las pantallas y módulos están terminados, pero el bucle de juego principal queda pendiente de reensamblar.
- `highScore` se resetea a 0 en `game_loop()`; decidir si debe persistir entre reinicios.
- El proyecto depende de `ST7789_common/st7789_driver.h` fuera de la carpeta (`HardwareProfile.h`).
