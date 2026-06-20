////////////////////////////////////////////////////////////////////////////////
//
//   filename        : boards.h
//   License         : GNU
//   Author          : Lio
//   Ported to       : PIC32MX795F512H - 64 pines
//   Compiler        : XC32 / MPLAB X
//
//   Configuración de pines del hardware:
//
//   Señal       Pin PIC32   Puerto/Bit   Función SPI
//   ─────────── ─────────── ──────────── ──────────────
//   Panel_CS    pin  ?      RB3          CS manual (ECSM)
//   Flash_CS    pin  ?      RB13         CS manual (FCSM)
//   MOSI        pin  ?      RF5          SPI4 SDO4
//   MISO        pin  ?      RF4          SPI4 SDI4
//   RST         pin  ?      RB2          GPIO salida
//   D/C         pin  ?      RB1          GPIO salida
//   BUSY        pin  ?      RB0          GPIO entrada
//   SCK         pin  ?      RB14         SPI4 SCK4
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "epaper.h"

// ─── Configuración única para PIC32MX795F512H ────────────────────────────────
// La variable boardPIC32MX795 ya está definida en epaper.c como
//   const pins_t boardPIC32MX795 = { ... };
// Aquí solo exponemos la declaración externa para que otros módulos la usen.

extern const pins_t boardPIC32MX795;

// Alias para mantener compatibilidad con el estilo de la versión RPi
#define BOARD_DEFAULT  boardPIC32MX795
