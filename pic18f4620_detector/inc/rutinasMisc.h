#ifndef RUTINASMISC_H  // Evita inclusión múltiple del header
#define RUTINASMISC_H  // Define el guardián de inclusión

#include "config.h"   // Configuración general y definiciones del sistema
#include "timers.h"   // Variables de temporización

// Variables globales de control
extern uint8_t Debug;          // Modo debug activado/desactivado
extern uint8_t Sensibilidad;   // Nivel de sensibilidad actual
extern uint8_t Menu;           // Número de menú activo
extern uint8_t Backlight;      // Estado del backlight
extern uint8_t Volumen;        // Nivel de volumen
extern uint8_t Ganancia;       // Ganancia del detector
extern uint8_t Discrimination; // Nivel de discriminación
extern uint8_t SamplePosition; // Posición de muestreo

extern word ValorZero;         // Valor de referencia cero calculado
extern word ValorDefinitivo;   // Valor definitivo después del procesamiento

// Control de LCD
void BACKLIGHT_ON(void);       // Enciende el backlight del LCD
void BACKLIGHT_OFF(void);      // Apaga el backlight del LCD

// Control de sensibilidad (potenciómetro digital)
void SENSIB_CERO(void);        // Lleva la sensibilidad a cero
void SENSIB_MAS(void);         // Incrementa la sensibilidad en un paso
void SENSIB_MENOS(void);       // Decrementa la sensibilidad en un paso
void FORZAR_SENSIBILIDAD(byte valor);  // Fija la sensibilidad a un valor específico

// Control de volumen (potenciómetro digital)
void VOLUMEN_CERO(void);       // Lleva el volumen a cero
void VOLUMEN_MAS(void);        // Incrementa el volumen en un paso
void VOLUMEN_MENOS(void);      // Decrementa el volumen en un paso
void FORZAR_VOLUMEN(byte valor);  // Fija el volumen a un valor específico

// Funciones de LCD
void ENVIARCOMANDO(byte comando);  // Envía un comando al LCD
void ENVIARDATO(byte dato);        // Envía un dato (carácter) al LCD
void INITLCD(void);                // Inicializa el LCD en modo 4 bits

// Funciones de visualización
void MostrarVolumen(uint8_t Numero);          // Muestra el volumen en LCD
void MostrarAjuste(uint8_t Numero);           // Muestra el valor de ajuste
void MostrarGanancia(uint8_t Numero);         // Muestra la ganancia en LCD
void MostrarTitulo(uint8_t Numero);           // Muestra el título del menú
void MostrarSample(uint8_t Numero);           // Muestra la posición de sample
void MostrarDiscrimination(uint8_t Numero);   // Muestra la discriminación
void MostrarTone(uint8_t Numero);             // Muestra la frecuencia del tono
void MostrarMute(void);                       // Muestra el estado de mute
void MostrarWord(word Numero);                // Muestra un número de 16 bits
void MostrarNumero(uint8_t Numero);           // Muestra un número de 8 bits
void MostrarOffset(uint8_t Numero);           // Muestra el offset de tono

uint8_t MostrarBat(void);                     // Muestra el nivel de batería y retorna estado
void Presentacion(void);                      // Muestra la pantalla de presentación
void DibujarVumetro(uint8_t Menuvalor);       // Dibuja el vúmetro de barras en LCD

// Funciones de menú
void SubirVariable(void);         // Incrementa la variable del menú actual
void BajarVariable(void);         // Decrementa la variable del menú actual
void RecargarValoresDefault(void);  // Recarga valores por defecto si se mantiene reset
void CargarDefaults(void);        // Carga y muestra valores por defecto

// Inicialización del hardware
void initPorts(void);             // Configura los puertos de E/S
void initMCU(void);               // Inicialización completa del microcontrolador
void initPLL(void);               // Configura el PLL del oscilador

// Funciones de retardo
void retardo50ms(void);   // Retardo de 50ms
void retardo01ms(void);   // Retardo de 0.1ms (100µs)
void retardo1ms(void);    // Retardo de 1ms
void retardo100ms(void);  // Retardo de 100ms
void retardo20ms(void);   // Retardo de 20ms
void retardo500(void);    // Retardo de 500ms
void retardo10(void);     // Retardo de 10ms
void retardo5(void);      // Retardo de 5ms
void retardo1(void);      // Retardo de 1ms
void retardo100(void);    // Retardo de 100ms
void retardo1s(void);     // Retardo de 1 segundo

void CalcularZero(void);  // Calcula el valor de referencia cero

#endif  // Fin del guardián de inclusión
