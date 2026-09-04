# PIC18F45550 - External Interrupt + ADC + Button

**Descripción:** Demostración de interrupción externa (INT0), ADC y pulsador en PIC18F45550. Al detectar flanco de subida en RB0, genera un pulso de 20µs en RB1.

## Características
- INT0 (RB0) dispara pulso de 20µs en RB1
- ADC en RB4 (AN11) lee valor analógico
- Pulsador RB5 controla LED0
- ADC < 10 enciende LED3
- Fosc = 20MHz

## Periféricos Utilizados
- INT0 (interrupción externa)
- ADC (AN11)
- GPIO

## Pines
| Pin | Función |
|-----|---------|
| RB0 | INT0 entrada |
| RB1 | Pulso de salida |
| RB4 | ADC AN11 |
| RB5 | Pulsador entrada |
| RD0 | LED indicador |
| RD3 | LED indicador ADC |
