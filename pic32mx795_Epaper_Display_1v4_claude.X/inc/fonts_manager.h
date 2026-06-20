////////////////////////////////////////////////////////////////////////////////
//
//   filename        : fonts_manager.h
//   License         : GNU
//   Author          : Lio
//   Ported to       : PIC32MX795F512H (C puro, sin C++)
//   Compiler        : XC32 / MPLAB X
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>

// ─── Tipos de fuente (idénticos al enum C++ original) ─────────────────────────
typedef enum {
    FONT_8x8          = 0,
    FONT_5x8,
    FONT_7x8_THICK,
    FONT_4x8_SEG,
    FONT_8x8_WIDE,
    FONT_3x8_TINY,
    FONT_7x8_HOMESPUN,
    FONT_16x32_BIGNUM,
    FONT_16x16_MEDNUM
} FontType;

// ─── Metadatos de una fuente ──────────────────────────────────────────────────
typedef struct {
    FontType       type;
    const void    *data;
    uint8_t        width;
    uint8_t        height;
    uint8_t        start_char;
    uint8_t        end_char;
    const char    *name;
} FontInfo_t;

// ─── Estado del gestor de fuentes ────────────────────────────────────────────
typedef struct {
    FontInfo_t current;
} FontManager_t;

// ─── API ──────────────────────────────────────────────────────────────────────
void           FM_Init        (FontManager_t *fm);
void           FM_SetFont     (FontManager_t *fm, FontType type);
const uint8_t *FM_GetCharBitmap(FontManager_t *fm, char c);
uint8_t        FM_GetWidth    (const FontManager_t *fm);
uint8_t        FM_GetHeight   (const FontManager_t *fm);
