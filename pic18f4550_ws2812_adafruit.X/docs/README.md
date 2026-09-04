# PIC18F4550 - WS2812 RGB LED Controller

**Descripción:** Controlador de tira LED RGB WS2812 (NeoPixel) con temporización por NOPs. Implementa la función `wheel()` para generar colores del arcoíris.

## Características
- 16 LEDs WS2812 direccionables
- Temporización precisa con NOPs (Fosc=20MHz)
- Función wheel() para colores HSV
- Modo actual: todos los LEDs en azul (intensity=90)
- Código comentado para animación arcoíris

## Periféricos Utilizados
- GPIO con temporización precisa

## Pines
| Pin | Función |
|-----|---------|
| RB0 | WS2812 data out |
