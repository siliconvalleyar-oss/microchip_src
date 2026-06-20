////////////////////////////////////////////////////////////////////////////////
//
//   filename        : main.c
//   Hardware        : PIC32MX795F512H - 64 pines
//   Compiler        : XC32 / MPLAB X
//
//   Secuencia:
//     1. Muestra imagen de presentación (customPattern, pantalla completa)
//     2. Limpia la pantalla a blanco
//     3. Muestra la hora centrada en negro
//     4. Actualiza cada segundo con zone refresh (sin parpadeo)
//
//   Zona del reloj:
//     - Hora grande  (FONT_7x8_THICK) en el centro vertical
//     - Fecha/estado (FONT_5x8)       debajo, opcional
//
////////////////////////////////////////////////////////////////////////////////

// ─── Fuses ───────────────────────────────────────────────────────────────────
#pragma config FNOSC    = PRIPLL
#pragma config POSCMOD  = HS
#pragma config FPLLIDIV = DIV_2
#pragma config FPLLMUL  = MUL_20
#pragma config FPLLODIV = DIV_1
#pragma config FPBDIV   = DIV_1
#pragma config FWDTEN   = OFF
#pragma config ICESEL   = ICS_PGx2
#pragma config BWP      = OFF
#pragma config CP       = OFF

#include <xc.h>
#include "boards.h"
#include "epaper_display.h"
#include "epaper_refresh.h"
#include "custom_image_example.h"   // const uint8_t customPattern[5624]

// ─────────────────────────────────────────────────────────────────────────────
//  Configuración de la zona del reloj
//
//  El display es 296 píxeles de alto × 152 de ancho (rotado: ancho=296, alto=152)
//  FONT_7x8_THICK: ancho=7px, alto=8px → "HH:MM:SS" = 8 chars × (7+1) = 64px
//
//  Zona: fila central ± margen, ancho completo
// ─────────────────────────────────────────────────────────────────────────────
#define CLOCK_FONT      FONT_7x8_THICK
#define CLOCK_CHAR_W    8           // 7px + 1px espacio
#define CLOCK_STR_LEN   8           // "HH:MM:SS"
#define CLOCK_TEXT_W    (CLOCK_CHAR_W * CLOCK_STR_LEN)   // 64 px

// Zona con margen generoso para cubrir el texto completamente
#define ZONE_X          0           // desde el borde izquierdo
#define ZONE_W          152         // ancho total de la pantalla
#define ZONE_H          24          // alto: fuente 8px + 8px de margen

// La zona se posiciona en el centro vertical de la pantalla
// display.height = 152, queremos centrar verticalmente:
// centro_y = 152/2 = 76 → zona desde 76 - ZONE_H/2 = 64
#define ZONE_Y          64

// ─────────────────────────────────────────────────────────────────────────────
//  Posición del texto dentro de la zona
// ─────────────────────────────────────────────────────────────────────────────
#define TEXT_Y          (ZONE_Y + 8)    // 8px de margen superior dentro de la zona
// TEXT_X se calcula dinámicamente para centrar

// ─────────────────────────────────────────────────────────────────────────────
//  Tiempo simulado — REEMPLAZAR con driver RTC real (DS1307, MCP7940, etc.)
// ─────────────────────────────────────────────────────────────────────────────
static uint8_t g_hh = 10;
static uint8_t g_mm = 30;
static uint8_t g_ss =  0;

static void clock_tick(void) {
    if (++g_ss >= 60) {
        g_ss = 0;
        if (++g_mm >= 60) {
            g_mm = 0;
            if (++g_hh >= 24) g_hh = 0;
        }
    }
}

// Construye el string "HH:MM:SS\0"
static void clock_str(char *buf) {
    buf[0] = '0' + g_hh / 10;
    buf[1] = '0' + g_hh % 10;
    buf[2] = ':';
    buf[3] = '0' + g_mm / 10;
    buf[4] = '0' + g_mm % 10;
    buf[5] = ':';
    buf[6] = '0' + g_ss / 10;
    buf[7] = '0' + g_ss % 10;
    buf[8] = '\0';
}

