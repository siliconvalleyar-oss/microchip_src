# Roadmap — pic32mx795_Epaper_Display.X

Trabajo actual y pendiente del driver E-Paper.

## Done / En curso

- [x] Driver base `EPD_Driver` (COG init, global update, power off).
- [x] Soporte de 8 tamaños de pantalla Pervasive Displays.
- [x] SPI nativo (SPI4, 8 MHz) sin `plib.h`.
- [x] Pines configurables vía `pins_t` + funciones de prueba de pines.
- [x] Demo de referencia en `main.cpp` (patrón + LEDs de estado).
- [x] Documentación inicial en `docs/`.
- [x] `.gitignore` para artefactos de MPLAB X.

## Pendiente / sugerido

### Funcionalidad
- [ ] Función de renderizado de texto e imágenes sobre el frame buffer.
- [ ] Soporte de partial refresh para pantallas que lo permiten.
- [ ] Manejo de temperaturas activas (comando temerature) según datasheet.
- [ ] Gestión de ahorro de energía con temporizaciones configurables.

### Robustez
- [ ] Timeout en la espera de BUSY para evitar bloqueos.
- [ ] Verificación de tamaño de buffer al enviar frames.
- [ ] Soporte explícito de otros micros de la familia (F512L/F512H configurable).

### Herramientas
- [ ] Script de conversión de imagen (PNG/BMP -> frame buffer de 1-bit).
- [ ] Añadir ejemplos por cada pantalla soportada.

## Prioridades

1. **Estabilidad** del flujo COG en todas las pantallas.
2. **Renderizado de contenido** (texto/imágenes) para uso real en aplicaciones.
3. **Documentación/ejemplos** por pantalla.

> Nota: este branch convive con otras variantes de E-Paper (`_1v1` a `_1v5`). La
> hoja de ruta aquí es específica del driver base de este branch.
