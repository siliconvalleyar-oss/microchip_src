# PIC18F4550 - Terminator PWM Controller

**Descripción:** Controlador PWM por software con 7 modos seleccionables mediante botones. El nombre "Terminator" hace referencia al característico sonido PWM de la película.

## Modos
| Modo | Descripción |
|------|-------------|
| 0 | PWM completo con barrido 0-100% |
| 1 | Parpadeo 1s on/1s off |
| 2 | 100% fijo |
| 3 | 75% fijo |
| 4 | 50% fijo |
| 5 | 25% fijo |
| 6 | PWM variable 5%-90% |

## Características
- RB4 = modo siguiente, RB5 = modo anterior
- PWM por software (ciclo de 100 pasos × 100µs)
- Salidas: RB1 + RD0
- Fosc = 20MHz
