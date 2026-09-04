#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

// Definir los pines de LEDs
#define LED0_PIN    7
#define LED1_PIN    6
#define LED2_PIN    5

#define LED3_PIN    4   // RE4 - pin 64
#define LED4_PIN    3   // RE3 - pin 63
#define LED5_PIN    2   // RE2 - pin 62
#define LED6_PIN    1   // RE1 - pin 61
#define LED7_PIN    0   // RE0 - pin 60

// LEDs en puerto D
#define LED8_PIN    7   // RD7 - pin 48
#define LED9_PIN    6   // RD6 - pin 49
#define LED10_PIN   5   // RD5 - pin 50
#define LED11_PIN   4   // RD4 - pin 51
#define LED12_PIN   3   // RD3 - pin 52
#define LED13_PIN   2   // RD2 - pin 53
#define LED14_PIN   1   // RD1 - pin 54
#define LED15_PIN   0   // RD0 - pin 55



// Macros para LEDs (visibles en cualquier archivo que incluya este header)
#define LED0_ON()   LATESET = (1 << LED0_PIN)
#define LED0_OFF()  LATECLR = (1 << LED0_PIN)
#define LED1_ON()   LATESET = (1 << LED1_PIN)
#define LED1_OFF()  LATECLR = (1 << LED1_PIN)
#define LED2_ON()   LATESET = (1 << LED2_PIN)
#define LED2_OFF()  LATECLR = (1 << LED2_PIN)
#define LED3_ON()   LATESET = (1 << LED3_PIN)
#define LED3_OFF()  LATECLR = (1 << LED3_PIN)
#define LED4_ON()   LATESET = (1 << LED4_PIN)
#define LED4_OFF()  LATECLR = (1 << LED4_PIN)


#define LED5_ON()   LATESET = (1 << LED5_PIN)
#define LED5_OFF()  LATECLR = (1 << LED5_PIN)
#define LED6_ON()   LATESET = (1 << LED6_PIN)
#define LED6_OFF()  LATECLR = (1 << LED6_PIN)
#define LED7_ON()   LATESET = (1 << LED7_PIN)
#define LED7_OFF()  LATECLR = (1 << LED7_PIN)

#define LED8_ON()   LATDSET = (1 << LED8_PIN)
#define LED8_OFF()  LATDCLR = (1 << LED8_PIN)
#define LED9_ON()   LATDSET = (1 << LED9_PIN)
#define LED9_OFF()  LATDCLR = (1 << LED9_PIN)
#define LED10_ON()   LATDSET = (1 << LED10_PIN)
#define LED10_OFF()  LATDCLR = (1 << LED10_PIN)
#define LED11_ON()   LATDSET = (1 << LED11_PIN)
#define LED11_OFF()  LATDCLR = (1 << LED11_PIN)
#define LED12_ON()   LATDSET = (1 << LED12_PIN)
#define LED12_OFF()  LATDCLR = (1 << LED12_PIN)
#define LED13_ON()   LATDSET = (1 << LED13_PIN)
#define LED13_OFF()  LATDCLR = (1 << LED13_PIN)
#define LED14_ON()   LATDSET = (1 << LED14_PIN)
#define LED14_OFF()  LATDCLR = (1 << LED14_PIN)
#define LED15_ON()   LATDSET = (1 << LED15_PIN)
#define LED15_OFF()  LATDCLR = (1 << LED15_PIN)


// Declaración de funciones (extern)
extern void delayMs(uint32_t ms);
extern void led_init(void);

#endif