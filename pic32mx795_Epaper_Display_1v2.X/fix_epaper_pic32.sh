#!/bin/bash

# Script para corregir errores de enlace (undefined reference) en funciones gráficas
# Restaura EPD_Graphics.cpp con las definiciones completas y la corrección de visualización

set -e

echo "=== Corrigiendo errores de enlace en funciones gráficas ==="

# Verificar que estamos en el directorio correcto
if [ ! -f "EPD_Graphics.cpp" ] || [ ! -f "EPD_Graphics.h" ]; then
    echo "Error: No se encuentran EPD_Graphics.cpp o EPD_Graphics.h"
    exit 1
fi

# Hacer backup
cp EPD_Graphics.cpp EPD_Graphics.cpp.bak2
cp EPD_Graphics.h EPD_Graphics.h.bak2

# ============================================================================
# 1. REGENERAR EPD_Graphics.cpp con contenido completo y corregido
# ============================================================================
cat > EPD_Graphics.cpp << 'EOF'
/**
 * @file    EPD_Graphics.cpp
 * @brief   Implementación de funciones gráficas corregidas (espejo horizontal + LSB first)
 */

#include "EPD_Graphics.h"
#include "fonts.h"

/*==============================================================================
 * IMPLEMENTACIÓN DE FUNCIONES GRÁFICAS
 *============================================================================*/

void epd_clearBuffer(uint8_t* buffer, uint32_t size, uint8_t color) {
    memset(buffer, color, size);
}

void epd_drawPixel(uint8_t* buffer, uint16_t x, uint16_t y, uint8_t color) {
    // CORRECCIÓN: Espejo horizontal (como en Raspberry Pi)
    uint16_t mirroredX = EPD_WIDTH - 1 - x;
    
    if (mirroredX >= EPD_WIDTH || y >= EPD_HEIGHT) return;
    
    // Organización por columnas (mirroredX * height + y)
    uint32_t byteIndex = (mirroredX * EPD_HEIGHT + y) / 8;
    uint8_t bitMask = 0x01 << (y % 8);   // LSB first
    
    if (color == COLOR_BLACK) {
        buffer[byteIndex] |= bitMask;
    } else {
        buffer[byteIndex] &= ~bitMask;
    }
}

uint8_t epd_getPixel(const uint8_t* buffer, uint16_t x, uint16_t y) {
    uint16_t mirroredX = EPD_WIDTH - 1 - x;
    if (mirroredX >= EPD_WIDTH || y >= EPD_HEIGHT) return COLOR_WHITE;
    
    uint32_t byteIndex = (mirroredX * EPD_HEIGHT + y) / 8;
    uint8_t bitMask = 0x01 << (y % 8);
    
    return (buffer[byteIndex] & bitMask) ? COLOR_BLACK : COLOR_WHITE;
}

void epd_drawHLine(uint8_t* buffer, uint16_t x0, uint16_t x1, uint16_t y, uint8_t color) {
    if (y >= EPD_HEIGHT) return;
    
    if (x0 > x1) {
        uint16_t temp = x0;
        x0 = x1;
        x1 = temp;
    }
    
    if (x0 >= EPD_WIDTH) x0 = EPD_WIDTH - 1;
    if (x1 >= EPD_WIDTH) x1 = EPD_WIDTH - 1;
    
    for (uint16_t x = x0; x <= x1; x++) {
        epd_drawPixel(buffer, x, y, color);
    }
}

void epd_drawVLine(uint8_t* buffer, uint16_t x, uint16_t y0, uint16_t y1, uint8_t color) {
    if (x >= EPD_WIDTH) return;
    
    if (y0 > y1) {
        uint16_t temp = y0;
        y0 = y1;
        y1 = temp;
    }
    
    if (y0 >= EPD_HEIGHT) y0 = EPD_HEIGHT - 1;
    if (y1 >= EPD_HEIGHT) y1 = EPD_HEIGHT - 1;
    
    for (uint16_t y = y0; y <= y1; y++) {
        epd_drawPixel(buffer, x, y, color);
    }
}

void epd_drawRect(uint8_t* buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color, uint8_t fill) {
    if (fill) {
        for (uint16_t i = 0; i < h; i++) {
            epd_drawHLine(buffer, x, x + w - 1, y + i, color);
        }
    } else {
        epd_drawHLine(buffer, x, x + w - 1, y, color);
        epd_drawHLine(buffer, x, x + w - 1, y + h - 1, color);
        epd_drawVLine(buffer, x, y, y + h - 1, color);
        epd_drawVLine(buffer, x + w - 1, y, y + h - 1, color);
    }
}

