////////////////////////////////////////////////////////////////////////////////
//
//   filename        : epaper_display.c
//   License         : GNU
//   Author          : Lio
//   Ported to       : PIC32MX795F512H
//   Compiler        : XC32 / MPLAB X
//
////////////////////////////////////////////////////////////////////////////////

#include "epaper_display.h"
#include "fonts_manager.h"
#include <string.h>
#include <stdlib.h>   // abs()

// ─────────────────────────────────────────────────────────────────────────────
//  Inicialización
// ─────────────────────────────────────────────────────────────────────────────

void Display_Init(EpaperDisplay_t *d, uint32_t screen_type, const pins_t *board) {
    // Dimensiones
    switch (screen_type) {
        case eScreen_EPD_213: d->width = 212; d->height = 104; break;
        case eScreen_EPD_266: d->width = 296; d->height = 152; break;
        case eScreen_EPD_154: d->width = 200; d->height = 200; break;
        default:              d->width = 296; d->height = 152; break;
    }

    d->buf_size    = (d->width * d->height) / 8;
    d->cursor_x    = 0;
    d->cursor_y    = 0;
    d->transparent = true;
    d->screen_type = screen_type;

    // Inicializar driver (GPIO + SPI)
    EPD_Init(&d->driver, screen_type, board);

    // Limpiar buffer
    Display_Clear(d, true);
}

