
#ifndef TEST_H    
#define TEST_H

#include <stdint.h>

void test_pin_output(uint8_t pin, const char* nombre, uint16_t tiempo_ms) ;
void test_pin_input(uint8_t pin, const char* nombre, uint8_t led_pin) ;
void test_all_pins(void);

#endif 