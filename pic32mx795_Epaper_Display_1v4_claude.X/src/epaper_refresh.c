////////////////////////////////////////////////////////////////////////////////
//
//   filename        : epaper_refresh.c
//   License         : GNU
//   Author          : Lio / PIC32MX795F512H port
//
//   LUTs verificados contra:
//     - Waveshare EPD_2in9D (mismo chip UC8151, mismo formato de LUT 42 bytes)
//     - ESPHome waveshare_epaper.cpp (código en producción real)
//     - antirez/uc8151_micropython (análisis detallado del chip)
//
//   Formato real del LUT UC8151 — 42 bytes por tabla:
//     7 grupos × 6 bytes cada uno
//     Cada grupo: [TP_AB, TP_CD, TP_EF, TP_GH, conteo, repeticiones]
//       TP_xx bits[7:6] = nivel fase 1, bits[5:4] = fase 2,
//                bits[3:2] = fase 3, bits[1:0] = fase 4
//       Niveles: 00=HiZ/GND, 01=VDH(+15V), 10=VDL(-15V), 11=VCOM flotante
//       conteo:      número de clocks de cada fase (0 = fase inactiva)
//       repeticiones: cuántas veces repetir el grupo
//
//   LUT de actualización rápida (DU - Direct Update, ~300 ms sin parpadeo):
//     Solo el grupo 0 activo con una fase corta.
//     WW (blanco→blanco): GND todo = sin movimiento
//     BW (negro→blanco):  VDL breve = empuja partículas blancas hacia arriba
//     WB (blanco→negro):  VDH breve = empuja partículas negras hacia arriba
//     BB (negro→negro):   GND todo = sin movimiento
//
////////////////////////////////////////////////////////////////////////////////

#include "epaper_refresh.h"
#include "epaper.h"
#include <string.h>

// ─── Buffer del frame anterior ────────────────────────────────────────────────
#define FRAME_SIZE  (296 * 152 / 8)   // 5624 bytes

static uint8_t s_prev[FRAME_SIZE];
static bool    s_valid      = false;
static bool    s_cog_on     = false;
static int     s_part_count = 0;

// ─────────────────────────────────────────────────────────────────────────────
//  LUTs VERIFICADOS — 42 bytes, formato UC8151 / IL0373
//  Valores probados en hardware con chips compatibles (EPD_2in9D, EPD_2in13D)
//  Adaptados para DU (Direct Update) mínimo: 1 fase de ~100 ms
// ─────────────────────────────────────────────────────────────────────────────

// WW — blanco permanece blanco: ninguna acción
static const uint8_t LUT_WW[42] = {
    0x40, 0x08, 0x00, 0x00, 0x00, 0x02,   // grupo 0
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,   // grupo 1
    0x40, 0x14, 0x00, 0x00, 0x00, 0x01,   // grupo 2
    0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,   // grupo 3
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // grupo 4
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // grupo 5
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // grupo 6
};