void epd_drawChar(uint8_t* buffer, uint16_t x, uint16_t y, char c, uint8_t color, uint8_t bgColor) {
    if (c < 32 || c > 127) return;
    
    uint16_t fontIndex = (c - 32) * 8;
    
    for (uint8_t row = 0; row < 8; row++) {
        uint8_t fontByte = font[fontIndex + row];
        
        for (uint8_t col = 0; col < 8; col++) {
            uint8_t bit = (fontByte >> (7 - col)) & 0x01;
            
            if (bit) {
                epd_drawPixel(buffer, x + col, y + row, color);
            } else if (bgColor != COLOR_WHITE) {
                epd_drawPixel(buffer, x + col, y + row, bgColor);
            }
        }
    }
}

void epd_drawString(uint8_t* buffer, uint16_t x, uint16_t y, const char* str, uint8_t color, uint8_t bgColor) {
    uint16_t currentX = x;
    
    while (*str) {
        if (*str == '\n') {
            y += 8;
            currentX = x;
        } else if (*str == '\r') {
            currentX = x;
        } else {
            epd_drawChar(buffer, currentX, y, *str, color, bgColor);
            currentX += 6;  // 8 píxeles de ancho + 2 de espacio
        }
        str++;
    }
}

void epd_loadImage(uint8_t* buffer, const uint8_t* imageData, uint32_t size) {
    uint32_t copySize = (size < EPD_BUFFER_SIZE) ? size : EPD_BUFFER_SIZE;
    memcpy(buffer, imageData, copySize);
}

void epd_invertBuffer(uint8_t* buffer, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = ~buffer[i];
    }
}
EOF

# ============================================================================
# 2. ASEGURAR QUE EPD_Graphics.h TENGA LOS PROTOTIPOS CORRECTOS
# ============================================================================
# El archivo original ya los tiene, pero verificamos que no falte ninguno
if ! grep -q "epd_invertBuffer" EPD_Graphics.h; then
    echo "Añadiendo prototipo faltante epd_invertBuffer a EPD_Graphics.h"
    sed -i '/void epd_loadImage/i void epd_invertBuffer(uint8_t* buffer, uint32_t size);' EPD_Graphics.h
fi

# ============================================================================
# 3. LIMPIAR ARCHIVOS OBJETO ANTIGUOS Y RECONSTRUIR
# ============================================================================
echo "Limpiando archivos objeto previos..."
rm -f build/default/debug/EPD_Graphics.o

echo ""
echo "✅ Corrección completada."
echo "Ahora compila de nuevo tu proyecto en MPLAB X (Clean & Build)."
echo "Si aún hay errores, verifica que el archivo fonts.h esté presente y tenga la definición de 'font'."
EOF

# Script para corregir la orientación del texto en E-Paper (PIC32MX795)
# Permite probar diferentes modos de mapeo

set -e

echo "=== Configurando modos de prueba para orientación del texto ==="

# Verificar archivo
if [ ! -f "EPD_Graphics.cpp" ]; then
    echo "Error: No se encuentra EPD_Graphics.cpp"
    exit 1
fi

# Backup
cp EPD_Graphics.cpp EPD_Graphics.cpp.ori

# Crear nuevo EPD_Graphics.cpp con opciones de configuración
cat > EPD_Graphics.cpp << 'EOF'
/**
 * @file    EPD_Graphics.cpp
 * @brief   Implementación de funciones gráficas - MODO CONFIGURABLE
 * 
 * Para cambiar el modo, modifica la siguiente línea:
 *   #define MODO 1
 * 
 * Modos disponibles:
 *   1 - Espejo horizontal + LSB first (original Raspberry Pi)
 *   2 - Sin espejo + LSB first
 *   3 - Espejo horizontal + MSB first
 *   4 - Sin espejo + MSB first (configuración original del driver)
 *   5 - Espejo horizontal + LSB first + intercambio X/Y
 */

#include "EPD_Graphics.h"
#include "fonts.h"

// ==================== SELECCIONAR MODO AQUÍ ====================
#define MODO 1   // Cambia este número (1 a 5) y recompila
// ================================================================

/*==============================================================================
 * IMPLEMENTACIÓN DE FUNCIONES GRÁFICAS
 *============================================================================*/

void epd_clearBuffer(uint8_t* buffer, uint32_t size, uint8_t color) {
    memset(buffer, color, size);
}

