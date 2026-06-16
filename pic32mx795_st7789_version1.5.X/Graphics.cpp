#include "Graphics.h"
#include "HardwareProfile.h"
#include <stdlib.h>   // abs
#include <stdio.h> 
// ============================================================
//  PRIMITIVAS
// ============================================================

void Graphics::fill_screen(uint16_t color) {
    set_window(0, 0, TFT_W-1, TFT_H-1);
    uint32_t n = (uint32_t)TFT_W * TFT_H;
    uint8_t hi = color>>8, lo = color&0xFF;
    for(uint32_t i=0; i<n; i++) { spi4_write(hi); spi4_write(lo); }
}

void Graphics::fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if(x>=(int16_t)TFT_W || y>=(int16_t)TFT_H || w<=0 || h<=0) return;
    if(x<0){ w+=x; x=0; }
    if(y<0){ h+=y; y=0; }
    if(x+w>(int16_t)TFT_W) w = TFT_W - x;
    if(y+h>(int16_t)TFT_H) h = TFT_H - y;
    set_window(x, y, x+w-1, y+h-1);
    uint32_t n = (uint32_t)w * h;
    uint8_t hi = color>>8, lo = color&0xFF;
    for(uint32_t i=0; i<n; i++) { spi4_write(hi); spi4_write(lo); }
}

void Graphics::draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if((uint16_t)x >= TFT_W || (uint16_t)y >= TFT_H) return;
    set_window(x, y, x, y);
    push_color(color);
}

void Graphics::draw_hline(int16_t x, int16_t y, int16_t len, uint16_t color) {
    fill_rect(x, y, len, 1, color);
}
void Graphics::draw_vline(int16_t x, int16_t y, int16_t len, uint16_t color) {
    fill_rect(x, y, 1, len, color);
}

// Círculo relleno — scanline por scanline (sin parpadeo, sin set_window por pixel)
void Graphics::fill_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    for(int16_t dy = -r; dy <= r; dy++) {
        int16_t dx = (int16_t)( // floor(sqrt(r²-dy²))
            [](int32_t v) -> int16_t {
                if(v <= 0) return 0;
                int16_t s = (int16_t)v;
                // Newton rápido, 4 iteraciones
                int16_t x = s;
                x = (x + s/x) >> 1;
                x = (x + s/x) >> 1;
                x = (x + s/x) >> 1;
                x = (x + s/x) >> 1;
                while((int32_t)x*x > v) x--;
                return x;
            }((int32_t)r*r - (int32_t)dy*dy)
        );
        fill_rect(cx - dx, cy + dy, 2*dx + 1, 1, color);
    }
}

void Graphics::draw_circle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    int16_t x = 0, y = r, d = 3 - 2*r;
    while(x <= y) {
        draw_pixel(cx+x,cy+y,color); draw_pixel(cx-x,cy+y,color);
        draw_pixel(cx+x,cy-y,color); draw_pixel(cx-x,cy-y,color);
        draw_pixel(cx+y,cy+x,color); draw_pixel(cx-y,cy+x,color);
        draw_pixel(cx+y,cy-x,color); draw_pixel(cx-y,cy-x,color);
        if(d<0) d += 4*x+6; else { d += 4*(x-y)+10; y--; }
        x++;
    }
}

// ============================================================
//  TEXTO
// ============================================================
void Graphics::draw_char(int16_t x, int16_t y, char c,
                          uint16_t fg, uint16_t bg, uint8_t scale) {
    if(c < 0x20 || c > 0x7E) c = ' ';
    uint8_t idx = c - 0x20;
    for(uint8_t col=0; col<5; col++) {
        uint8_t line = font5x7[idx][col];
        for(uint8_t row=0; row<7; row++) {
            uint16_t color = (line & (1<<row)) ? fg : bg;
            fill_rect(x+col*scale, y+row*scale, scale, scale, color);
        }
    }
}

void Graphics::draw_string(int16_t x, int16_t y, const char *s,
                             uint16_t fg, uint16_t bg, uint8_t scale) {
    while(*s) { draw_char(x, y, *s++, fg, bg, scale); x += 6*scale; }
}

// ============================================================
//  HELPERS INTERNOS
// ============================================================

// sqrt entera rápida
static int16_t isqrt(int32_t v) {
    if(v <= 0) return 0;
    int16_t x = (int16_t)(v < 65536L ? v : 256);
    for(int i=0;i<6;i++) x = (int16_t)((x + v/x)>>1);
    while((int32_t)x*x > v) x--;
    return x;
}

