 
#include "test.h"
//#include "EPD_Driver.h"
//#include "EPD_Graphics.h"
//#include "HardwareProfile.h"

#include <xc.h>


void test_pin_output(uint8_t pin, const char* nombre, uint16_t tiempo_ms) {
    // Configurar como salida
    epd_pinMode(pin, 1);
    // Parpadear 3 veces
    for (int i = 0; i < 3; i++) {
        epd_digitalWrite(pin, 1);
        delayMs(tiempo_ms);
        epd_digitalWrite(pin, 0);
        delayMs(tiempo_ms);
    }
    // Dejar en bajo
    epd_digitalWrite(pin, 0);
}

void test_pin_input(uint8_t pin, const char* nombre, uint8_t led_pin) {
    epd_pinMode(pin, 0);  // entrada
    epd_pinMode(led_pin, 1); // LED como salida
    // Leer 10 veces cada 100ms y mostrar en LED
    for (int i = 0; i < 10; i++) {
        uint8_t val = epd_digitalRead(pin);
        if (val) {
            // Si el pin está en alto, enciende LED
            epd_digitalWrite(led_pin, 1);
        } else {
            epd_digitalWrite(led_pin, 0);
        }
        delayMs(100);
    }
    epd_digitalWrite(led_pin, 0);
}


void test_all_pins(void) {
    // Usar LED0 (RE7) para indicar inicio de prueba
    epd_pinMode(7, 1); // Asumiendo LED0 en RE7 (pin 7 según hardware profile)
    epd_digitalWrite(7, 1);
    delayMs(500);
    epd_digitalWrite(7, 0);
    delayMs(500);

    // Lista de pines a probar como salida
    struct {
        uint8_t pin;
        const char* name;
    } output_pins[] = {
        {16, "RB0"},
        {17, "RB1"},
        {18, "RB2"},
        {19, "RB3"},
        {29, "RB13"},
        {30, "RB14"},
        {84, "RF4"},
        {85, "RF5"}
    };

    // Probar cada pin como salida (parpadeo)
    for (uint8_t i = 0; i < sizeof(output_pins)/sizeof(output_pins[0]); i++) {
        // Indicar con LED0 que estamos probando este pin
        epd_digitalWrite(7, 1);
        delayMs(100);
        epd_digitalWrite(7, 0);
        
        // Parpadear el pin 5 veces para que se vea bien
        test_pin_output(output_pins[i].pin, output_pins[i].name, 200);
        
        delayMs(500); // pausa entre pines
    }

    // Prueba especial para RB0 como entrada (BUSY)
    // Conecta un pulsador a RB0 o déjalo flotante (flotante leerá 0 o 1 inestable)
    // Mostramos el estado en LED0 durante 2 segundos
    epd_pinMode(16, 0); // RB0 como entrada
    epd_pinMode(7, 1);  // LED0 salida
    for (int i = 0; i < 20; i++) {
        uint8_t val = epd_digitalRead(16);
        epd_digitalWrite(7, val);
        delayMs(100);
    }
    epd_digitalWrite(7, 0);

    // Finalizar: parpadeo rápido de LED0
    for (int i = 0; i < 10; i++) {
        epd_digitalWrite(7, 1);
        delayMs(100);
        epd_digitalWrite(7, 0);
        delayMs(100);
    }
}