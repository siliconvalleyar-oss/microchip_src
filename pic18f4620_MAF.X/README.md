# PIC18F4620 - MAF Signal Regenerator

**Descripción:** Regenerador de señal con frecuencia ajustable. Usa comparador C1 para convertir señal analógica a cuadrada, captura el periodo con CCP2 y replica la frecuencia en RC2 con CCP1 toggle. Incluye salida de frecuencia variable en RB3 controlada por pulsador.

## Características
- Captura de periodo con CCP2 en RC1
- Replicación de frecuencia en RC2 (CCP1 toggle, 100Hz-20kHz)
- Frecuencia variable en RB3: 2.3kHz-4kHz, paso 150Hz
- Pulsador RA5 para cambiar frecuencia RB3
- LED RB5 indica señal presente
- Timeout 500ms para pérdida de señal
- Fosc = 20MHz

## Periféricos Utilizados
- CCP1 (compare/toggle), CCP2 (capture)
- Timer0, Timer1
- Comparador C1
- GPIO

## Pines
| Pin | Función |
|-----|---------|
| RA2 | C1IN+ (señal entrada) |
| RA3 | C1IN- (ref 2.5V) |
| RA4 | C1OUT |
| RA5 | Pulsador (activo bajo) |
| RC1 | CCP2 capture |
| RC2 | CCP1 señal replicada |
| RB3 | Frecuencia variable |
| RB5 | LED indicador |
