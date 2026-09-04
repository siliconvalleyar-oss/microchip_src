# Changelog

Todos los cambios notables de este proyecto serán documentados en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/es/1.0.0/),
y este proyecto adhiere a [Semantic Versioning](https://semver.org/lang/es/).

## [1.0.0] - 2026-09-04

### Agregado
- Driver completo M48T559Y/M48T08 Timekeeper® SRAM + RTC
- Interfaz multiplexada AD0-AD7 con AS0/AS1 para direccionamiento
- API de alto nivel con struct `TK_DateTime` para fecha/hora
- Conversión BCD automática (bin_to_bcd / bcd_to_bcd)
- Funciones RTC: start/stop/set/get con freeze/thaw usando registro CONTROL
- Acceso a SRAM no volátil (8KB respaldados por batería)
- Integración en main.c: frecuencia RB3 persistente en SRAM
- Documentación completa en `docs/SKILL.md`
- Datasheet reference en `docs/M48T559Y.md`

### Características del Driver
- `m48txx_init()` - Inicialización de pines y bus
- `m48txx_read()` / `m48txx_write()` - Acceso raw a registros
- `m48txx_read_sram()` / `m48txx_write_sram()` - Acceso a SRAM
- `m48txx_rtc_start()` / `m48txx_rtc_stop()` - Control del oscilador
- `m48txx_get_datetime()` / `m48txx_set_datetime()` - API DateTime
- `bin_to_bcd()` / `bcd_to_bin()` - Conversiones de formato

### Hardware
- PIC18F4620 @ 20MHz
- M48T559Y en 28-pin SOIC con SNAPHAT
- Bus multiplexado: AD0-AD7 en PORTD
- Control: AS0=RA6, AS1=RA7, E=RC0, R=RC3, W=RC4

### Documentación
- `docs/SKILL.md` - Guía completa del driver
- `docs/M48T559Y.md` - Especificaciones del datasheet
- `README.md` - Documentación del proyecto

## [0.1.0] - 2026-09-04

### Agregado
- Estructura inicial del proyecto
- Driver básico M48T559Y (lectura/escritura)
- Integración con regenerador de señal existente
- Almacenamiento de frecuencia en SRAM
