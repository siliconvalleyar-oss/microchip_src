#include "Sound.h"
#include "HardwareProfile.h"

static uint8_t sound_enabled = 1;

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
    for(uint32_t i = 0; i < cycles; i++) {
        SOUND_ON();
        delay_us(half_period_us);
        SOUND_OFF();
        delay_us(half_period_us);
    }
}

// Sonido de inicio de partida
void sound_start(void) {
    sound_beep(523, 150);
    delay_ms(80);
    sound_beep(659, 150);
    delay_ms(80);
    sound_beep(784, 200);
    delay_ms(100);
    sound_beep(1047, 300);
    delay_ms(150);
    sound_beep(1319, 400);
}

// Sonido de subir nivel
void sound_level_up(void) {
    sound_beep(523, 80);
    delay_ms(70);
    sound_beep(659, 80);
    delay_ms(70);
    sound_beep(784, 80);
    delay_ms(70);
    sound_beep(1047, 80);
    delay_ms(70);
    sound_beep(1319, 200);
}

// Sonido de comer punto (corto)
void sound_eat(void) {
    static uint8_t eat_counter = 0;
    eat_counter = (eat_counter + 1) % 8;
    uint16_t base_freq = 1200 + (eat_counter * 15);
    sound_beep(base_freq, 35);
}

// Sonido de power pellet
void sound_power_pellet(void) {
    for(uint16_t freq = 400; freq <= 1200; freq += 80) {
        sound_beep(freq, 8);
        delay_us(5000);
    }
    sound_beep(1500, 100);
}

// Sonido de comer fantasma
void sound_ghost(void) {
    uint16_t freqs[] = {1200, 1000, 800, 600, 400, 300, 200};
    for(int i = 0; i < 7; i++) {
        sound_beep(freqs[i], 45);
        delay_ms(20);
    }
    sound_beep(1600, 60);
}

// Sonido de muerte
void sound_death(void) {
    for(uint16_t freq = 600; freq > 100; freq -= 20) {
        sound_beep(freq, 12);
        delay_us(8000);
    }
    delay_ms(100);
    sound_beep(80, 400);
}

// Sonido de vida extra
void sound_extra_life(void) {
    for(uint16_t freq = 800; freq <= 2000; freq += 150) {
        sound_beep(freq, 25);
        delay_us(6000);
    }
    for(uint16_t freq = 2000; freq >= 800; freq -= 150) {
        sound_beep(freq, 20);
        delay_us(4000);
    }
}