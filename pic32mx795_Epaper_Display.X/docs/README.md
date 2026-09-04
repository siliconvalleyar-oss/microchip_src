# PIC32MX795F512H - E-Paper Display Driver (EPD)

**Descripción:** Driver base para pantallas E-Paper de Pervasive Displays usando el
microcontrolador **PIC32MX795F512H** (64 pines), controlado por **SPI nativo** sin
dependencia de `plib.h`.

Este proyecto forma parte de la familia `pic32mx795_Epaper_Display` del repositorio
`microchip_src`.

## Índice de la documentación

| Documento | Contenido |
|-----------|-----------|
| [README.md](README.md) | Esta página: visión general del proyecto |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Arquitectura del software y flujo de trabajo |
| [USAGE.md](USAGE.md) | Uso de la API del driver y ejemplo de integración |
| [HARDWARE.md](HARDWARE.md) | Conexionado (pines SPI, PPS, LEDs) y placa |
| [DEPLOY.md](DEPLOY.md) | Compilación y programación (flasheo) del dispositivo |
| [TESTING.md](TESTING.md) | Estrategia de pruebas y verificación de pines |
| [ROADMAP.md](ROADMAP.md) | Trabajo pendiente y mejoras sugeridas |
| [SKILL_EPAPER.md](SKILL_EPAPER.md) | Referencia práctica rápida del driver E-Paper |
| [CHANGELOG.md](CHANGELOG.md) | Historial de versiones del proyecto |

## Características

- Soporte para pantallas E-Paper Pervasive Displays de varios tamaños:
  **1.54", 2.13", 2.66", 2.71", 2.87", 3.70", 4.17", 4.37"**
- Control por **SPI nativo** (sin `plib.h`), 8 MHz
- Frame buffers calculados automáticamente para cada tamaño
- Sistema a **80 MHz** (cristal HS 8 MHz + PLL x20)
- Pines configurables mediante la estructura `pins_t`
- Registros de inicialización para pantallas pequeñas/medianas

## Periféricos Utilizados

- **SPI4** (maestro) — transferencia de datos al display
- **GPIO** — pines BUSY / DC / RESET / CS + LEDs de estado
- **Timer / delay** — temporización de la secuencia COG

## Herramientas

| Herramienta | Versión |
|-------------|---------|
| MPLAB X IDE | 6.35 |
| XC32 Compiler | 5.00 |
| Device Pack (DFP) | PIC32MX_DFP 1.7.380 |
| Microcontrolador | PIC32MX795F512H |

## Estado del proyecto

- **VERSION** branch: `1.1.0`
- **Estado**: funcional, driver base documentado.
- Ver [CHANGELOG.md](CHANGELOG.md) para el historial.

> Este es un *driver base* orientado a demostración/integración: `main.cpp` muestra un
> patrón de prueba y la secuencia típica (init -> update -> powerOff) con LEDs de estado.