// Rellena semicírculo (mitad izq/der/arr/abajo según dir)
// Usada para dibujar la "boca cerrada" y el cuerpo de Pac-Man
static void fill_semicircle(int16_t cx, int16_t cy, int16_t r,
                              uint16_t color, uint8_t dir) {
    // dir: 0=arr(boca arr) 1=abajo 2=izq 3=der
    for(int16_t dy = -r; dy <= r; dy++) {
        int16_t dx = isqrt((int32_t)r*r - (int32_t)dy*dy);
        for(int16_t px = cx-dx; px <= cx+dx; px++) {
            bool draw = false;
            switch(dir) {
                case 0: draw = (cy+dy <= cy); break; // mitad superior
                case 1: draw = (cy+dy >= cy); break; // mitad inferior
                case 2: draw = (px <= cx);    break; // mitad izquierda
                case 3: draw = (px >= cx);    break; // mitad derecha
            }
            if(draw) Graphics::draw_pixel(px, cy+dy, color);
        }
    }
}

// ============================================================
//  PAC-MAN  (sprite 18×18, centrado en (x+9, y+9))
//  Técnica anti-flicker:
//   1) Borrar bounding box completo con fill_rect (1 transacción SPI)
//   2) Dibujar cuerpo círculo relleno scanline a scanline
//   3) Dibujar boca encima (rectángulo negro o triángulo)
//   4) Dibujar ojo
// ============================================================
void Graphics::draw_pacman(int16_t x, int16_t y, bool mouth_open, uint8_t dir) {
    draw_pacman_scaled(x + PACMAN_R, y + PACMAN_R, mouth_open, dir, 1);
}

void Graphics::draw_pacman_scaled(int16_t cx, int16_t cy, bool mouth_open,
                                   uint8_t dir, uint8_t sc) {
    int16_t r = PACMAN_R * sc;

    // 1) Borrar bounding box de una vez — CERO flicker
    fill_rect(cx - r, cy - r, 2*r+1, 2*r+1, BLACK);

    // 2) Cuerpo circular
    for(int16_t dy = -r; dy <= r; dy++) {
        int16_t dx = isqrt((int32_t)r*r - (int32_t)dy*dy);
        fill_rect(cx - dx, cy + dy, 2*dx+1, 1, YELLOW);
    }

    // 3) Boca
    if(mouth_open) {
        // Ángulo ~35° → tan(35°)≈0.7 → offset = dx * 0.7
        for(int16_t i = 0; i <= r; i++) {
            int16_t w = (int16_t)((int32_t)i * 7 / 10) + 1; // apertura triangular
            switch(dir) {
                case 3: // derecha (default)
                    fill_rect(cx + i - 1, cy - w, 1, 2*w+1, BLACK);
                    break;
                case 2: // izquierda
                    fill_rect(cx - i, cy - w, 1, 2*w+1, BLACK);
                    break;
                case 0: // arriba
                    fill_rect(cx - w, cy - i, 2*w+1, 1, BLACK);
                    break;
                case 1: // abajo
                    fill_rect(cx - w, cy + i - 1, 2*w+1, 1, BLACK);
                    break;
            }
        }
    }

    // 4) Ojo (siempre arriba-derecha del centro)
    int16_t ex = cx + r/3;
    int16_t ey = cy - r/2;
    int16_t er = (sc <= 1) ? 1 : sc;
    fill_circle(ex, ey, er, BLACK);
}

// ============================================================
//  FANTASMA  (bounding box GHOST_SIZE × GHOST_SIZE = 18×18)
//  Anti-flicker: mismo principio — fill_rect primero, luego dibujar.
// ============================================================
void Graphics::draw_ghost(int16_t x, int16_t y, uint16_t color, bool frightened) {
    draw_ghost_scaled(x + GHOST_SIZE/2, y + GHOST_SIZE/2, color, frightened, 1);
}

