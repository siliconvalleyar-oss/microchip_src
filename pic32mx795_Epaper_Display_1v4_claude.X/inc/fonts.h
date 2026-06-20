////////////////////////////////////////////////////////////////////////////////
//
//   filename        : fonts.h
//   License         : GNU
//   Author          : Lio
//   Ported to       : PIC32MX795F512H
//   Compiler        : XC32 v5.00  (compilado como C++ porque fonts.cpp
//                     contiene reinterpret_cast)
//
////////////////////////////////////////////////////////////////////////////////

#ifndef FONTS_H
#define FONTS_H

#include <stdint.h>   // uint8_t  (XC32 no tiene <cstdint> en modo C)

#ifdef __cplusplus
extern "C" {
#endif

// ─── Arrays de datos de fuentes ───────────────────────────────────────────────
extern uint8_t              font[1024];          // 8x8  myc64_lower
extern const unsigned char  Font_One[];          // 5x8  Standard ASCII
extern const unsigned char  Font_Two[];          // 7x8  Thick
extern const unsigned char  Font_Three[];        // 4x8  Seven-Segment
extern const unsigned char  Font_Four[];         // 8x8  Wide
extern const unsigned char  Font_Five[];         // 3x8  Tiny
extern const unsigned char  Font_Six[];          // 7x8  Homespun
extern const uint8_t        Font_Seven[11][64];  // 16x32 BigNum  (0-9 + :)
extern const uint8_t        Font_Eight[11][32];  // 16x16 MedNum  (0-9 + :)

// ─── Punteros de acceso rápido ────────────────────────────────────────────────
extern const unsigned char *pFontDefaultptr;     // → Font_One
extern const unsigned char *pFontThickptr;       // → Font_Two
extern const unsigned char *pFontSevenSegptr;    // → Font_Three
extern const unsigned char *pFontWideptr;        // → Font_Four
extern const unsigned char *pFontTinyptr;        // → Font_Five
extern const unsigned char *pFontHomeSpunptr;    // → Font_Six
extern const uint8_t       *pFontBigNumptr;      // → Font_Seven (cast plano)
extern const uint8_t       *pFontMedNumptr;      // → Font_Eight (cast plano)

#ifdef __cplusplus
}
#endif

#endif // FONTS_H
