# PIC18F4620 - Metal Detector (Full Featured)

**Descripción:** Detector de metales profesional con discriminación ferroso/no-ferroso, vúmetro LCD de 40 segmentos, menú de configuración, EEPROM para guardado de calibración y control de bobina PWM.

## Características
- Detección con discriminación por fases (ferroso vs no ferroso)
- Vúmetro LCD de 40 segmentos con respuesta suavizada
- Sensibilidad ajustable (potenciómetro digital)
- Discriminación ajustable
- Tono de audio PWM variable según intensidad
- Menú con 6 modos de configuración
- Auto-calibración (Auto Zero) por botón o automática
- EEPROM para guardar configuración al apagar
- Control de backlight
- Monitor de batería con apagado automático
- Mute toggle
- Debug mode (muestra valores en tiempo real)

## Menús
| Menú | Descripción |
|------|-------------|
| 0 | Detector (principal) |
| 2 | Volumen |
| 3 | Tono |
| 4 | Discriminación |
| 5 | Ganancia |
| 6 | Grueso/Fino |

## Periféricos Utilizados
- ADC, PWM, Timer2, EEPROM, GPIO, LCD 4-bit

## Archivos
| Archivo | Descripción |
|---------|-------------|
| main.c | Bucle principal y lógica de detección |
| config.h | Definiciones y pines |
| eeprom.c/h | Gestión EEPROM |
| timers.c/h | Temporización e ISR |
| rutinasMisc.c/h | LCD, PWM, ADC, menú |
| uart.c/h | Debug serial |
