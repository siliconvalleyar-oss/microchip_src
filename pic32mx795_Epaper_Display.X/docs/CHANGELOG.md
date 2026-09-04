# Changelog — pic32mx795_Epaper_Display.X

Todas las versiones notables de este proyecto se documentan aquí.
El formato se basa en [Keep a Changelog](https://keepachangelog.com/es/1.1.0/)
y el proyecto sigue [SemVer](https://semver.org/lang/es/).

## [1.1.0] — 2026-09-04

### Agregado
- Reestructura del proyecto a **layout plano** desde el backup.
- Carpeta `docs/` con documentación completa:
  - `README.md` — visión general.
  - `ARCHITECTURE.md` — arquitectura de software.
  - `USAGE.md` — uso de la API del driver.
  - `HARDWARE.md` — conexionado y placa.
  - `DEPLOY.md` — compilación y programación (flashing).
  - `TESTING.md` — estrategia de pruebas y verificación de pines.
  - `ROADMAP.md` — trabajo pendiente.
  - `SKILL_EPAPER.md` — guía de referencia práctica del driver E-Paper.
- Archivo `VERSION` en la raíz del branch (convención del repositorio).
- `.gitignore` específico del proyecto para artefactos de MPLAB X.

### Cambiado
- Configuración de picos y packs de MPLAB X actualizada (DFP 1.7.380).

## [1.0.0] — 2026-04-03

### Agregado
- Driver base `EPD_Driver` (versión 2.0 interna del driver).
- Soporte de pantallas E-Paper Pervasive Displays de 1.54" a 4.37".
- Control por SPI nativo sin `plib.h`.
- Ejemplo de demostración en `main.cpp` con patrón de prueba.
- Funciones de verificación de pines (`test_pines_*`).

### Notas
- Proyecto originalmente orientado a PIC32MX795F512L y XC32 v5.00.

## [0.1.0] — Inicio del proyecto

### Agregado
- Boceto inicial del driver E-Paper y configuración de hardware.