// ─────────────────────────────────────────────────────────────────────────────
//  Display global
// ─────────────────────────────────────────────────────────────────────────────
static EpaperDisplay_t g_display;

// ─────────────────────────────────────────────────────────────────────────────
int main(void) {

    DDPCONbits.JTAGEN = 0;   // liberar RB10-RB13

    // ── Inicializar display ───────────────────────────────────────────────────
    Display_Init (&g_display, eScreen_EPD_266, &boardPIC32MX795);
    Display_Begin(&g_display);

    // ════════════════════════════════════════════════════════════════════════
    //  PASO 1 — Imagen de presentación (pantalla completa, full refresh)
    // ════════════════════════════════════════════════════════════════════════
    Display_ShowImage(&g_display, customPattern);
    // Esperar que el usuario vea la imagen ~3 segundos
    HAL_Delay_ms(3000);

    // ════════════════════════════════════════════════════════════════════════
    //  PASO 2 — Limpiar pantalla a blanco (full refresh)
    //  Esto inicializa el frame anterior guardado internamente,
    //  necesario para que RefreshZone funcione correctamente después.
    // ════════════════════════════════════════════════════════════════════════
    Display_Clear(&g_display, true);      // buffer = todo blanco
    Display_RefreshFull(&g_display);      // mostrar blanco + guardar frame
    HAL_Delay_ms(500);

    // ════════════════════════════════════════════════════════════════════════
    //  PASO 3 — Primer dibujo del reloj (full refresh)
    //  Se dibuja una vez completo para tener el fondo correcto.
    // ════════════════════════════════════════════════════════════════════════
    {
        char tbuf[12];
        clock_str(tbuf);

        // Calcular X para centrar el texto horizontalmente
        int text_x = (g_display.width - CLOCK_TEXT_W) / 2;

        Display_Clear(&g_display, true);
        Display_DrawString(&g_display, text_x, TEXT_Y, tbuf, CLOCK_FONT, true);
        Display_RefreshFull(&g_display);
    }

    // ════════════════════════════════════════════════════════════════════════
    //  PASO 4 — Bucle del reloj
    //
    //  Cada segundo:
    //    a) Avanzar el tiempo
    //    b) Borrar solo la zona del reloj en el buffer (rect blanco)
    //    c) Dibujar el nuevo texto en el buffer
    //    d) Refrescar solo esa zona → sin parpadeo, ~300 ms
    //
    //  Cada PARTIAL_REFRESH_CYCLE (definido en epaper_refresh.h = 10)
    //  se ejecuta automáticamente un full refresh para limpiar ghosting.
    // ════════════════════════════════════════════════════════════════════════
    char tbuf[12];
    int  text_x = (g_display.width - CLOCK_TEXT_W) / 2;

    while (1) {

        // a) Avanzar tiempo
        clock_tick();
        clock_str(tbuf);

        // b) Borrar zona del reloj en el buffer (poner blanco)
        Display_DrawRect(&g_display,
                         ZONE_X, ZONE_Y,
                         ZONE_W, ZONE_H,
                         true,   // fill
                         false);  // blanco

        // c) Dibujar nuevo tiempo en negro
        Display_DrawString(&g_display,
                           text_x, TEXT_Y,
                           tbuf, CLOCK_FONT, true);

        // d) Refrescar solo la zona del reloj
        Display_RefreshZone(&g_display,
                            ZONE_X, ZONE_Y,
                            ZONE_W, ZONE_H);

        // Esperar 1 segundo
        // REEMPLAZAR con timer de hardware o RTC para precisión real
        HAL_Delay_ms(1000);
    }

    // Nunca llega aquí
    return 0;
}
