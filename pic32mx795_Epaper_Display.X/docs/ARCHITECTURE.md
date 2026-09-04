# Arquitectura del software

## Visión general

El proyecto se organiza en módulos con responsabilidades separadas:

```
pic32mx795_Epaper_Display.X/
├── EPD_Configuration.h      # Definición de pantallas, tamaños, estructura de pines
├── EPD_Driver.h             # Interfaz pública de la clase EPD_Driver
├── EPD_Driver.cpp           # Implementación del protocolo COG por SPI
├── HardwareProfile.h        # Macros de LEDs y configuración de la placa
├── main.cpp                 # Aplicación de prueba / punto de entrada
├── fonts.h                  # Fuentes (si se usan en la app)
├── Makefile                 # Build de MPLAB X
└── nbproject/               # Metadatos de MPLAB X
```

## Flujo de datos

```
        Aplicación (main.cpp)
               │
               ▼
   ┌─────────────────────────┐
   │  EPD_Driver (clase)     │  ──  COG_initial()
   │                         │  ──  globalUpdate(data1s, data2s)
   │                         │  ──  COG_powerOff()
   └───────────┬─────────────┘
               │  _sendIndexData(index, data, len)
               ▼
        SPI4 (maestro)
               │  8 MHz
               ▼
      Display E-Paper (COG)
```

## Módulos

### 1. `EPD_Configuration.h`
Solo cabecera. Centraliza:
- Códigos de pantalla (`eScreen_EPD_xxx`) y su resolución.
- Tamaños de frame buffer en bytes (`frameSize_EPD_xxx`).
- Frecuencias del sistema y del bus SPI.
- La estructura `pins_t` para mapeo de pines.

### 2. `EPD_Driver`
Clase C++ que encapsula el control del display:

| Método | Función |
|--------|---------|
| `EPD_Driver(screen, board)` | Constructor: guarda configuración y calcula frame buffer |
| `COG_initial()` | Secuencia de arranque del chip COG (reset, DCDC, registros) |
| `globalUpdate(d1, d2)` | Refresco global con dos frames (B/W y segundo color) |
| `COG_powerOff()` | Apagado del COG y del DC/DC |
| `getFrameSize()/getWidth()/getHeight()` | Consultas del buffer y resolución |

Métodos privados que implementan la secuencia:
- `_sendIndexData()`: protocolo SPI index/data (DC controla comando vs datos).
- `_softReset()`, `_reset()`, `_DCDC_powerOn()`, `_displayRefresh()`:
  pasos de la secuencia del fabricante (Pervasive Displays).

### 3. `main.cpp`
Aplicación de demostración que:
1. Configura el reloj (HS 8 MHz + PLL x20 = 80 MHz) y los LEDs.
2. Genera un patrón de prueba (marco + relleno) para una pantalla 2.13".
3. Instancia `EPD_Driver(eScreen_EPD_213, boardConfig)`.
4. Ejecuta `COG_initial() -> globalUpdate() -> COG_powerOff()`.
5. Parpadea los LEDs como indicador de fin.

## Configuración de pines

La placa de referencia usa estos pines para la pantalla 2.13":

| Señal | Pin | Puerto | Función |
|-------|-----|--------|---------|
| SCK  | 29 | RB14 | Reloj SPI |
| SDO  | 42 | RF5 | MOSI |
| SDI  | 41 | RF4 | MISO |
| BUSY | 48 | RD0 | Estado del display |
| DC   | 49 | RD1 | Data/Command |
| RESET| 50 | RD2 | Reset |
| CS   | 51 | RD3 | Chip Select |

> El mapeo de pines en `boardConfig` se expresa como **port code** según
> `EPD_Configuration.h` (p. ej. RB0 = 32, RB1 = 33, ...).

## Notas de diseño

- **Sin `plib.h`**: el driver usa registros del micro directamente y SPI nativo.
- **Modelo de dos frames**: `globalUpdate` recibe dos buffers, lo que permite
  displays con segundo color (rojo) o reservar `0x00` para monocromo.
- **Independencia de tamaño**: el tamaño del frame buffer se calcula a partir del
  código de pantalla en el constructor.
