# Hardware — Conexionado y placa

## Microcontrolador

- **Dispositivo:** PIC32MX795F512H (SOIC/SPDIP de 64 pines)
- **Frecuencia del sistema:** 80 MHz
  - Oscilador HS externo de 8 MHz
  - PLL: FPLLIDIV=2, FPLLMUL=20, FPLLODIV=1
- **Frecuencia periférica (PB):** 80 MHz (FPBDIV=1)

### Configuración de fuses (`main.cpp`)

```c
#pragma config FNOSC   = PRIPLL
#pragma config POSCMOD = HS
#pragma config FPLLIDIV = DIV_2
#pragma config FPLLMUL = MUL_20
#pragma config FPLLODIV = DIV_1
#pragma config FPBDIV  = DIV_1
#pragma config FWDTEN  = OFF
#pragma config ICESEL  = ICS_PGx1
#pragma config CP      = OFF
```

## Pantalla E-Paper

Control de displays E-Paper de **Pervasive Displays** mediante interfaz SPI.

| Tamaño | Resolución | Código | Frame buffer |
|--------|-----------|--------|--------------|
| 1.54" | 152x152 | `eScreen_EPD_154` | 2888 B |
| 2.13" | 212x104 | `eScreen_EPD_213` | 2756 B |
| 2.66" | 296x152 | `eScreen_EPD_266` | 5624 B |
| 2.71" | 264x176 | `eScreen_EPD_271` | 5808 B |
| 2.87" | 296x128 | `eScreen_EPD_287` | 4736 B |
| 3.70" | 416x240 | `eScreen_EPD_370` | 12480 B |
| 4.17" | 300x400 | `eScreen_EPD_417` | 15000 B |
| 4.37" | 480x176 | `eScreen_EPD_437` | 10560 B |

> Fuente: https://www.pervasivedisplays.com/products/

## Conexionado SPI (PPS — Peripheral Pin Select)

Se usa **SPI4** en modo maestro a 8 MHz con PPS:

| Señal | Pin físico | Puerto | Rol SPI | Nota |
|-------|-----------|--------|---------|------|
| SCK  | 29 | RB14 | SCLK | configurado por PPS |
| SDO  | 42 | RF5 | MOSI / SDO4 | configurado por PPS |
| SDI  | 41 | RF4 | MISO / SDI4 | configurado por PPS |

Parámetros SPI: 8-bit, CPOL=0 (idle low), CPHA=0 (captura en flanco ascendente),
SMP=0 (muestreo en medio).

### Pines de control del display

| Señal | Pin físico | Puerto | Dirección |
|-------|-----------|--------|-----------|
| BUSY  | 48 | RD0 | Entrada |
| DC (D/C) | 49 | RD1 | Salida |
| RESET | 50 | RD2 | Salida |
| CS    | 51 | RD3 | Salida |

La tabla completa de mapeo de puertos está comentada en `main.cpp`
(signaux del 0 al 16, puertos B/D/E/F).

## LEDs de estado

Distribución de los 16 LEDs de la placa para indicar el avance de la demo:

| LED | Pin físico | Puerto |
|-----|-----------|--------|
| LED0 | 1 | RE5 |
| LED1 | 2 | RE6 |
| LED2 | 3 | RE7 |
| LED3 | 64 | RE4 |
| LED4 | 63 | RE3 |
| LED5 | 62 | RE2 |
| LED6 | 61 | RE1 |
| LED7 | 60 | RE0 |
| LED8 | 48 | RD7 |
| LED9 | 49 | RD6 |
| LED10 | 50 | RD5 |
| LED11 | 51 | RD4 |
| LED12 | 52 | RD3 |
| LED13 | 53 | RD2 |
| LED14 | 54 | RD1 |
| LED15 | 55 | RD0 |

> Nota: los pines de algunos LEDs comparten puerto con los pines de control del
> display en el mismo rango físico; verificar el circuito de la placa antes de
> cablear simultáneamente.
