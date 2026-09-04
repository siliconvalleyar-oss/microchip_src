# M48T559Y Timekeeper® SRAM + RTC Driver

## Descripción

Driver completo para el chip M48T559Y (y compatible M48T08) de Microchip, que combina:
- **SRAM no volátil**: 8 KB respaldados por batería de litio
- **RTC (Real Time Clock)**: Reloj de tiempo real con calendario
- **Watchdog Timer**: Monitoreo de procesador
- **Alarmas**: Interrupciones programables

## Arquitectura del Driver

```
m48t59y.h     - API pública (structs, funciones, constantes)
m48t59y.c     - Implementación de bajo nivel (bus multiplexado)
main.c        - Aplicación ejemplo (RTC + SRAM + regenerador de señal)
```

## Hardware

### M48T59Y vs M48T08

| Característica | M48T559Y | M48T08 |
|----------------|----------|--------|
| Bus | Multiplexado AD0-AD7 | Paralelo A0-A12 + D0-D7 |
| Encapsulado | 28-pin SOIC + SNAPHAT | 28-pin DIP/SOIC |
| Registros RTC | Idénticos | Idénticos |
| SRAM | 8 KB | 8 KB |

### Pinout PIC18F4620 -> M48T559Y

| Señal | PIC18F4620 | Función |
|-------|------------|---------|
| AD0-AD7 | PORTD (RD0-RD7) | Bus multiplexado dirección/dato |
| AS0 | RA6 | Address Strobe 0 (byte bajo) |
| AS1 | RA7 | Address Strobe 1 (byte alto) |
| E | RC0 | Chip Enable (activo bajo) |
| R | RC3 | Read Enable (activo bajo) |
| W | RC4 | Write Enable (activo bajo) |
| RST | RC5 | Reset entrada (opcional) |
| IRQ/FT | RC6 | Alarma/Watchdog (opcional) |

## API del Driver

### Inicialización

```c
void m48txx_init(void);
```

Inicializa pines de control y bus de datos. Debe llamarse una vez al inicio del programa.

### Acceso a SRAM

```c
uint8_t m48txx_read_sram(uint16_t address);
void m48txx_write_sram(uint16_t address, uint8_t data);
```

Acceso a memoria SRAM no volátil (direcciones 0x0000-0x1FEF).

### RTC - Nivel Bajo

```c
void m48txx_rtc_start(void);
void m48txx_rtc_stop(void);
void m48txx_rtc_set_time(uint8_t year, uint8_t month, uint8_t day,
                          uint8_t hour, uint8_t minute, uint8_t second);
void m48txx_rtc_get_time(uint8_t *year, uint8_t *month, uint8_t *day,
                          uint8_t *hour, uint8_t *minute, uint8_t *second);
```

Control directo del RTC. Los valores están en formato BCD.

### RTC - Nivel Alto (DateTime)

```c
typedef struct {
    uint8_t year;    // 00-99
    uint8_t month;   // 01-12
    uint8_t date;    // 01-31
    uint8_t day;     // 01-07 (día de la semana)
    uint8_t hours;   // 00-23
    uint8_t minutes; // 00-59
    uint8_t seconds; // 00-59
} TK_DateTime;

void m48txx_get_datetime(TK_DateTime *dt);
void m48txx_set_datetime(const TK_DateTime *dt);
```

API conveniente con conversión BCD automática.

### Conversiones BCD

```c
uint8_t bin_to_bcd(uint8_t bin);
uint8_t bcd_to_bin(uint8_t bcd);
```

## Secuencia de Acceso al Bus

### Lectura

1. Cargar byte bajo de dirección en AD0-AD7
2. Pulso en AS0 (flanco de subida)
3. Cargar byte alto de dirección en AD0-AD7
4. Pulso en AS1 (flanco de subida)
5. Poner E=0, R=0 (chip enable + read enable)
6. Leer dato desde AD0-AD7
7. Poner R=1, E=1 (fin de ciclo)

### Escritura

1. Cargar byte bajo de dirección en AD0-AD7
2. Pulso en AS0
3. Cargar byte alto de dirección en AD0-AD7
4. Pulso en AS1
5. Cargar dato en AD0-AD7
6. Poner E=0, W=0 (chip enable + write enable)
7. Pulso en W
8. Poner W=1, E=1 (fin de ciclo)