void Graphics::draw_ghost_scaled(int16_t cx, int16_t cy, uint16_t color,
                                   bool frightened, uint8_t sc) {
    uint16_t bodyColor = frightened ? COLOR565(0,0,200) : color;
    int16_t  r   = (GHOST_SIZE/2) * sc;   // radio de la cúpula
    int16_t  top = cy - r;
    int16_t  bot = cy + r;

    // 1) Borrar bounding box completo
    fill_rect(cx - r, top, 2*r+1, 2*r+1, BLACK);

    // 2) Cúpula superior (semicírculo relleno)
    int16_t midY = cy;
    for(int16_t dy = -r; dy <= 0; dy++) {
        int16_t dx = isqrt((int32_t)r*r - (int32_t)dy*dy);
        fill_rect(cx - dx, cy + dy, 2*dx+1, 1, bodyColor);
    }
    // 3) Cuerpo rectangular debajo de la cúpula
    fill_rect(cx - r, midY, 2*r+1, r, bodyColor);

    // 4) Faldón dentado (3 dientes)
    // Cada diente: ancho = (2r)/3, alto = r/3
    int16_t dw = (2*r) / 3;
    int16_t dh = r / 3 + 1;
    int16_t startX = cx - r;
    for(int8_t t = 0; t < 3; t++) {
        // Espacio entre dientes (negro)
        int16_t gapX = startX + t * dw + dw/2;
        fill_rect(gapX, bot - dh, dw/2 + 1, dh + 1, BLACK);
    }

    // 5) Ojos (no en modo asustado)
    if(!frightened) {
        int16_t ew = (sc <= 1) ? 2 : 3*sc/2;
        int16_t ex1 = cx - r/3 - ew/2;
        int16_t ex2 = cx + r/3 - ew/2;
        int16_t ey  = cy - r/3;
        fill_rect(ex1, ey,     ew, ew, WHITE);
        fill_rect(ex2, ey,     ew, ew, WHITE);
        fill_rect(ex1+ew/2, ey+ew/2, ew/2+1, ew/2+1, BLUE);
        fill_rect(ex2+ew/2, ey+ew/2, ew/2+1, ew/2+1, BLUE);
    } else {
        // Ojos parpadeantes en modo asustado: X blancas
        int16_t ew = sc+1;
        int16_t ex1 = cx - r/3;
        int16_t ex2 = cx + r/3;
        int16_t ey  = cy - r/4;
        for(int16_t k=0; k<ew; k++) {
            draw_pixel(ex1-k, ey+k, WHITE); draw_pixel(ex1+k, ey+k, WHITE);
            draw_pixel(ex2-k, ey+k, WHITE); draw_pixel(ex2+k, ey+k, WHITE);
        }
        // Boca ondulada asustada
        for(int16_t bx = cx-r+1; bx < cx+r; bx++) {
            int16_t by = cy + r/3 + ((bx/2)&1);
            draw_pixel(bx, by, WHITE);
        }
    }
}

// ============================================================
//  LABERINTO
// ============================================================

void Graphics::draw_dot(int16_t px, int16_t py) {
    // Punto pequeño centrado en la celda
    int16_t cx = px + CELL_W/2;
    int16_t cy = py + CELL_H/2;
    fill_rect(cx-1, cy-1, 3, 3, DOT_COLOR);
}

void Graphics::draw_power(int16_t px, int16_t py) {
    int16_t cx = px + CELL_W/2;
    int16_t cy = py + CELL_H/2;
    fill_circle(cx, cy, 4, POWER_COLOR);
}

// Dibuja una celda de pared con borde redondeado/biselado
// Las flags top/bot/lft/rgt indican si el lado adyacente también es pared
// (si lo es, no se pone borde en esa dirección)
void Graphics::draw_wall_cell(int16_t px, int16_t py,
                               bool top, bool bot, bool lft, bool rgt) {
    // Fondo de pared
    fill_rect(px, py, CELL_W, CELL_H, WALL_COLOR);

    // Bordes brillantes donde hay transición pared→pasillo
    const uint16_t EC = WALL_EDGE;
    if(!top) draw_hline(px, py,          CELL_W, EC);
    if(!bot) draw_hline(px, py+CELL_H-1, CELL_W, EC);
    if(!lft) draw_vline(px,          py,  CELL_H, EC);
    if(!rgt) draw_vline(px+CELL_W-1, py,  CELL_H, EC);
}

// Efecto de explosi�n de puntos (cuando se come un fantasma)
void Graphics::draw_score_popup(int16_t x, int16_t y, uint16_t points) {
    char buf[8];
    sprintf(buf, "%u", points);
    for(int offset = 0; offset < 15; offset++) {
        draw_string(x - 10 + (rand() % 20), 
                   y - 20 + offset, 
                   buf, 
                   COLOR565(255, 200 - offset*10, 0), 
                   BLACK, 1);
        delay_ms(20);
    }
}