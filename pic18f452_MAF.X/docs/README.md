# PIC18F452 - Frequency Meter & Signal Regenerator

**Descripción:** Mide frecuencia de señal externa usando CCP2 (captura en RC1) y la regenera en RB0 con Timer2 toggle. Incluye salida fija 1kHz en RB1 y detección de pérdida de señal. Usa comparador analógico para convertir señal senoidal a cuadrada.

## Características
- Captura de periodo con Timer1 + CCP2
- Regeneración en RB0 (100Hz-20kHz)
- Salida fija 1kHz en RB1
- LED RB5 indica señal OK
- Timeout 500ms para pérdida de señal
- Fosc = 20MHz, tick Timer1 = 0.2µs

## Periféricos Utilizados
- CCP2 (captura), CCP1 (compare/toggle)
- Timer0, Timer1, Timer2
- Comparador C1
- GPIO

## Pines
| Pin | Función |
|-----|---------|
| RA2 | C1IN+ (señal entrada) |
| RA3 | C1IN- (ref 2.5V) |
| RA4 | C1OUT → RC1 (CCP2) |
| RC1 | CCP2 capture input |
| RB0 | Señal regenerada |
| RB1 | 1kHz fijo |
| RB5 | LED indicador |
