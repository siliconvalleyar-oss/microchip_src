/*
 * leds_btns.h  - LEDs RD0-RD3 y pulsadores RB4, RB5
 */
#ifndef LEDS_BTNS_H
#define LEDS_BTNS_H

#include "config.h"

/* Inicializar pines de LEDs y botones */
void LEDS_BTNS_Init(void);

/* Control individual de LEDs (0=apagado, 1=encendido) */
void LED_Set(uint8_t led_num, uint8_t state);

/* Encender todos los LEDs */
void LED_AllOn(void);

/* Apagar todos los LEDs */
void LED_AllOff(void);

/* Patron binario en los 4 LEDs (bits 3..0) */
void LED_Pattern(uint8_t pattern);

/* Efecto de walking LED para indicar actividad */
void LED_Walk(void);

/* Leer boton (ya debounced). 1=presionado */
uint8_t BTN_Read(uint8_t btn_num);

/* Debounce: retorna 1 si el boton fue presionado (flanco) */
uint8_t BTN_Pressed(uint8_t btn_num);

/* Actualizar estado de debounce - llamar cada ~10ms */
void BTN_Update(void);

#endif /* LEDS_BTNS_H */