#if MODO == 1
// Modo 1: Espejo horizontal + LSB first (como Raspberry Pi)
void epd_drawPixel(uint8_t* buffer, uint16_t x, uint16_t y, uint8_t color) {
    uint16_t mx = EPD_WIDTH - 1 - x;
    if (mx >= EPD_WIDTH || y >= EPD_HEIGHT) return;
    uint32_t byteIndex = (mx * EPD_HEIGHT + y) / 8;
    uint8_t bitMask = 0x01 << (y % 8);
    if (color == COLOR_BLACK) buffer[byteIndex] |= bitMask;
    else buffer[byteIndex] &= ~bitMask;
}
#elif MODO == 2
// Modo 2: Sin espejo + LSB first
void epd_drawPixel(uint8_t* buffer, uint16_t x, uint16_t y, uint8_t color) {
    if (x >= EPD_WIDTH || y >= EPD_HEIGHT) return;
    uint32_t byteIndex = (x * EPD_HEIGHT + y) / 8;
    uint8_t bitMask = 0x01 << (y % 8);
    if (color == COLOR_BLACK) buffer[byteIndex] |= bitMask;
    else buffer[byteIndex] &= ~bitMask;
}
#elif MODO == 3
// Modo 3: Espejo horizontal + MSB first
void epd_drawPixel(uint8_t* buffer, uint16_t x, uint16_t y, uint8_t color) {
    uint16_t mx = EPD_WIDTH - 1 - x;
    if (mx >= EPD_WIDTH || y >= EPD_HEIGHT) return;
    uint32_t byteIndex = (mx * EPD_HEIGHT + y) / 8;
    uint8_t bitMask = 0x80 >> (y % 8);
    if (color == COLOR_BLACK) buffer[byteIndex] |= bitMask;
    else buffer[byteIndex] &= ~bitMask;
}
#elif MODO == 4
// Modo 4: Sin espejo + MSB first (original)
void epd_drawPixel(uint8_t* buffer, uint16_t x, uint16_t y, uint8_t color) {
    if (x >= EPD_WIDTH || y >= EPD_HEIGHT) return;
    uint32_t byteIndex = (x * EPD_HEIGHT + y) / 8;
    uint8_t bitMask = 0x80 >> (y % 8);
    if (color == COLOR_BLACK) buffer[byteIndex] |= bitMask;
    else buffer[byteIndex] &= ~bitMask;
}
#elif MODO == 5
// Modo 5: Espejo horizontal + LSB first + intercambio X/Y (prueba extra)
void epd_drawPixel(uint8_t* buffer, uint16_t x, uint16_t y, uint8_t color) {
    uint16_t mx = EPD_WIDTH - 1 - y;   // intercambio
    uint16_t my = x;
    if (mx >= EPD_WIDTH || my >= EPD_HEIGHT) return;
    uint32_t byteIndex = (mx * EPD_HEIGHT + my) / 8;
    uint8_t bitMask = 0x01 << (my % 8);
    if (color == COLOR_BLACK) buffer[byteIndex] |= bitMask;
    else buffer[byteIndex] &= ~bitMask;
}
#else
#error "MODO no válido. Define MODO entre 1 y 5"
#endif

uint8_t epd_getPixel(const uint8_t* buffer, uint16_t x, uint16_t y) {
#if MODO == 1
    uint16_t mx = EPD_WIDTH - 1 - x;
    if (mx >= EPD_WIDTH || y >= EPD_HEIGHT) return COLOR_WHITE;
    uint32_t byteIndex = (mx * EPD_HEIGHT + y) / 8;
    uint8_t bitMask = 0x01 << (y % 8);
    return (buffer[byteIndex] & bitMask) ? COLOR_BLACK : COLOR_WHITE;
#elif MODO == 2
    if (x >= EPD_WIDTH || y >= EPD_HEIGHT) return COLOR_WHITE;
    uint32_t byteIndex = (x * EPD_HEIGHT + y) / 8;
    uint8_t bitMask = 0x01 << (y % 8);
    return (buffer[byteIndex] & bitMask) ? COLOR_BLACK : COLOR_WHITE;
#elif MODO == 3
    uint16_t mx = EPD_WIDTH - 1 - x;
    if (mx >= EPD_WIDTH || y >= EPD_HEIGHT) return COLOR_WHITE;
    uint32_t byteIndex = (mx * EPD_HEIGHT + y) / 8;
    uint8_t bitMask = 0x80 >> (y % 8);
    return (buffer[byteIndex] & bitMask) ? COLOR_BLACK : COLOR_WHITE;
#elif MODO == 4
    if (x >= EPD_WIDTH || y >= EPD_HEIGHT) return COLOR_WHITE;
    uint32_t byteIndex = (x * EPD_HEIGHT + y) / 8;
    uint8_t bitMask = 0x80 >> (y % 8);
    return (buffer[byteIndex] & bitMask) ? COLOR_BLACK : COLOR_WHITE;
#elif MODO == 5
    uint16_t mx = EPD_WIDTH - 1 - y;
    uint16_t my = x;
    if (mx >= EPD_WIDTH || my >= EPD_HEIGHT) return COLOR_WHITE;
    uint32_t byteIndex = (mx * EPD_HEIGHT + my) / 8;
    uint8_t bitMask = 0x01 << (my % 8);
    return (buffer[byteIndex] & bitMask) ? COLOR_BLACK : COLOR_WHITE;
#endif
}

