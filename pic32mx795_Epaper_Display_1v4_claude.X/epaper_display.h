////////////////////////////////////////////////////////////////////////////////
//
//   filename        : epaper_display.h
//   License         : GNU
//   Author          : Lio
//   Ported to       : PIC32MX795F512H
//   Compiler        : XC32 / MPLAB X
//
//   Alto nivel: buffer de imagen + funciones de dibujo + texto.
//   Equivalente a la clase EpaperDisplay del código C++ para RPi.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "epaper.h"
#include "fonts_manager.h"   // FontType, FontManager_t

// ─── Dimensiones por defecto (2.66") ─────────────────────────────────────────
#define DISPLAY_WIDTH   296
#define DISPLAY_HEIGHT  152
#define DISPLAY_BUF_SIZE  (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)

// ─── Estructura del display de alto nivel ────────────────────────────────────
typedef struct {
    uint8_t       buffer[DISPLAY_BUF_SIZE];
    int           width;
    int           height;
    int           buf_size;
    int           cursor_x;
    int           cursor_y;
    bool          transparent;
    uint32_t      screen_type;
    EPD_Driver_t  driver;
} EpaperDisplay_t;

// ─── Inicialización ───────────────────────────────────────────────────────────
void Display_Init     (EpaperDisplay_t *d,
                       uint32_t screen_type,
                       const pins_t *board);
void Display_Begin    (EpaperDisplay_t *d);   // Llama a EPD_COG_initial

// ─── Operaciones básicas ──────────────────────────────────────────────────────
void Display_Clear    (EpaperDisplay_t *d, bool white);
void Display_Update   (EpaperDisplay_t *d);

// ─── Dibujo ──────────────────────────────────────────────────────────────────
void Display_DrawPixel     (EpaperDisplay_t *d, int x, int y, bool black);
void Display_DrawLine      (EpaperDisplay_t *d,
                            int x0, int y0, int x1, int y1, bool black);
void Display_DrawRect      (EpaperDisplay_t *d,
                            int x, int y, int w, int h,
                            bool fill, bool black);

// ─── Texto ────────────────────────────────────────────────────────────────────
void Display_DrawChar      (EpaperDisplay_t *d,
                            int x, int y, char c,
                            FontType font, bool black);
void Display_DrawString    (EpaperDisplay_t *d,
                            int x, int y, const char *text,
                            FontType font, bool black);
void Display_DrawCentered  (EpaperDisplay_t *d,
                            int y, const char *text,
                            FontType font, bool black);

// ─── Métricas de texto ────────────────────────────────────────────────────────
int  Display_TextWidth     (const char *text, FontType font);
int  Display_TextHeight    (FontType font);

// ─── Patrón de prueba ────────────────────────────────────────────────────────
void Display_TestPattern   (EpaperDisplay_t *d);

// ─── Imagen completa desde buffer externo ────────────────────────────────────

/**
 * Copia una imagen completa (5624 bytes para 296x152) al buffer interno
 * y la muestra en pantalla con full refresh.
 *
 * El array de imagen debe estar en el formato nativo del display:
 *   - Organización por COLUMNAS: cada columna de 152px ocupa 19 bytes
 *   - bit 7 del byte 0 = píxel (col 0, fila 0), bit más significativo = arriba
 *   - 1 = negro, 0 = blanco
 *   - Tamaño total: 296 columnas × 19 bytes/col = 5624 bytes
 *
 * @param d     Display ya inicializado y con COG activo (Display_Begin llamado)
 * @param img   Puntero al array de imagen (const uint8_t[], tamaño DISPLAY_BUF_SIZE)
 */
void Display_ShowImage     (EpaperDisplay_t *d, const uint8_t *img);

/**
 * Igual que Display_ShowImage pero usa fast refresh (sin parpadeo).
 * Requiere haber hecho al menos un Display_ShowImage o Display_RefreshFull antes.
 */
void Display_ShowImageFast (EpaperDisplay_t *d, const uint8_t *img);

/**
 * Copia la imagen al buffer interno sin mostrarla todavía.
 * Útil para mezclar imagen + texto antes de actualizar.
 * Llamar Display_Update() o Display_RefreshFull() después.
 *
 * @param d     Display
 * @param img   Array de imagen
 */
void Display_LoadImage     (EpaperDisplay_t *d, const uint8_t *img);

/**
 * Mezcla una imagen sobre el buffer actual (OR lógico bit a bit).
 * Permite superponer texto o gráficos sobre una imagen de fondo.
 *
 * @param d     Display
 * @param img   Array de imagen a superponer
 */
void Display_OverlayImage  (EpaperDisplay_t *d, const uint8_t *img);
