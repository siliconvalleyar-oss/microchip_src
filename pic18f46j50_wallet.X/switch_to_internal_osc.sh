#!/bin/bash

echo "=== Cambiando PIC18F46J50 a oscilador INTERNO de 12MHz ==="

# 1. Modificar hal/hal_config.h
if [ -f "hal/hal_config.h" ]; then
    cp hal/hal_config.h hal/hal_config.h.cristal_backup
    echo "✓ Backup creado: hal/hal_config.h.cristal_backup"
    
    # Cambiar defines de frecuencia
    sed -i 's|#define XTAL_FREQ       12000000L|#define XTAL_FREQ       12000000L  // Oscilador interno|' hal/hal_config.h
    sed -i 's|#define PLL_FREQ        (XTAL_FREQ \* 4)|#define PLL_FREQ        (XTAL_FREQ * 4)  // PLL para 48MHz USB|' hal/hal_config.h
    
    # Cambiar fusible PLLDIV
    sed -i 's|#pragma config PLLDIV = 3|#pragma config PLLDIV = 1       // Divide by 1 (internal osc)|' hal/hal_config.h
    
    # Cambiar fusible OSC de ECPLL a IRCIO67
    sed -i 's|#pragma config OSC = ECPLL|#pragma config OSC = IRCIO67    // Oscilador INTERNO, RA6/RA7 como I/O|' hal/hal_config.h
    
    echo "✓ Modificado hal/hal_config.h"
else
    echo "✗ No se encuentra hal/hal_config.h"
fi

# 2. Agregar función de inicialización del oscilador a hal/hal_config.h
if [ -f "hal/hal_config.h" ]; then
    # Verificar si ya existe la función
    if ! grep -q "Oscillator_Init" hal/hal_config.h; then
        cat >> hal/hal_config.h << 'EOF'

//=============================================================================
// Inicialización del Oscilador Interno (12MHz)
//=============================================================================
static inline void Oscillator_Init(void) {
    // Configurar oscilador interno a 12MHz
    OSCCONbits.IRCF = 0b1110;   // 12MHz (0b1110 = 12MHz, 0b1111 = 16MHz)
    OSCCONbits.SCS = 0b10;      // Usar oscilador interno HFINTOSC
    OSCTUNEbits.PLLEN = 0;      // PLL deshabilitado (habilitar si necesitas USB)
    
    while(!OSCCONbits.HFIOFS);  // Esperar que el oscilador esté estable
    
    // Si necesitas USB a 48MHz, descomenta la siguiente línea:
    // OSCTUNEbits.PLLEN = 1;    // Habilitar PLL para 48MHz (requiere ajuste de fusibles)
}