// Resto de funciones (drawHLine, drawVLine, drawRect, drawChar, drawString, etc.)
// se mantienen igual porque llaman a epd_drawPixel

void epd_drawHLine(uint8_t* buffer, uint16_t x0, uint16_t x1, uint16_t y, uint8_t color) {
    if (y >= EPD_HEIGHT) return;
    if (x0 > x1) { uint16_t t = x0; x0 = x1; x1 = t; }
    if (x0 >= EPD_WIDTH) x0 = EPD_WIDTH - 1;
    if (x1 >= EPD_WIDTH) x1 = EPD_WIDTH - 1;
    for (uint16_t x = x0; x <= x1; x++) epd_drawPixel(buffer, x, y, color);
}

void epd_drawVLine(uint8_t* buffer, uint16_t x, uint16_t y0, uint16_t y1, uint8_t color) {
    if (x >= EPD_WIDTH) return;
    if (y0 > y1) { uint16_t t = y0; y0 = y1; y1 = t; }
    if (y0 >= EPD_HEIGHT) y0 = EPD_HEIGHT - 1;
    if (y1 >= EPD_HEIGHT) y1 = EPD_HEIGHT - 1;
    for (uint16_t y = y0; y <= y1; y++) epd_drawPixel(buffer, x, y, color);
}

void epd_drawRect(uint8_t* buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color, uint8_t fill) {
    if (fill) {
        for (uint16_t i = 0; i < h; i++) epd_drawHLine(buffer, x, x + w - 1, y + i, color);
    } else {
        epd_drawHLine(buffer, x, x + w - 1, y, color);
        epd_drawHLine(buffer, x, x + w - 1, y + h - 1, color);
        epd_drawVLine(buffer, x, y, y + h - 1, color);
        epd_drawVLine(buffer, x + w - 1, y, y + h - 1, color);
    }
}

void epd_drawChar(uint8_t* buffer, uint16_t x, uint16_t y, char c, uint8_t color, uint8_t bgColor) {
    if (c < 32 || c > 127) return;
    uint16_t fontIndex = (c - 32) * 8;
    for (uint8_t row = 0; row < 8; row++) {
        uint8_t fontByte = font[fontIndex + row];
        for (uint8_t col = 0; col < 8; col++) {
            uint8_t bit = (fontByte >> (7 - col)) & 0x01;
            if (bit) epd_drawPixel(buffer, x + col, y + row, color);
            else if (bgColor != COLOR_WHITE) epd_drawPixel(buffer, x + col, y + row, bgColor);
        }
    }
}

void epd_drawString(uint8_t* buffer, uint16_t x, uint16_t y, const char* str, uint8_t color, uint8_t bgColor) {
    uint16_t currentX = x;
    while (*str) {
        if (*str == '\n') { y += 8; currentX = x; }
        else if (*str == '\r') { currentX = x; }
        else {
            epd_drawChar(buffer, currentX, y, *str, color, bgColor);
            currentX += 6;
        }
        str++;
    }
}

void epd_loadImage(uint8_t* buffer, const uint8_t* imageData, uint32_t size) {
    uint32_t copySize = (size < EPD_BUFFER_SIZE) ? size : EPD_BUFFER_SIZE;
    memcpy(buffer, imageData, copySize);
}

void epd_invertBuffer(uint8_t* buffer, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) buffer[i] = ~buffer[i];
}
EOF

echo "✅ Archivo EPD_Graphics.cpp regenerado con modo configurable."
echo ""
echo "Instrucciones:"
echo "1. Abre EPD_Graphics.cpp y cambia el valor de #define MODO (1 a 5)."
echo "2. Compila y prueba cada modo hasta que el texto se vea correctamente."
echo "3. Modo actual: MODO 1 (espejo horizontal + LSB first)."
echo ""
echo "Sugerencia de prueba:"
echo "  - MODO 1: ya lo has probado (solo veías %). Prueba MODO 2, 3, 4, 5."
echo "  - Si ningún modo funciona, ajusta también SPI4CONbits.CKE en EPD_Driver.cpp"
echo "    (cambia entre 1 y 0)."
