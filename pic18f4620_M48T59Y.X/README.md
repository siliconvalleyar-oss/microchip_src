# PIC18F4620 - MAF Signal Regenerator

**Descripción:** Regenerador de señal con frecuencia ajustable. Usa comparador C1 para convertir señal analógica a cuadrada, captura el periodo con CCP2 y replica la frecuencia en RC2 con CCP1 toggle. Incluye salida de frecuencia variable en RB3 controlada por pulsador y memoria no volátil M48T559Y.

## Características
- Captura de periodo con CCP2 en RC1
- Replicación de frecuencia en RC2 (CCP1 toggle, 100Hz-20kHz)
- Frecuencia variable en RB3: 2.3kHz-4kHz, paso 150Hz
- Pulsador RA5 para cambiar frecuencia RB3
- LED RB5 indica señal presente
- Timeout 500ms para pérdida de señal
- SRAM no volátil M48T559Y (8KB con RTC)
- Frecuencia RB3 persistente en SRAM del M48T559Y
- Fosc = 20MHz

## Periféricos Utilizados
- CCP1 (compare/toggle), CCP2 (capture)
- Timer0, Timer1
- Comparador C1
- GPIO
- M48T559Y Timekeeper SRAM (interfaz multiplexada)

## Pines
| Pin | Función |
|-----|---------|
| RA2 | C1IN+ (señal entrada) |
| RA3 | C1IN- (ref 2.5V) |
| RA4 | C1OUT |
| RA5 | Pulsador (activo bajo) |
| RA6 | AS0 (M48T559Y Address Strobe 0) |
| RA7 | AS1 (M48T559Y Address Strobe 1) |
| RC0 | E (M48T559Y Chip Enable) |
| RC1 | CCP2 capture |
| RC2 | CCP1 señal replicada |
| RC3 | R (M48T559Y Read Enable) |
| RC4 | W (M48T559Y Write Enable) |
| RD0-RD7 | AD0-AD7 (M48T559Y Address/Data multiplexed) |
| RB3 | Frecuencia variable |
| RB5 | LED indicador |

## Archivos
- `main.c` - Aplicación principal
- `m48t59y.c` - Driver M48T559Y
- `m48t59y.h` - API pública del driver
- `docs/M48T559Y.md` - Documentación del datasheet

## Requisitos de Hardware M48T559Y
- Resistencia pull-up de 1kΩ en pin RST (salida open-drain)
- Resistencia pull-up de 500Ω-10kΩ en pin IRQ/FT
- Capacitor de desacople 0.1µF cerca de VCC-VSS
- Diodo Schottky VCC a VSS para protección (1N5817 o MBRS120T3)
- SNAPHAT con batería de litio y cristal 32.768kHz
