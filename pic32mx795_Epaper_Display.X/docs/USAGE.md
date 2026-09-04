# Uso de la API del driver (EPD_Driver)

## Dependencias

- `EPD_Configuration.h` — presente en el directorio del proyecto.
- `EPD_Driver.h` — incluye la clase y las funciones auxiliares.
- Compilador **XC32 v5.00** y DFP **1.7.380**.

## Instanciar el driver

```cpp
#include "EPD_Driver.h"

// Seleccionar la pantalla y el mapeo de pines
EPD_Driver display(eScreen_EPD_213, boardConfig);
```

Pantallas disponibles (`EPD_Configuration.h`):

| Macro | Tamaño | Resolución | Frame buffer |
|-------|--------|-----------|--------------|
| `eScreen_EPD_154` | 1.54" | 152x152 | 2888 B |
| `eScreen_EPD_213` | 2.13" | 212x104 | 2756 B |
| `eScreen_EPD_266` | 2.66" | 296x152 | 5624 B |
| `eScreen_EPD_271` | 2.71" | 264x176 | 5808 B |
| `eScreen_EPD_287` | 2.87" | 296x128 | 4736 B |
| `eScreen_EPD_370` | 3.70" | 416x240 | 12480 B |
| `eScreen_EPD_417` | 4.17" | 300x400 | 15000 B |
| `eScreen_EPD_437` | 4.37" | 480x176 | 10560 B |

## Secuencia típica

```cpp
int main(void) {
    // 1. Configuración de reloj (HS 8 MHz + PLL x20 = 80 MHz)
    // 2. Inicialización de periféricos y LEDs

    // Buffers de imagen (uno por frame)
    static uint8_t frame1[2756];   // B/W
    static uint8_t frame2[2756];   // segundo color (0x00 en monocromo)

    EPD_Driver display(eScreen_EPD_213, boardConfig);

    display.COG_initial();          // arranque del controlador COG
    display.globalUpdate(frame1, frame2);  // refresco de pantalla
    display.COG_powerOff();         // apagado (ahorro de energía)
}
```

## Métodos públicos

### `EPD_Driver(eScreen_EPD_t screen, pins_t board)`
Constructor. Almacena la configuración y calcula `image_data_size`, `width` y `height`
según el código de pantalla.

### `void COG_initial()`
Realiza la secuencia de inicialización del chip COG (reset, DC/DC on y escritura de
registros de configuración). Debe llamarse antes de `globalUpdate`.

### `void globalUpdate(const uint8_t* data1s, const uint8_t* data2s)`
Envía los dos frames de imagen y dispara el refresco global de pantalla.
- `data1s`: frame principal (blanco/negro).
- `data2s`: frame secundario; en displays monocromo pasar un buffer de `0x00`.

### `void COG_powerOff()`
Apaga el controlador COG y el convertidor DC/DC para minimizar el consumo.

### Consultas
- `uint32_t getFrameSize() const` — tamaño en bytes del frame buffer.
- `uint16_t getWidth() const` — ancho en píxeles.
- `uint16_t getHeight() const` — alto en píxeles.

## Funciones auxiliares (nivel C)

Declaradas en `EPD_Driver.h`:

- `void delayMs(uint32_t ms)` — delay en milisegundos.
- `void test_pines_display(uint16_t repeticiones, uint16_t tiempo_ms)`
- `void test_pines_simultaneos(uint16_t tiempo_ms)`
- `void test_pines_carrusel(uint16_t repeticiones, uint16_t tiempo_ms)`

Estas funciones sirven para **verificar el cableado** de los pines BUSY/DC/RESET/CS
(secuencia de pulsos, carrusel o activación simultánea) durante la puesta en marcha.

## Notas

- Fair taman Buffers: utilice exactamente el tamaño devuelto por `getFrameSize()`
  (o la macro `frameSize_EPD_xxx`) para el buffer de cada frame.
- El display 2.13" usado en `main.cpp` requiere buffers de `2756` bytes.
