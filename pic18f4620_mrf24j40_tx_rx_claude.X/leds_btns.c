/*
 * leds_btns.c  - LEDs y pulsadores con debounce
 *
 * LEDs: RD0-RD3 salidas activo alto
 * BTN:  RB4, RB5 entradas activo bajo con pull-up interno
 *        (INTCON2: RBPU=0 para activar pull-ups)
 */
#include "leds_btns.h"

/* Estado de debounce */
static uint8_t btn_state[2]    = {1u, 1u};   /* 1=no presionado */
static uint8_t btn_prev[2]     = {1u, 1u};
static uint8_t btn_pressed[2]  = {0u, 0u};
static uint8_t walk_pos        = 0u;

/* ---- Inicializar --------------------------------------- */
void LEDS_BTNS_Init(void)
{
    /* LEDs como salidas, apagados */
    LED0_TRIS = 0; LED0 = 0;
    LED1_TRIS = 0; LED1 = 0;
    LED2_TRIS = 0; LED2 = 0;
    LED3_TRIS = 0; LED3 = 0;

    /* Botones como entradas */
    BTN0_TRIS = 1;
    BTN1_TRIS = 1;

    /* Activar pull-ups internos en PORTB */
    /* INTCON2: RBPU=0 (activo bajo = pull-ups ON) */
    INTCON2bits.RBPU = 0;
}

/* ---- Control LEDs -------------------------------------- */
void LED_Set(uint8_t num, uint8_t state)
{
    switch (num) {
        case 0: LED0 = state ? 1u : 0u; break;
        case 1: LED1 = state ? 1u : 0u; break;
        case 2: LED2 = state ? 1u : 0u; break;
        case 3: LED3 = state ? 1u : 0u; break;
        default: break;
    }
}

void LED_AllOn(void)  { LED0=1; LED1=1; LED2=1; LED3=1; }
void LED_AllOff(void) { LED0=0; LED1=0; LED2=0; LED3=0; }

void LED_Pattern(uint8_t p)
{
    LED0 = (p >> 0u) & 1u;
    LED1 = (p >> 1u) & 1u;
    LED2 = (p >> 2u) & 1u;
    LED3 = (p >> 3u) & 1u;
}

void LED_Walk(void)
{
    LED_AllOff();
    LED_Set(walk_pos, 1);
    walk_pos = (uint8_t)((walk_pos + 1u) % 4u);
}

/* ---- Botones con debounce simple ----------------------- */
void BTN_Update(void)
{
    uint8_t raw[2];
    raw[0] = BTN0_PORT ? 1u : 0u;   /* 1=no presionado (pull-up) */
    raw[1] = BTN1_PORT ? 1u : 0u;

    /* Debounce: solo cambiar estado si coincide dos lecturas */
    /* Aqui implementamos un debounce sencillo de un ciclo */
    uint8_t i;
    for (i = 0; i < 2u; i++) {
        btn_prev[i]    = btn_state[i];
        btn_state[i]   = raw[i];
        /* Flanco descendente = presionado */
        if (btn_prev[i] == 1u && btn_state[i] == 0u) {
            btn_pressed[i] = 1u;
        }
    }
}

uint8_t BTN_Read(uint8_t num)
{
    if (num > 1u) return 0u;
    return (btn_state[num] == 0u) ? 1u : 0u;
}

uint8_t BTN_Pressed(uint8_t num)
{
    uint8_t p;
    if (num > 1u) return 0u;
    p = btn_pressed[num];
    btn_pressed[num] = 0u;   /* Limpiar flanco */
    return p;
}
