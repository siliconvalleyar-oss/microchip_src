#ifndef TIMERS_H  // Evita inclusión múltiple del header
#define TIMERS_H  // Define el guardián de inclusión

#include "config.h"  // Configuración general y definiciones del sistema

// Variables volátiles compartidas entre ISR y programa principal
extern volatile uint8_t contador100ms;      // Contador de 100ms
extern volatile uint8_t contador10ms;       // Contador de 10ms
extern volatile uint8_t contadordecontador; // Contador auxiliar anidado
extern volatile uint8_t TimeoutMenu;        // Temporizador de timeout del menú
extern volatile uint8_t FlagMenuOFF;        // Bandera para ocultar menú por timeout
extern volatile uint8_t ContadorF;          // Contador de muestras ferrosas
extern volatile uint8_t ContadorNF;         // Contador de muestras no ferrosas
extern volatile uint8_t ContadorM;          // Contador de muestras para promedio
extern volatile uint8_t Contador10seg;      // Contador de 10 segundos
extern volatile uint8_t Flag10seg;          // Bandera de evento cada 10 segundos
extern volatile uint8_t Contador500Hz;      // Contador de eventos a 500Hz
extern volatile uint8_t FlagCalcular;       // Bandera para calcular segmentos

extern volatile uint8_t Segmentos;          // Número de segmentos detectados
extern volatile uint8_t SegmentosMostrados; // Segmentos actualmente mostrados en display
extern volatile uint8_t Mute;               // Estado de silencio
extern volatile uint8_t Tone;               // Frecuencia del tono de audio
extern volatile uint8_t OffsetTono;         // Offset de tono
extern volatile uint8_t Nofuncional;        // Bandera de modo no funcional

extern volatile word Medicion;              // Acumulador de mediciones ADC
extern volatile word ValorCalculado;        // Valor calculado del ADC (promediado)
extern volatile word ADCBateria;            // Valor ADC de la batería

// Prototipos de funciones
void initTIM2(void);                        // Inicializa el Timer2
void TonoON(void);                          // Activa el tono PWM
void TonoOFF(void);                         // Desactiva el tono PWM
void AjustarPWM(word valor, uint8_t offset); // Ajusta el ciclo de trabajo del PWM

#endif  // Fin del guardián de inclusión