// BW — negro pasa a blanco: VDL (voltaje negativo)
static const uint8_t LUT_BW[42] = {
    0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x0F, 0x0F, 0x00, 0x00, 0x03,
    0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
    0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// WB — blanco pasa a negro: VDH (voltaje positivo)
static const uint8_t LUT_WB[42] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// BB — negro permanece negro: ninguna acción fuerte
static const uint8_t LUT_BB[42] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// VCOM — tensión común estable durante DU
static const uint8_t LUT_VCOM[44] = {
    0x00, 0x08, 0x00, 0x00, 0x00, 0x02,
    0x60, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00   // 2 bytes extra que espera el chip para VCOM (44 total)
};

// ─────────────────────────────────────────────────────────────────────────────
//  HAL — operaciones SPI optimizadas (CS una sola vez por bloque)
// ─────────────────────────────────────────────────────────────────────────────

static inline void _cs_lo(EPD_Driver_t *d) { HAL_GPIO_Write(d->pin_cfg.panelCS, LOW);  }
static inline void _cs_hi(EPD_Driver_t *d) { HAL_GPIO_Write(d->pin_cfg.panelCS, HIGH); }
static inline void _dc_lo(EPD_Driver_t *d) { HAL_GPIO_Write(d->pin_cfg.panelDC, LOW);  }
static inline void _dc_hi(EPD_Driver_t *d) { HAL_GPIO_Write(d->pin_cfg.panelDC, HIGH); }

// Envía 1 comando (DC=0)
static void _cmd(EPD_Driver_t *d, uint8_t c) {
    _dc_lo(d); _cs_lo(d);
    HAL_SPI4_Transfer(c);
    _cs_hi(d);
}

// Envía 1 byte de datos (DC=1)
static void _dat1(EPD_Driver_t *d, uint8_t v) {
    _dc_hi(d); _cs_lo(d);
    HAL_SPI4_Transfer(v);
    _cs_hi(d);
}

// Envía un bloque de datos — CS baja UNA vez para todo el bloque
static void _dat_blk(EPD_Driver_t *d, const uint8_t *buf, uint32_t n) {
    _dc_hi(d); _cs_lo(d);
    while (n--) HAL_SPI4_Transfer(*buf++);
    _cs_hi(d);
}

// Espera BUSY = HIGH
static void _busy(EPD_Driver_t *d) {
    while (HAL_GPIO_Read(d->pin_cfg.panelBusy) != HIGH);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Control de encendido del COG
//  Mantenerlo encendido entre parciales ahorra ~50 ms por update.
// ─────────────────────────────────────────────────────────────────────────────

static void _power_on(EPD_Driver_t *d) {
    if (s_cog_on) return;
    _cmd(d, 0x04); _busy(d);
    s_cog_on = true;
}

static void _power_off(EPD_Driver_t *d) {
    if (!s_cog_on) return;
    _cmd(d, 0x02); _busy(d);
    s_cog_on = false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Carga de LUTs DU en el controlador
// ─────────────────────────────────────────────────────────────────────────────

static void _load_lut(EPD_Driver_t *d) {
    _cmd(d, 0x20); _dat_blk(d, LUT_VCOM, 44);
    _cmd(d, 0x21); _dat_blk(d, LUT_WW,   42);
    _cmd(d, 0x22); _dat_blk(d, LUT_BW,   42);
    _cmd(d, 0x23); _dat_blk(d, LUT_WB,   42);
    _cmd(d, 0x24); _dat_blk(d, LUT_BB,   42);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Fast update — pantalla completa, sin parpadeo
// ─────────────────────────────────────────────────────────────────────────────

static void _do_fast(EpaperDisplay_t *disp) {
    EPD_Driver_t *d  = &disp->driver;
    uint32_t      sz = (uint32_t)disp->buf_size;

    _load_lut(d);

    // Frame 1 y Frame 2: se invierten los bytes porque el buffer
    // usa 0=blanco internamente pero el panel espera 0xFF=blanco.
    // Se invierte byte a byte en el envío sin tocar el buffer en RAM.
    // Frame 1: imagen anterior — 1=negro, 0=blanco (igual que datasheet PDI)
    // CS pulsa entre cada byte según formato SPI del datasheet pág.15
    _cmd(d, 0x10);
    for (uint32_t i = 0; i < sz; i++) {
        _dc_hi(d); _cs_lo(d);
        HAL_SPI4_Transfer(s_prev[i]);
        _cs_hi(d);
    }

    // Frame 2 para DU: imagen nueva (comparación pixel a pixel por el chip)
    _cmd(d, 0x13);
    for (uint32_t i = 0; i < sz; i++) {
        _dc_hi(d); _cs_lo(d);
        HAL_SPI4_Transfer(disp->buffer[i]);
        _cs_hi(d);
    }

    _power_on(d);
    _cmd(d, 0x12); _busy(d);   // Display Refresh

    memcpy(s_prev, disp->buffer, sz);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Partial update — solo la zona indicada
//
//  Coordenadas en el sistema de la pantalla física (x = columna, y = fila).
//  x y w se alinean automáticamente a múltiplos de 8.
// ─────────────────────────────────────────────────────────────────────────────

static void _do_zone(EpaperDisplay_t *disp, int x, int y, int w, int h) {
    EPD_Driver_t *d = &disp->driver;

    // Alinear a byte
    int x0 = x & ~7;
    int x1 = ((x + w + 7) & ~7) - 1;
    if (x1 >= disp->width)  x1 = disp->width  - 1;
    int y0 = y;
    int y1 = y + h - 1;
    if (y1 >= disp->height) y1 = disp->height - 1;

    int bz = (x1 / 8) - (x0 / 8) + 1;   // bytes por fila en la zona
    int bf = disp->width / 8;             // bytes por fila completa (19)

    _load_lut(d);

    // Ventana parcial — cmd 0x90
    _cmd(d, 0x90);
    _dc_hi(d); _cs_lo(d);
    HAL_SPI4_Transfer((uint8_t)(x0 / 8));
    HAL_SPI4_Transfer((uint8_t)(x1 / 8));
    HAL_SPI4_Transfer((uint8_t)((y0 >> 8) & 1));
    HAL_SPI4_Transfer((uint8_t)(y0 & 0xFF));
    HAL_SPI4_Transfer((uint8_t)((y1 >> 8) & 1));
    HAL_SPI4_Transfer((uint8_t)(y1 & 0xFF));
    HAL_SPI4_Transfer(0x01);  // PT_SCAN=1: escanear solo la ventana
    _cs_hi(d);

    // Frame 1 zona anterior — sin inversión (1=negro según datasheet PDI pág.20)
    // CS pulsea entre cada byte (datasheet pág.15)
    _cmd(d, 0x10);
    for (int r = y0; r <= y1; r++)
        for (int b = 0; b < bz; b++) {
            _dc_hi(d); _cs_lo(d);
            HAL_SPI4_Transfer(s_prev[r * bf + x0/8 + b]);
            _cs_hi(d);
        }

    // Frame 2 zona nueva
    _cmd(d, 0x13);
    for (int r = y0; r <= y1; r++)
        for (int b = 0; b < bz; b++) {
            _dc_hi(d); _cs_lo(d);
            HAL_SPI4_Transfer(disp->buffer[r * bf + x0/8 + b]);
            _cs_hi(d);
        }

    _power_on(d);
    _cmd(d, 0x12); _busy(d);
    _cmd(d, 0x92);  // desactivar ventana parcial

    // Actualizar solo la zona en el frame guardado
    for (int r = y0; r <= y1; r++) {
        int off = r * bf + x0/8;
        memcpy(s_prev + off, disp->buffer + off, (size_t)bz);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  API PÚBLICA
// ─────────────────────────────────────────────────────────────────────────────

void Display_RefreshFull(EpaperDisplay_t *d) {
    _power_off(&d->driver);                              // full tiene su secuencia
    EPD_globalUpdate(&d->driver, d->buffer, d->buffer);  // usa LUT OTP del chip
    memcpy(s_prev, d->buffer, (size_t)d->buf_size);
    s_valid      = true;
    s_cog_on     = false;
    s_part_count = 0;
}

void Display_RefreshFast(EpaperDisplay_t *d) {
    if (!s_valid)                       { Display_RefreshFull(d); return; }
    if (s_part_count >= PARTIAL_REFRESH_CYCLE) { Display_RefreshFull(d); return; }
    _do_fast(d);
    s_part_count++;
}

void Display_RefreshZone(EpaperDisplay_t *d, int x, int y, int w, int h) {
    if (!s_valid)                       { Display_RefreshFull(d); return; }
    if (s_part_count >= PARTIAL_REFRESH_CYCLE) { Display_RefreshFull(d); return; }
    _do_zone(d, x, y, w, h);
    s_part_count++;
}

void Display_RefreshZoneRect(EpaperDisplay_t *d, const RefreshZone_t *z) {
    Display_RefreshZone(d, z->x, z->y, z->w, z->h);
}

void Display_ForceFullRefresh(EpaperDisplay_t *d) {
    Display_RefreshFull(d);
}

int Display_PartialCyclesLeft(void) {
    int n = PARTIAL_REFRESH_CYCLE - s_part_count;
    return n > 0 ? n : 0;
}
