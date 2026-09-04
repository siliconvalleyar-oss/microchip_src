#ifndef EEPROM_H  // Evita inclusión múltiple del header
#define EEPROM_H  // Define el guardián de inclusión

#include "config.h"   // Configuración general y definiciones del sistema
#include "timers.h"   // Variables de temporización

// Variables globales almacenadas en RAM que se persisten en EEPROM
extern uint8_t Volumen;          // Nivel de volumen del audio
extern uint8_t Discrimination;   // Nivel de discriminación ferroso/no-ferroso
extern uint8_t SamplePosition;   // Posición de muestreo de la señal
extern uint8_t Ganancia;         // Ganancia del detector
extern uint8_t OtroParam;        // Otro parámetro configurable

// Funciones públicas de gestión de EEPROM
void VerEEPROM(void);            // Verifica y carga los valores desde EEPROM
void GrabarEEPROM(void);         // Guarda todos los valores actuales en EEPROM
void LoadRAM(void);              // Carga los valores desde EEPROM a RAM
void LoadRAM_Default(void);      // Carga los valores por defecto en RAM
void ReLoadEEPROM(void);         // Recarga valores por defecto y los guarda en EEPROM

#endif  // Fin del guardián de inclusión
