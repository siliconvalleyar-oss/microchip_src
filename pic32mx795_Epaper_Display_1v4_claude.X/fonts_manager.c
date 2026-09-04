////////////////////////////////////////////////////////////////////////////////
//
//   filename        : fonts_manager.c
//   License         : GNU
//   Author          : Lio
//   Ported to       : PIC32MX795F512H (C puro, sin C++)
//   Compiler        : XC32 / MPLAB X
//
//   Nota: los arrays de fuentes (Font_One .. Font_Eight, font[])
//         se compilan exactamente igual que en la versión RPi.
//         Solo se cambia la interfaz de C++ a C.
//
////////////////////////////////////////////////////////////////////////////////

#include "fonts_manager.h"
#include "fonts.h"     // extern declarations: font[], Font_One ... Font_Eight
#include <xc.h>
// ─── Tamaño de cada carácter en bytes ────────────────────────────────────────
static uint8_t _charSize(FontType type) {
    switch (type) {
        case FONT_8x8:           return 8;
        case FONT_5x8:           return 5;
        case FONT_7x8_THICK:     return 7;
        case FONT_4x8_SEG:       return 4;
        case FONT_8x8_WIDE:      return 8;
        case FONT_3x8_TINY:      return 3;
        case FONT_7x8_HOMESPUN:  return 7;
        case FONT_16x32_BIGNUM:  return 64;
        case FONT_16x16_MEDNUM:  return 32;
        default:                 return 5;
    }
}

// ─── Puntero a los datos de la fuente ────────────────────────────────────────
static const void *_fontData(FontType type) {
    switch (type) {
        case FONT_8x8:           return font;
        case FONT_5x8:           return Font_One;
        case FONT_7x8_THICK:     return Font_Two;
        case FONT_4x8_SEG:       return Font_Three;
        case FONT_8x8_WIDE:      return Font_Four;
        case FONT_3x8_TINY:      return Font_Five;
        case FONT_7x8_HOMESPUN:  return Font_Six;
        case FONT_16x32_BIGNUM:  return Font_Seven;
        case FONT_16x16_MEDNUM:  return Font_Eight;
        default:                 return Font_One;
    }
}

// ─── Inicializar con fuente por defecto ───────────────────────────────────────
void FM_Init(FontManager_t *fm) {
    FM_SetFont(fm, FONT_5x8);
}

// ─── Cambiar fuente activa ────────────────────────────────────────────────────
void FM_SetFont(FontManager_t *fm, FontType type) {
    fm->current.type = type;
    fm->current.data = _fontData(type);

    switch (type) {
        case FONT_8x8:
            fm->current.width      = 8;
            fm->current.height     = 8;
            fm->current.start_char = 32;
            fm->current.end_char   = 127;
            fm->current.name       = "8x8 myc64_lower";
            break;
        case FONT_5x8:
            fm->current.width      = 5;
            fm->current.height     = 8;
            fm->current.start_char = 32;
            fm->current.end_char   = 127;
            fm->current.name       = "5x8 Standard";
            break;
        case FONT_7x8_THICK:
            fm->current.width      = 7;
            fm->current.height     = 8;
            fm->current.start_char = 32;
            fm->current.end_char   = 127;
            fm->current.name       = "7x8 Thick";
            break;
        case FONT_4x8_SEG:
            fm->current.width      = 4;
            fm->current.height     = 8;
            fm->current.start_char = 32;
            fm->current.end_char   = 127;
            fm->current.name       = "4x8 Seven Seg";
            break;
        case FONT_8x8_WIDE:
            fm->current.width      = 8;
            fm->current.height     = 8;
            fm->current.start_char = 32;
            fm->current.end_char   = 127;
            fm->current.name       = "8x8 Wide";
            break;
        case FONT_3x8_TINY:
            fm->current.width      = 3;
            fm->current.height     = 8;
            fm->current.start_char = 32;
            fm->current.end_char   = 127;
            fm->current.name       = "3x8 Tiny";
            break;
        case FONT_7x8_HOMESPUN:
            fm->current.width      = 7;
            fm->current.height     = 8;
            fm->current.start_char = 32;
            fm->current.end_char   = 127;
            fm->current.name       = "7x8 Homespun";
            break;
        case FONT_16x32_BIGNUM:
            fm->current.width      = 16;
            fm->current.height     = 32;
            fm->current.start_char = 48;   // '0'
            fm->current.end_char   = 58;   // ':' 
            fm->current.name       = "16x32 BigNum";
            break;
        case FONT_16x16_MEDNUM:
            fm->current.width      = 16;
            fm->current.height     = 16;
            fm->current.start_char = 48;
            fm->current.end_char   = 58;
            fm->current.name       = "16x16 MedNum";
            break;
        default:
            FM_SetFont(fm, FONT_5x8);
            break;
    }
}

// ─── Obtener bitmap de un carácter ───────────────────────────────────────────
const uint8_t *FM_GetCharBitmap(FontManager_t *fm, char c) {
    int index = (int)c - (int)fm->current.start_char;
    if (index < 0 || index > (int)(fm->current.end_char - fm->current.start_char))
        return 0;   // carácter fuera de rango → NULL

    uint8_t charSize = _charSize(fm->current.type);
    const uint8_t *fontData = (const uint8_t *)fm->current.data;

    // Fuentes de número grande: arrays 2D contiguos en memoria
    // Font_Seven[11][64]  → cada entrada ocupa 64 bytes
    // Font_Eight[11][32]  → cada entrada ocupa 32 bytes
    return &fontData[(size_t)index * charSize];
}

uint8_t FM_GetWidth (const FontManager_t *fm) { return fm->current.width;  }
uint8_t FM_GetHeight(const FontManager_t *fm) { return fm->current.height; }
