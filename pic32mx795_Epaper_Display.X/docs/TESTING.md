# Testing — Estrategia de pruebas

Este documento describe cómo verificar el correcto funcionamiento del driver
E-Paper y del conexionado, tanto en la placa como a nivel de lógica.

## 1. Verificación de pines (hardware)

Antes de integrar el driver completo conviene comprobar el cableado de los pines
de control del display. El driver expone tres funciones de prueba:

| Función | Comportamiento |
|---------|----------------|
| `test_pines_display(rep, ms)` | Secuencia de pulsos (rep ciclos, ms por pulso) |
| `test_pines_simultaneos(ms)` | Activa todos los pines a la vez (ms) |
| `test_pines_carrusel(rep, ms)` | Recorre cada pin en carrusel (rep vueltas, ms) |

Estas funciones son útiles con un multímetro/scope para confirmar que cada pin
(BUSY/DC/RESET/CS) llega al pad correcto del display.

Para ejecutarlas, descomentar `loop_scope();` en `main()`.

## 2. Prueba funcional básica (demo)

`main.cpp` incluye una demo que cubre la secuencia esencial:

1. Inicio con LED0.
2. Generación de un **patrón de prueba** (marco perimetral) en `imageBW`/`imageSecond`.
3. `COG_initial()` (LED1 al iniciar, LED2 al terminar).
4. `globalUpdate()` (LED3 al completar el refresco).
5. `COG_powerOff()` (LED4).
6. Parpadeo de todos los LEDs al finalizar.

**Criterio de aceptación:** la pantalla muestra un marco en el perímetro y los LEDs
indican correctamente cada etapa.

## 3. Pruebas del frame buffer

Verificar que el tamaño del buffer coincida con la resolución seleccionada:

```cpp
EPD_Driver display(eScreen_EPD_213, boardConfig);
uint32_t size = display.getFrameSize();      // 2756 para 2.13"
uint16_t w = display.getWidth();             // 212
uint16_t h = display.getHeight();            // 104
```

Usar siempre `getFrameSize()` (o la macro `frameSize_EPD_xxx`) para dimensionar
los buffers y evitar desbordamientos.

## 4. Pruebas por pantalla soportada

Aunque la demo usa 2.13", el driver soporta 8 tamaños. Para validar otro panel:

1. Cambiar el parámetro del constructor (p. ej. `eScreen_EPD_287`).
2. Dimensionar los buffers con el `frameSize` correspondiente.
3. Verificar resolución y datos en el display.

## 5. Pruebas de temporización

- `delayMs()` usa el Core Timer (SYS_FREQ/2 = 40 MHz). Validar que la secuencia
  COG respete los tiempos del datasheet de Pervasive Displays.
- En caso de fallos de refresco, revisar los tiempos de `_reset()` y del
  `_DCDC_powerOn()`.

## 6. Checklist de integración

- [ ] Pines SPI correctos (SCK=RB14, SDO=RF5, SDI=RF4).
- [ ] Pines de control (BUSY/DC/RESET/CS) cableados y configurados.
- [ ] Frecuencia del sistema 80 MHz verificada.
- [ ] Buffers del tamaño correcto según `getFrameSize()`.
- [ ] Secuencia `COG_initial -> globalUpdate -> COG_powerOff` sin errores.
- [ ] `.gitignore` presente (artefactos de build ignorados).
