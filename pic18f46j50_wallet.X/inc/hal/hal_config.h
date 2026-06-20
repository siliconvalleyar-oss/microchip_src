//=============================================================================
// hal_config.h - Configuración Global del Hardware para PIC18F46J50
//=============================================================================
#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//=============================================================================
// Configuración del Oscilador (12MHz cristal -> 48MHz PLL)
//=============================================================================
#define XTAL_FREQ       12000000L
#define PLL_FREQ        (XTAL_FREQ * 4)
#define FCY             (PLL_FREQ / 4)
#define _XTAL_FREQ      FCY

//=============================================================================
// Macros de utilidad
//=============================================================================
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))
#define BIT_SET(reg, bit)   ((reg) |= (1UL << (bit)))
#define BIT_CLEAR(reg, bit) ((reg) &= ~(1UL << (bit)))
#define BIT_READ(reg, bit)  (((reg) >> (bit)) & 1)

//=============================================================================
// Fusibles para PIC18F46J50
//=============================================================================
// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer deshabilitado
#pragma config PLLDIV = 3       // Divide by 3 (12MHz input)
#pragma config STVREN = ON      // Stack Overflow Reset habilitado
#pragma config XINST = OFF      // Extended Instruction Set deshabilitado

// CONFIG1H
#pragma config CPUDIV = OSC1    // No CPU system clock divide
#pragma config CP0 = OFF        // Code Protection deshabilitada

// CONFIG2L
#pragma config OSC = ECPLL      // HS oscillator, PLL enabled
#pragma config T1DIG = ON       // Secondary Oscillator clock source may be selected
#pragma config LPT1OSC = OFF    // Timer1 high power operation
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor deshabilitado
#pragma config IESO = OFF       // Two-Speed Start-up deshabilitado

// CONFIG2H
#pragma config WDTPS = 32768    // Watchdog Postscale 1:32768
#pragma config DSWDTOSC = INTOSCREF
#pragma config RTCOSC = T1OSCREF
#pragma config DSBOREN = OFF
#pragma config DSWDTEN = OFF

// CONFIG3L
#pragma config DSWDTPS = 8192
#pragma config IOL1WAY = OFF

// CONFIG3H
#pragma config MSSP7B_EN = MSK7
#pragma config WPFP = PAGE_1
#pragma config WPEND = PAGE_0
#pragma config WPCFG = OFF
#pragma config WPDIS = OFF

#endif
