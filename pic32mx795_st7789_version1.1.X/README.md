# PIC32MX795 - Space Shooter Avanzado (v1.1)

**Descripción:** Segunda versión del Space Shooter para display ST7789 (240×240) vía SPI4. Mejora el juego original con enemigos más variados y mecánicas refinadas.

## Características
- Enemigos que caen con velocidades variables (1-3 píxeles/frame)
- Proyectiles del jugador (máximo 5 simultáneos)
- Colisiones bala-enemigo y nave-enemigo
- 3 vidas con frames de invencibilidad (60 frames)
- Puntuación (10 puntos por enemigo)
- Control por botones (RB7=izquierda, RB8=derecha, RB9=disparo)
- Optimización: la nave solo se redibuja si se mueve
- Namespace `Graphics` para funciones gráficas
- Namespace `Game` para lógica del juego
- Memoria estática (sin heap)

## Novedades v1.1 vs v1.0
- Refactorización en namespaces (`Graphics::`, `Game::`)
- Sprite de nave más detallado (16×24 píxeles)
- Efecto de parpadeo durante invencibilidad
- Máxima de 8 enemigos y 5 balas simultáneas

## Pines
| Pin | Señal |
|-----|-------|
| RB0 | BL (Backlight) |
| RB1 | DC (Data/Command) |
| RB2 | RST (Reset) |
| RB6 | SCK4 (SPI Clock) |
| RF5 | SDO4 (SPI MOSI) |
| RF4 | SDI4 (SPI MISO) |
| RB7 | Botón izquierda |
| RB8 | Botón derecha |
| RB9 | Botón disparo |
