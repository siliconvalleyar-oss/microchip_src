////////////////////////////////////////////////////////////////////////////////
//
//   filename        : epaper_refresh.h
//   License         : GNU
//   Author          : Lio / PIC32MX795F512H port
//
//   Modos de refresco para e-paper 2.66" (UC8151 / IL0373)
//
//   ┌──────────────────┬────────────┬───────────┬─────────────────────────────┐
//   │ Función          │ Tiempo     │ Parpadeo  │ Uso                         │
//   ├──────────────────┼────────────┼───────────┼─────────────────────────────┤
//   │ RefreshFull      │ ~2 s       │ múltiple  │ primera imagen, limpieza    │
//   │ RefreshFast      │ ~300 ms    │ ninguno   │ update frecuente completo   │
//   │ RefreshZone      │ ~300 ms    │ ninguno   │ solo una zona (ej. reloj)   │
//   └──────────────────┴────────────┴───────────┴─────────────────────────────┘
//
//   IMPORTANTE — regla de ghosting:
//     Cada PARTIAL_REFRESH_CYCLE actualizaciones parciales se fuerza
//     automáticamente un RefreshFull para limpiar residuos.
//     Ajustar la constante según el caso de uso.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef EPAPER_REFRESH_H
#define EPAPER_REFRESH_H

#include <stdint.h>
#include <stdbool.h>
#include "epaper_display.h"

#define PARTIAL_REFRESH_CYCLE  10   // full auto cada N parciales

typedef struct {
    int x, y, w, h;
} RefreshZone_t;

// ─── API ─────────────────────────────────────────────────────────────────────
void Display_RefreshFull      (EpaperDisplay_t *d);
void Display_RefreshFast      (EpaperDisplay_t *d);
void Display_RefreshZone      (EpaperDisplay_t *d, int x, int y, int w, int h);
void Display_RefreshZoneRect  (EpaperDisplay_t *d, const RefreshZone_t *z);
void Display_ForceFullRefresh (EpaperDisplay_t *d);
int  Display_PartialCyclesLeft(void);

#endif
