# SKILL - Frecuenciómetro + Regenerador de Señal

## Microcontrolador
PIC18F452

## Descripción
Mide frecuencia de señal externa en RC1 (CCP2 capture) usando comparador C1 para cuadrar la onda, y regenera la misma frecuencia en RB0 mediante Timer2. Genera 1 kHz fijo en RB1. LED en RB5 indica presencia de señal.

## Periféricos Clave
CCP2 (capture), CCP1 (compare/toggle), Timer0, Timer1, Timer2, Comparador C1

## Aprendizajes
- Captura de frecuencia con CCP módulo
- Regeneración de señal con Timer2 + ISR
- Comparador analógico para condicionamiento de señal
- Timeout por software para detección de pérdida de señal
- ISR única sin prioridades

## Archivos
main.c
