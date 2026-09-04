# Compilación y programación (Deploy)

## Requisitos

- **MPLAB X IDE** 6.35 (o superior compatible).
- **XC32** compiler v5.00.
- **PIC32MX_DFP** 1.7.380 (device pack).
- Un programador compatible para PIC32, p. ej. **PICkit 4** / **PK5**.

> En este trabajo se configuró el proyecto con plataforma `noID` (sin herramienta
> de programación definida) — el target es **PIC32MX795F512H**.

## Compilar desde MPLAB X

1. Abrir el proyecto `pic32mx795_Epaper_Display.X` en MPLAB X.
2. Seleccionar la configuración (p. ej. `default`).
3. Limpiar y compilar: botón **Clean and Build** (o `Shift+F11`).

### Compilar desde línea de comandos (make)

Desde la raíz del proyecto:

```bash
# Build de producción (genera .hex)
make CONF=default TYPE_IMAGE=IMAGE_RUN build

# Build de depuración (genera .elf con símbolos)
make CONF=default TYPE_IMAGE=IMAGE_RUN clean
make CONF=default TYPE_IMAGE=IMAGE_DEBUG build
```

### Outputs generados

```
dist/default/production/pic32mx795_Epaper_Display.X.production.hex
dist/default/production/pic32mx795_Epaper_Display.X.production.elf
```

> Los directorios `build/`, `dist/` y `.generated_files/` están **ignorados** por
> git y no forman parte del repositorio.

## Programar (flashear) el microcontrolador

Desde MPLAB X:

1. Conectar el programador (PICkit/PK5) y el dispositivo.
2. Configurar la herramienta en *Project Properties* → conf → *Hardware Tool*.
3. Seleccionar la configuración con el hex de producción.
4. Botón **Make and Program Device** (o `F6`).

### Verificación de pines

Para validar el cableado antes de la integración, el driver incluye funciones de
prueba de pines (ver `USAGE.md` y `TESTING.md`):
`test_pines_display()`, `test_pines_simultaneos()`, `test_pines_carrusel()`.

## Notas de flasheo

- Verificar que el cristal HS de 8 MHz y el PLL estén correctos para que el reloj
  del sistema sea 80 MHz (ver `HARDWARE.md`).
- Si el display no responde, revisar orden de la secuencia `COG_initial()` y los
  niveles lógicos de BUSY/DC/RESET/CS.
