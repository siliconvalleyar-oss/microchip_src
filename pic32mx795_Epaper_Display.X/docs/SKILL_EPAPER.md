# SKILL_EPAPER.md — Referencia práctica del driver E-Paper

> Guía resumida de trabajo con el driver `EPD_Driver` para PIC32MX795. Pensada
> como referencia rápida de consulta al editar, depurar o integrar este módulo.

## 1. Resumen de la API

Clase principal: `EPD_Driver` (`EPD_Driver.h` / `EPD_Driver.cpp`).

```cpp
// Constructor
EPD_Driver(eScreen_EPD_t screen, pins_t board);
```

| Método | Finalidad |
|--------|-----------|
| `COG_initial()` | Arranque del chip COG (reset, DC/DC, registros). **Antes** de update. |
| `globalUpdate(d1, d2)` | Refresco global (d1 = B/W, d2 = 2.º color / 0x00 monocromo). |
| `COG_powerOff()` | Apagado del COG y DC/DC (ahorro de energía). |
| `getFrameSize()` | Tamaño del frame buffer en bytes. |
| `getWidth()` / `getHeight()` | Resolución en píxeles. |

Funciones de soporte (nivel C): `delayMs()`, `test_pines_display()`,
`test_pines_simultaneos()`, `test_pines_carrusel()`.

## 2. Flujo típico (patrón mínimo)

```cpp
static uint8_t frame1[2756];   // usar getFrameSize() para dimensionar
static uint8_t frame2[2756];

EPD_Driver display(eScreen_EPD_213, boardConfig);  // 2.13"

display.COG_initial();            // 1
display.globalUpdate(frame1, frame2);  // 2
display.COG_powerOff();           // 3
```

**Regla:** siempre `COG_initial()` -> `globalUpdate()` -> `COG_powerOff()`.

## 3. Pantallas soportadas (EPD_Configuration.h)

| Código | Tamaño | Resolución | Buffer |
|--------|--------|-----------|--------|
| `eScreen_EPD_154` | 1.54" | 152x152 | 2888 |
| `eScreen_EPD_213` | 2.13" | 212x104 | 2756 |
| `eScreen_EPD_266` | 2.66" | 296x152 | 5624 |
| `eScreen_EPD_271` | 2.71" | 264x176 | 5808 |
| `eScreen_EPD_287` | 2.87" | 296x128 | 4736 |
| `eScreen_EPD_370` | 3.70" | 416x240 | 12480 |
| `eScreen_EPD_417` | 4.17" | 300x400 | 15000 |
| `eScreen_EPD_437` | 4.37" | 480x176 | 10560 |

## 4. Parámetros de hardware clave

- **MCU:** PIC32MX795F512H, 80 MHz (HS 8 MHz + PLL x20).
- **SPI4** maestro, 8 MHz, 8-bit, CPOL=0, CPHA=0, SMP=0.
- **PPS:** SCK=RB14 (pin 29), SDO=RF5 (pin 42), SDI=RF4 (pin 41).
- **Control:** BUSY=RD0 (48), DC=RD1 (49), RESET=RD2 (50), CS=RD3 (51).
- Pines definidos vía estructura `pins_t` (port code: RB0=32, RB1=33, ...).

## 5. Reglas de edición / convenciones

- **No introducir `plib.h`** — el módulo es SPI/registros nativos.
- Mantener el driver **independiente del tamaño** (usar `getFrameSize()`).
- `delayMs()` depende de `SYS_FREQ/2` (Core Timer 40 MHz).
- Respetar la secuencia del fabricante en `COG_initial()` (reset -> DCDC -> registros
  -> refresh).
- Documentar cualquier cambio en `docs/CHANGELOG.md`.

## 6. Troubleshooting rápido

| Síntoma | Posible causa / solución |
|---------|--------------------------|
| Pantalla en blanco | Falta `COG_initial()` antes del update, o buffers de 0x00 |
| Refresco incompleto | Tamaño de buffer incorrecto (usar `getFrameSize()`) |
| Sin respuesta SPI | Revisar PPS (SCK/SDO/SDI) y niveles de DC/CS/RESET |
| Se bloquea el MCU | Espera de BUSY sin timeout; revisar el pin BUSY |
| Consumo alto tras usar | Faltó `COG_powerOff()` |
| LEDs no encienden | Dirección de LEDs mal configurada (`HardwareProfile.h`) |

## 7. Convención del repositorio

- `VERSION` vive en la **raíz del branch** (no dentro del proyecto).
- Los `docs/` del proyecto documentan el módulo en particular.
- Los artefactos de MPLAB X (`build/`, `dist/`, `.generated_files/`) están
  **ignorados** por `.gitignore` — no se commitean.
