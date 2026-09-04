# PIC32MX795F512H - SPI4 Test for E-Paper Pins

**Descripción:** Prueba de configuración SPI4 en modo Master para PIC32MX795F512H de 64 pines. Genera señal de reloj SCK4 y datos SDO4 para verificar con osciloscopio que la configuración PPS (Peripheral Pin Select) y SPI son correctas.

## Características
- SPI4 Master a 8MHz (SCK4=RB14, SDO4=RF5, SDI4=RF4)
- Configuración PPS (Peripheral Pin Select) para mapeo de pines
- Liberación de pines JTAG (DDPCON)
- Envío continuo de byte 0xAA para verificación con osciloscopio
- Sistema 80MHz (cristal HS 8MHz + PLL x20)

## Periféricos Utilizados
- SPI4, GPIO, PPS

## Pines
| Pin físico | Pin lógico | Función |
|------------|------------|---------|
| 29 (TQFP-64) | RB14 | SCK4 |
| 32 (TQFP-64) | RF5 | SDO4 |
| 31 (TQFP-64) | RF4 | SDI4 |
