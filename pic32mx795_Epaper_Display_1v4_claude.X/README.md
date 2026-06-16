# Port E-Paper: Raspberry Pi Zero 2W → PIC32MX795F512H

## Mapa de pines

| Señal      | RPi GPIO | PIC32 pin   | Notas                    |
|------------|----------|-------------|--------------------------|
| Panel_CS   | GPIO 8   | **RB3**     | CS manual (ECSM)         |
| Flash_CS   | GPIO 22  | **RB13**    | CS manual (FCSM)         |
| MOSI       | GPIO 10  | **RF5**     | SPI4 SDO4                |
| MISO       | GPIO 9   | **RF4**     | SPI4 SDI4                |
| RST        | GPIO 23  | **RB2**     | GPIO salida              |
| D/C        | GPIO 24  | **RB1**     | GPIO salida              |
| BUSY       | GPIO 25  | **RB0**     | GPIO entrada             |
| SCK        | GPIO 11  | **RB14**    | SPI4 SCK4                |

## Archivos generados

| Archivo             | Descripción                                              |
|---------------------|----------------------------------------------------------|
| `epaper.h`          | Definiciones, estructura `EPD_Driver_t`, API HAL         |
| `epaper.c`          | HAL GPIO (puertos B/F), HAL SPI4, driver del panel       |
| `epaper_display.h`  | API de alto nivel: buffer, dibujo, texto                 |
| `epaper_display.c`  | Implementación del buffer y las funciones de dibujo      |
| `fonts_manager.h`   | Enum `FontType`, estructura `FontManager_t`, API         |
| `fonts_manager.c`   | Gestión de fuentes (C puro, sin clases)                  |
| `boards.h`          | Exporta `boardPIC32MX795` y alias `BOARD_DEFAULT`        |
| `main.c`            | Equivalente al `main.cpp` de RPi, con fuses incluidos    |

Los archivos de datos de fuentes (`fonts.c` / `fonts.h`) y los bitmaps de imagen  
(`image_266_296x152_BW.cpp`, `qr_code.cpp`) **no necesitan modificación**;  
compilan sin cambios bajo XC32 porque son arrays `const uint8_t[]` estándar.

## Cambios respecto a la versión RPi

### 1. C++ → C puro
- La clase `EPD_Driver` se convirtió en la estructura `EPD_Driver_t`  
  con funciones de prefijo `EPD_*`.
- La clase `EpaperDisplay` se convirtió en `EpaperDisplay_t`  
  con funciones de prefijo `Display_*`.
- La clase `FontManager` se convirtió en `FontManager_t`  
  con funciones de prefijo `FM_*`.
- Se eliminó `std::unique_ptr`; el driver se almacena por valor dentro  
  del display (sin asignación dinámica de memoria).

### 2. bcm2835 → registros SFR del PIC32
| bcm2835                        | PIC32 equivalente                     |
|--------------------------------|---------------------------------------|
| `bcm2835_gpio_fsel(pin, mode)` | `TRISBbits.TRISBx = 0/1`             |
| `bcm2835_gpio_write(pin, v)`   | `LATBSET/LATBCLR = (1<<bit)`          |
| `bcm2835_gpio_lev(pin)`        | `(PORTB >> bit) & 1`                  |
| `bcm2835_spi_transfer(b)`      | escritura/lectura de `SPI4BUF`        |
| `usleep(ms * 1000)`            | bucle con `_CP0_GET_COUNT()`          |

### 3. SPI
- Se usa **SPI4** del PIC32 (pines nativos RF4/RF5/RB14).
- El CS se gestiona manualmente igual que en la versión RPi  
  (CS = bajo antes de enviar, CS = alto después).
- Frecuencia configurada en ~3 MHz (`SPI4BRG = 12` con PBCLK = 80 MHz).  
  Ajustar `SPI4BRG` si se usa otro PBCLK.

### 4. Fuses
Los fuses de configuración del PIC32 están en `main.c` con `#pragma config`.  
Ajustar `FPLLIDIV` / `FPLLMUL` según el cristal utilizado.

## Notas de compilación (MPLAB X + XC32)

1. Añadir todos los `.c` al proyecto.
2. Añadir las rutas de include si los headers están en subdirectorios.
3. `fonts.c` puede ser muy grande; verificar que el linker script tenga  
   suficiente espacio en flash (PIC32MX795F512H tiene 512 KB).
4. El buffer de imagen ocupa `296 × 152 / 8 = 5624 bytes` en RAM.  
   El PIC32MX795F512H tiene 128 KB de RAM, así que no hay problema.
5. Habilitar optimización `-O1` o superior para reducir el tamaño del código.
