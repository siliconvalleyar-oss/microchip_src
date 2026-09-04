# PIC32MX795 - ST7789 Driver Base + Space Shooter (v1.0)

**Descripción:** Primera versión del driver para display ST7789 (240×240) vía SPI4 en el PIC32MX795F512H. Incluye un juego Space Shooter básico como demostración.

## Características
- Driver completo ST7789 (init, píxeles, rectángulos, líneas, círculos, texto)
- Fuente 5×7 con 95 caracteres imprimibles
- Juego Space Shooter con nave controlada por botones (RB7=izquierda, RB8=derecha, RB9=disparo)
- Enemigos que caen aleatoriamente
- Proyectiles, colisiones, vidas y puntuación
- Animación de explosión (círculos concéntricos)
- HUD con vidas y puntuación
- Presentación opcional (`#define PRESENTACION`)

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

## Periféricos
- **SPI4**: Modo 3, Master, 80MHz
- **Timer systick**: Core Timer (`_CP0_GET_COUNT`) para delays
- **PLL**: 20× multiplicador, 2× predividor → 80MHz
- **Remappable pins**: RP6=SCK4, RP5=SDO4, RP4=SDI4