void Display_Begin(EpaperDisplay_t *d) {
    EPD_COG_initial(&d->driver);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Operaciones de buffer
// ─────────────────────────────────────────────────────────────────────────────

void Display_Clear(EpaperDisplay_t *d, bool white) {
    // 0x00 = blanco, 0xFF = negro (igual que versión RPi)
    memset(d->buffer, white ? 0x00 : 0xFF, (size_t)d->buf_size);
}

void Display_Update(EpaperDisplay_t *d) {
    EPD_globalUpdate(&d->driver, d->buffer, d->buffer);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Dibujo de píxel
//  Mismo esquema que epaper_display.cpp:
//   - espejo horizontal (mirroredX = width - 1 - x)
//   - organización por columnas: byteIndex = (mirroredX * height + y) / 8
// ─────────────────────────────────────────────────────────────────────────────

void Display_DrawPixel(EpaperDisplay_t *d, int x, int y, bool black) {
    int mx = d->width - 1 - x;           // espejo horizontal

    if (mx < 0 || mx >= d->width)  return;
    if (y  < 0 || y  >= d->height) return;

    int byteIndex = (mx * d->height + y) / 8;
    int bitIndex  = 7 - (y % 8);

    if (black) d->buffer[byteIndex] |=  (uint8_t)(1u << bitIndex);
    else       d->buffer[byteIndex] &= ~(uint8_t)(1u << bitIndex);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Línea (Bresenham)
// ─────────────────────────────────────────────────────────────────────────────

void Display_DrawLine(EpaperDisplay_t *d,
                      int x0, int y0, int x1, int y1, bool black)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        Display_DrawPixel(d, x0, y0, black);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Rectángulo
// ─────────────────────────────────────────────────────────────────────────────

void Display_DrawRect(EpaperDisplay_t *d,
                      int x, int y, int w, int h,
                      bool fill, bool black)
{
    if (fill) {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
                Display_DrawPixel(d, x + i, y + j, black);
    } else {
        Display_DrawLine(d, x,     y,     x + w, y,     black);
        Display_DrawLine(d, x + w, y,     x + w, y + h, black);
        Display_DrawLine(d, x + w, y + h, x,     y + h, black);
        Display_DrawLine(d, x,     y + h, x,     y,     black);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Texto
// ─────────────────────────────────────────────────────────────────────────────

static void _drawCharToBuffer(EpaperDisplay_t *d,
                               int x, int y, char c,
                               FontManager_t *fm, bool black)
{
    const uint8_t *bitmap = FM_GetCharBitmap(fm, c);
    if (!bitmap) return;

    int w = FM_GetWidth(fm);
    int h = FM_GetHeight(fm);

    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            uint8_t byte = bitmap[col];
            bool pixel = (byte >> row) & 0x01;

            if (pixel) {
                Display_DrawPixel(d, x + col, y + row, black);
            } else if (!d->transparent) {
                Display_DrawPixel(d, x + col, y + row, false);
            }
        }
    }
}

void Display_DrawChar(EpaperDisplay_t *d,
                      int x, int y, char c,
                      FontType font, bool black)
{
    FontManager_t fm;
    FM_SetFont(&fm, font);
    _drawCharToBuffer(d, x, y, c, &fm, black);
}

void Display_DrawString(EpaperDisplay_t *d,
                        int x, int y, const char *text,
                        FontType font, bool black)
{
    FontManager_t fm;
    FM_SetFont(&fm, font);
    int charWidth = FM_GetWidth(&fm);
    int spacing   = 1;
    int currentX  = x;

    for (; *text != '\0'; text++) {
        _drawCharToBuffer(d, currentX, y, *text, &fm, black);
        currentX += charWidth + spacing;
        if (currentX + charWidth > d->width) break;
    }
}

void Display_DrawCentered(EpaperDisplay_t *d,
                          int y, const char *text,
                          FontType font, bool black)
{
    int tw = Display_TextWidth(text, font);
    int x  = (d->width - tw) / 2;
    if (x < 0) x = 0;
    Display_DrawString(d, x, y, text, font, black);
}

int Display_TextWidth(const char *text, FontType font) {
    FontManager_t fm;
    FM_SetFont(&fm, font);
    int len = 0;
    while (text[len]) len++;
    return len * (FM_GetWidth(&fm) + 1) - 1;
}

int Display_TextHeight(FontType font) {
    FontManager_t fm;
    FM_SetFont(&fm, font);
    return FM_GetHeight(&fm);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Patrón de prueba
// ─────────────────────────────────────────────────────────────────────────────

void Display_TestPattern(EpaperDisplay_t *d) {
    Display_Clear(d, true);

    // Marco
    for (int x = 0; x < d->width;  x++) {
        Display_DrawPixel(d, x, 0,            true);
        Display_DrawPixel(d, x, d->height - 1, true);
    }
    for (int y = 0; y < d->height; y++) {
        Display_DrawPixel(d, 0,           y, true);
        Display_DrawPixel(d, d->width - 1, y, true);
    }

    // Diagonal
    int diag = d->width < d->height ? d->width : d->height;
    for (int i = 0; i < diag; i++)
        Display_DrawPixel(d, i, i, true);

    // Rectángulo central
    int cx = d->width  / 2;
    int cy = d->height / 2;
    Display_DrawRect(d, cx - 20, cy - 20, 40, 40, false, true);

    // Texto
    Display_DrawString(d, 10, 10, "TEST PIC32", FONT_5x8, true);

    Display_Update(d);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Funciones de imagen
//
//  El formato del array externo (custom_image_example.h y similares) es:
//    Organización por COLUMNAS — igual que el buffer interno del display.
//    - Columna 0: bytes[0..18]   → 19 bytes × 8 bits = 152 filas
//    - Columna 1: bytes[19..37]
//    - ...
//    - Columna 295: bytes[5605..5623]
//    Total: 296 × 19 = 5624 bytes
//
//  1 = negro, 0 = blanco.
//  Se copia directo sin transformación porque el buffer interno usa el mismo
//  layout que drawPixel() define en epaper_display.c.
// ─────────────────────────────────────────────────────────────────────────────

void Display_LoadImage(EpaperDisplay_t *d, const uint8_t *img) {
    if (!d || !img) return;
    memcpy(d->buffer, img, (size_t)d->buf_size);
}

void Display_ShowImage(EpaperDisplay_t *d, const uint8_t *img) {
    Display_LoadImage(d, img);
    EPD_globalUpdate(&d->driver, d->buffer, d->buffer);
}

void Display_ShowImageFast(EpaperDisplay_t *d, const uint8_t *img) {
    Display_LoadImage(d, img);
    Display_Update(d);
}

void Display_OverlayImage(EpaperDisplay_t *d, const uint8_t *img) {
    if (!d || !img) return;
    for (int i = 0; i < d->buf_size; i++)
        d->buffer[i] |= img[i];
}
