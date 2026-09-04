# PIC18F4620 - MAF Signal Regenerator + M48T559Y RTC

**Descripción:** Regenerador de señal con frecuencia ajustable. Usa comparador C1 para convertir señal analógica a cuadrada, captura el periodo con CCP2 y replica la frecuencia en RC2 con CCP1 toggle. Incluye salida de frecuencia variable en RB3 controlada por pulsador, memoria no volátil M48T559Y y RTC con API DateTime.

## Características
- Captura de periodo con CCP2 en RC1
- Replicación de frecuencia en RC2 (CCP1 toggle, 100Hz-20kHz)
- Frecuencia variable en RB3: 2.3kHz-4kHz, paso 150Hz
- Pulsador RA5 para cambiar frecuencia RB3
- LED RB5 indica señal presente
- Timeout 500ms para pérdida de señal
- SRAM no volátil M48T559Y (8KB con RTC)
- Frecuencia RB3 persistente en SRAM del M48T559Y
- RTC con API de alto nivel (struct TK_DateTime)
- Conversión BCD automática
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
| Archivo | Descripción |
|---------|-------------|
| `main.c` | Aplicación principal: regenerador de señal + RTC |
| `m48t59y.c` | Driver M48T559Y (bus multiplexado, RTC, SRAM) |
| `m48t59y.h` | API pública del driver |
| `docs/SKILL.md` | Documentación técnica del driver |
| `docs/M48T559Y.md` | Datasheet y especificaciones |
| `README.md` | Este archivo |

## API RTC

```c
// Estructura de fecha/hora
typedef struct {
    uint8_t year;    // 00-99
    uint8_t month;   // 01-12
    uint8_t date;    // 01-31
    uint8_t day;     // 01-07 (día de la semana)
    uint8_t hours;   // 00-23
    uint8_t minutes; // 00-59
    uint8_t seconds; // 00-59
} TK_DateTime;

// Leer fecha/hora (conversión BCD automática)
TK_DateTime dt;
m48txx_get_datetime(&dt);

// Escribir fecha/hora
m48txx_set_datetime(&dt);
```

## Requisitos de Hardware M48T559Y
- Resistencia pull-up de 1kΩ en pin RST (salida open-drain)
- Resistencia pull-up de 500Ω-10kΩ en pin IRQ/FT
- Capacitor de desacople 0.1µF cerca de VCC-VSS
- Diodo Schottky VCC a VSS para protección (1N5817 o MBRS120T3)
- SNAPHAT con batería de litio y cristal 32.768kHz

## Funcionamiento

1. Al encender, el PIC18F4620 inicializa el M48T559Y
2. El RTC se inicia automáticamente (oscilador 32.768 kHz)
3. La frecuencia de RB3 se carga desde SRAM del M48T559Y
4. Al cambiar frecuencia con el pulsador, se guarda en SRAM
5. Los datos persisten aunque se quite la alimentación (batería de litio)

## Compilación

```bash
make
make clean  # Limpiar build anterior
```

## Programación

```bash
make flash  # Si está configurado
# O usar MPLAB IPE para cargar el .hex
```

## Notas

- El M48T559Y se envía de fábrica con el oscilador detenido (ST=1)
- Usar `m48txx_rtc_start()` para iniciar el RTC
- La SRAM está disponible en direcciones 0x0000-0x1FEF
- El RTC usa registros en 0x1FF8-0x1FFF
- Formato BCD: 0x45 = 45 decimal
