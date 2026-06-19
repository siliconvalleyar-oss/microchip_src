# PIC32MX795F512H - E-Paper Digital Clock (v1.4 Claude)

**Descripción:** Reloj digital para display E-Paper de 2.66" (296×152) con actualización por zonas (zone refresh) sin parpadeo. Muestra hora en formato HH:MM:SS con fuente grande, y refresca solo el área del reloj cada segundo.

## Características
- Display EPD 2.66" (296×152) de Pervasive Displays
- Zone refresh: actualiza solo el área del reloj (~300ms, sin parpadeo)
- Full refresh automático cada 10 ciclos (anti-ghosting)
- Imagen de presentación al inicio
- Fuente Terminal 16×24 para hora grande
- Hora simulada (HH:MM:SS) — preparado para RTC real
- SPI4: SCK=RB14, SDO=RF5, SDI=RF4
- Sistema 80MHz, SPI 8MHz

## Periféricos Utilizados
- SPI4, GPIO, Timer

## Archivos
| Archivo | Descripción |
|---------|-------------|
| main.c | Lógica del reloj digital |
| epaper.c/h | Driver EPD low-level |
| epaper_display.c/h | Capa de display (dibujo) |
| epaper_refresh.c/h | Sistema de refresco por zonas |
| fonts.c/h | Fuentes bitmap |
| boards.h | Configuración de pines |
