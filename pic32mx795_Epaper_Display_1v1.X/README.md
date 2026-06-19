# PIC32MX795F512H - E-Paper Display v1.1 (Pin Test)

**Descripción:** Primera versión del driver para pantallas E-Paper Pervasive Displays con funciones de prueba de pines. Incluye test_all_pins() que verifica cada pin de control como salida (parpadeo) y RB0 como entrada (BUSY).

## Características
- Funciones de prueba: test_pin_output, test_pin_input, test_all_pins
- Prueba de pines: RB0-RB3, RB13-RB14, RF4-RF5
- Verificación de señal BUSY en RB0
- Pins mapeados por número lógico (0-96+)
- Sistema a 80MHz (PLL desde 8MHz HS)

## Periféricos Utilizados
- SPI4, GPIO

## Archivos
| Archivo | Descripción |
|---------|-------------|
| test.c | Funciones de test de pines |
| test.h | Header para tests |
| EPD_Driver.h | Driver EPD base |
| EPD_Graphics.h | Primitivas gráficas |
| EPD_Configuration.h | Configuración de pantalla |
