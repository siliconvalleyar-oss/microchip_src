# PIC18F4550 - Hardware PWM (CCP1)

**Descripción:** Control PWM por hardware usando el módulo CCP1 del PIC18F4550. Genera un barrido de 0% a 100% con fade suave (dir=1 / dir=-1).

## Características
- PWM en RC2 (CCP1) con frecuencia de ~1.6kHz (PR2=124)
- Resolución de 10 bits (0-1023)
- Fade automático con dirección variable
- Fosc = 20MHz

## Periféricos Utilizados
- CCP1 (PWM)
- Timer2

## Pines
| Pin | Función |
|-----|---------|
| RC2 | PWM output (CCP1) |