## Formato BCD

El M48T559Y almacena valores de fecha/hora en formato BCD:

- **Byte BCD**: nibble alto = decenas, nibble bajo = unidades
- **Ejemplo**: 0x45 = 45 decimal, 0x23 = 23 decimal
- **Rango**: 0x00-0x99 para años, 0x01-0x12 para meses, etc.

## Registros del RTC

| Dirección | Registro | Función |
|-----------|----------|---------|
| 0x1FF0 | FLAGS | Flags (WDF, AF, BL) |
| 0x1FF1 | - | Reservado |
| 0x1FF2-0x1FF5 | ALARM | Alarma programable |
| 0x1FF6 | INTERRUPT | Control de interrupciones |
| 0x1FF7 | WATCHDOG | Watchdog timer |
| 0x1FF8 | CONTROL | Control (W, R, S) |
| 0x1FF9 | SECONDS | Segundos (ST bit) |
| 0x1FFA | MINUTES | Minutos |
| 0x1FFB | HOURS | Horas |
| 0x1FFC | DAY | Día de la semana (FT bit) |
| 0x1FFD | DATE | Día del mes |
| 0x1FFE | MONTH | Mes |
| 0x1FFF | YEAR | Año (00-99) |

## Uso en el Proyecto

### Almacenamiento de Frecuencia

El proyecto usa la SRAM del M48T559Y para persistir la frecuencia de RB3:

```c
// Guardar
uint16_t addr = 0x0000;
m48txx_write_sram(addr,     0xA5);      // Magic byte
m48txx_write_sram(addr + 1, freq_h);    // Byte alto
m48txx_write_sram(addr + 2, freq_l);    // Byte bajo

// Leer
uint8_t magic = m48txx_read_sram(addr);
if (magic == 0xA5) {
    current_rb3_freq = (freq_h << 8) | freq_l;
}
```

### Inicialización del RTC

```c
m48txx_init();
m48txx_rtc_start();  // Inicia el oscilador 32.768 kHz

// Configurar fecha/hora
TK_DateTime dt = {
    .year = 26,
    .month = 9,
    .date = 4,
    .day = 5,    // Viernes
    .hours = 12,
    .minutes = 0,
    .seconds = 0
};
m48txx_set_datetime(&dt);

// Leer fecha/hora
m48txx_get_datetime(&dt);
```

## Consideraciones de Diseño

### Timing

- **tAS**: 20 ns (setup dirección) -> __delay_us(1) sobra margen
- **tDS**: 60 ns (setup datos) -> __delay_us(1) sobra margen
- **tWLWH**: 50 ns (pulso W mínimo) -> NOPs suficientes

### Alimentación

- **VCC**: 4.5V - 5.5V
- **Decoupling**: Capacitor 0.1µF cerca de VCC-VSS
- **Protección**: Diodo Schottky VCC a VSS (1N5817)
- **Pull-ups**: 1kΩ en RST, 500Ω-10kΩ en IRQ/FT

### Batería

- **Tipo**: Litio 3V (SNAPHAT)
- **Duración**: Mínimo 7 años a 25°C
- **Almacenamiento**: No usar espuma conductiva (drena batería)

## Troubleshooting

### Problema: Lectura incorrecta de datos

**Solución**: Verificar:
1. Conexiones de bus AD0-AD7
2. Señales AS0/AS1 con flanco de subida
3. Tiempos de setup/hold suficientes
4. Resistencia pull-up en RST

### Problema: RTC no inicia

**Solución**: El M48T559Y se envía de fábrica con ST=1 (oscilador detenido). Llamar a `m48txx_rtc_start()` después de `m48txx_init()`.

### Problema: Pérdida de hora al apagar

**Solución**: Verificar:
1. Batería de litio insertada correctamente
2. SNAPHAT no en espuma conductiva
3. VCC no cae más rápido que tF (300µs)

## Referencias

- [M48T559Y Datasheet](https://www.microchip.com/wwwproducts/en/M48T559Y)
- [M48T08 Datasheet](https://www.microchip.com/wwwproducts/en/M48T08)
- AN2001: "Using the M48T59Y Timekeeper SRAM"
