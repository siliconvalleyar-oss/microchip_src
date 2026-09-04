#include "Sound.h"
#include "HardwareProfile.h"

void sound_init(void) {
    SOUND_TRIS = 0;   // salida
    SOUND_OFF();
}

// Genera un tono a la frecuencia dada (Hz) durante duración (ms)
void sound_beep(uint16_t freq_hz, uint16_t duration_ms) {
    if(freq_hz == 0) return;
    uint32_t period_us = 1000000UL / freq_hz;
    uint32_t half_period_us = period_us / 2;
    uint32_t cycles = (duration_ms * 1000UL) / period_us;
    for(uint32_t i=0; i<cycles; i++) {
        SOUND_ON();
        delay_us(half_period_us);
        SOUND_OFF();
        delay_us(half_period_us);
    }
}

// Sonido de inicio de partida (ascendente)
void sound_start(void) {
    sound_beep(440, 100);
    delay_ms(50);
    sound_beep(880, 100);
    delay_ms(50);
    sound_beep(1320, 200);
}

// Sonido de comer punto (corto)
void sound_eat(void) {
    sound_beep(1000, 50);
}

// Sonido de comer fantasma (descendente)
void sound_ghost(void) {
    sound_beep(800, 80);
    delay_ms(30);
    sound_beep(600, 80);
}

// Sonido de muerte (triste)
void sound_death(void) {
    sound_beep(400, 300);
    delay_ms(100);
    sound_beep(300, 300);
}
