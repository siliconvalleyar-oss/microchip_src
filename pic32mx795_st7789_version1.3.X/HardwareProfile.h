/**
 * @file HardwareProfile.h
 * @brief Configuración específica del proyecto (usa driver base común).
 */

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include "../../ST7789_common/st7789_driver.h"

// ==================== PINES DE BOTONES (opcionales) ====================
// #define USE_BUTTONS
#ifdef USE_BUTTONS
#define BTN_LEFT_PIN  7
#define BTN_RIGHT_PIN 8
#define BTN_FIRE_PIN  9
#define BTN_LEFT      (!(PORTB >> BTN_LEFT_PIN) & 1)
#define BTN_RIGHT     (!(PORTB >> BTN_RIGHT_PIN) & 1)
#define BTN_FIRE      (!(PORTB >> BTN_FIRE_PIN) & 1)
#endif

// ==================== PARÁMETROS ESPECÍFICOS ====================
#define SHIP_W      16
#define SHIP_H      24

#endif // HARDWARE_PROFILE_H
