// PIC18F452 - Frecuencímetro + Regenerador de señal
// Entradas:  RC1 (CCP2 captura), RA2 (señal generador), RA3 (ref 2.5V)
// Salidas:   RB0 (señal reproducida, 5V), RB1 (1 kHz fijo), RB5 (LED señal OK)
// Fosc = 20 MHz -> Fosc/4 = 5 MHz (base para timers)

#include <xc.h>
#pragma config OSC = HS, WDT = OFF, LVP = OFF, DEBUG = OFF
#define _XTAL_FREQ 20000000

// ??? Variables globales (accedidas en ISR y main) ????????????????????????????
volatile unsigned int  period_ticks  = 0;  // Ticks de Timer1 entre flancos
volatile unsigned char new_period    = 0;  // Flag: hay periodo nuevo para procesar
volatile unsigned char signal_lost   = 1;  // 1 = sin señal
volatile unsigned int  timeout_cnt   = 0;  // Contador timeout (ref: 1000 = 500 ms)
volatile unsigned char toggle_rb1    = 0;  // Toggle para 1 kHz en RB1

// ??? ISR única (prioridad simple) ????????????????????????????????????????????
void __interrupt() isr(void)
{
    // ?? CCP2: captura flanco de subida en RC1 (salida del comparador ? RA4?RC1)
    if (PIR2bits.CCP2IF)
    {
        static unsigned int prev = 0;
        unsigned int curr = ((unsigned int)CCPR2H << 8) | CCPR2L;

        if (prev != 0)
        {
            unsigned int diff = curr - prev;  // Resta modular: OK con overflow de Timer1
            // Rango válido: 100 Hz ? ticks=50000 / 20 kHz ? ticks=250
            if (diff >= 250 && diff <= 50000)
            {
                period_ticks = diff;
                new_period   = 1;
            }
        }
        prev = curr;

        signal_lost  = 0;
        timeout_cnt  = 0;
        PIR2bits.CCP2IF = 0;
    }

    // ?? Timer0: base de tiempo 500 µs ? genera 1 kHz en RB1 + timeout
    if (INTCONbits.TMR0IF)
    {
        // Recargar ANTES de procesar (minimiza drift acumulado)
        // 500 µs @ Fosc/4=5MHz, prescaler 1:1 ? 2500 ticks ? 65536-2500 = 63036 = 0xF63C
        TMR0H = 0xF6;
        TMR0L = 0x3C;

        // 1 kHz en RB1: toggle cada 500 µs
        toggle_rb1 ^= 1;
        LATBbits.LATB1 = toggle_rb1;

        // Timeout: 1000 interrupciones × 500 µs = 500 ms sin captura ? señal perdida
        if (!signal_lost)
        {
            if (++timeout_cnt >= 1000)
                signal_lost = 1;
        }
        INTCONbits.TMR0IF = 0;
    }

    // ?? Timer2: genera la frecuencia reproducida en RB0 (toggle = mitad del periodo)
    if (PIR1bits.TMR2IF)
    {
        LATBbits.LATB0 ^= 1;
        PIR1bits.TMR2IF = 0;
    }
}

// ??? Configura Timer2 para reproducir freq_hz en RB0 ????????????????????????
// Timer2 prescaler 1:4  ? tick = 0.8 µs
// Toggle en ISR ? periodo real = 2 × (PR2+1) × tick
// Despejando: PR2 = (Fosc/4) / (4 × freq_hz × 2) - 1
//           = 625000 / freq_hz - 1
static unsigned char ultimo_pr2 = 0;

void set_frecuencia_salida(unsigned int freq_hz)
{
    if (freq_hz < 100 || freq_hz > 20000) return;

    unsigned long pr2_val = (625000UL / (unsigned long)freq_hz);
    if (pr2_val > 0) pr2_val--;          // PR2 = valor - 1
    if (pr2_val > 255) pr2_val = 255;
    if (pr2_val < 1)   pr2_val = 1;

    unsigned char pr2 = (unsigned char)pr2_val;

    // Solo reconfigurar si el valor cambió (evita glitches innecesarios)
    if (pr2 == ultimo_pr2) return;
    ultimo_pr2 = pr2;

    T2CONbits.TMR2ON = 0;   // Apagar mientras reconfigura
    TMR2 = 0;               // Reset contador
    PR2  = pr2;
    // T2CKPS ya configurado en main (prescaler 1:4)
    PIR1bits.TMR2IF = 0;
    T2CONbits.TMR2ON = 1;
}

// ??? Programa principal ???????????????????????????????????????????????????????
void main(void)
{
    // Test oscilador: 3 parpadeos en RB5
    TRISBbits.TRISB5 = 0;
    LATBbits.LATB5   = 0;
    for (int i = 0; i < 3; i++) {
        LATBbits.LATB5 = 1; __delay_ms(150);
        LATBbits.LATB5 = 0; __delay_ms(150);
    }

    // ?? Pines ????????????????????????????????????????????????????????????????
    // RA2 = entrada analógica (señal del generador, hasta 3V)
    // RA3 = entrada analógica (referencia 2.5V con divisor 10k+10k)
    // RA4 = salida del comparador C1 (conectar físicamente a RC1)
    // RC1 = entrada CCP2
    // RB0 = salida señal reproducida (5V, misma frecuencia)
    // RB1 = salida 1 kHz fijo
    // RB5 = LED indicador señal OK

    ADCON1 = 0x06;          // RA0-RA4 como analógicas (necesario para comparador)
                            // Resto digitales
    TRISAbits.TRISA2 = 1;  // RA2 entrada (señal generador ? V+)
    TRISAbits.TRISA3 = 1;  // RA3 entrada (referencia 2.5V ? V-)
    TRISAbits.TRISA4 = 0;  // RA4 salida del comparador (C1OUT)
    TRISBbits.TRISB0 = 0;  // RB0 salida (señal reproducida)
    TRISBbits.TRISB1 = 0;  // RB1 salida (1 kHz)
    TRISBbits.TRISB5 = 0;  // RB5 LED
    TRISCbits.TRISC1 = 1;  // RC1 entrada CCP2

    LATBbits.LATB0 = 0;
    LATBbits.LATB1 = 0;
    LATBbits.LATB5 = 0;

    // ?? Comparador analógico ?????????????????????????????????????????????????
    // C1 activo: V+ = RA2 (señal), V- = RA3 (ref 2.5V)
    // Salida C1OUT en RA4 (conectar a RC1 para CCP2)
    // CMCON: CM2:CM0 = 010 ? Un comparador independiente con salida en RA4
    //        CIS=0 (RA0?V-, RA3?V+... ojo: revisar datasheet para pinout exacto)
    // Para PIC18F452: CM=010, C1INV=0, C2INV=0
    // RA2=C1IN+ (pin 4), RA3=C1IN- (pin 5) con CM=010
//    CMCON = 0x02;           // Un comparador, salida en RA4, sin inversión
// DESPUÉS (correcto para XC8 v3.x con PIC18F452):
//CMCON0 = 0x02;                          // C1: V+ = RA2, V- = RA3

    // ?? Timer1: base de tiempo para CCP2 (capture) ???????????????????????????
    // TMR1CS=0 (Fosc/4), T1CKPS=00 (prescaler 1:1) ? tick = 0.2 µs
    // Con 16 bits: overflow cada 13.1 ms ? la resta modular en ISR lo maneja
    T1CON = 0x81;           // ON, Fosc/4, prescaler 1:1, 16-bit

    // ?? CCP2: captura en RC1, flanco de subida ????????????????????????????????
    CCP2CON = 0x05;         // Captura cada flanco de subida

    // ?? Timer0: 500 µs para 1 kHz y timeout ??????????????????????????????????
    T0CON = 0x88;           // 16-bit, Fosc/4, prescaler 1:1, ON
    TMR0H = 0xF6;
    TMR0L = 0x3C;

    // ?? Timer2: prescaler 1:4, postscaler 1:1 (se activa desde main) ?????????
    T2CON  = 0x05;          // Postscaler 1:1, prescaler 1:4, ON=0 (arranca apagado)
    PR2    = 124;           // Valor por defecto (no importa, se ajusta antes de ON)
    T2CONbits.TMR2ON = 0;

    // ?? Interrupciones ????????????????????????????????????????????????????????
    RCONbits.IPEN    = 0;   // Sin prioridades (ISR única)
    PIE2bits.CCP2IE  = 1;
    PIR2bits.CCP2IF  = 0;
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;
    PIE1bits.TMR2IE  = 1;
    PIR1bits.TMR2IF  = 0;
    INTCONbits.PEIE  = 1;
    INTCONbits.GIE   = 1;

    // ?? Bucle principal ???????????????????????????????????????????????????????
    unsigned int last_freq = 0;

    while (1)
    {
        if (new_period)
        {
            new_period = 0;
            unsigned int ticks = period_ticks; // Leer una vez (volatile)

            if (ticks >= 250 && ticks <= 50000)
            {
                // Fosc/4 = 5 MHz, Timer1 prescaler 1:1 ? tick = 0.2 µs
                // freq = 1 / (ticks × 0.2µs) = 5.000.000 / ticks
                unsigned int freq = (unsigned int)(5000000UL / ticks);

                if (freq >= 100 && freq <= 20000 && freq != last_freq)
                {
                    last_freq = freq;
                    set_frecuencia_salida(freq);
                }
            }
        }

        // Control señal perdida: apagar Timer2, poner RB0=0, LED apagado
        if (signal_lost)
        {
            if (T2CONbits.TMR2ON)
            {
                T2CONbits.TMR2ON = 0;
                LATBbits.LATB0   = 0;
                ultimo_pr2       = 0; // Forzar reconfiguración al recuperar señal
                last_freq        = 0;
            }
            LATBbits.LATB5 = 0; // LED apagado = sin señal
        }
        else
        {
            LATBbits.LATB5 = 1; // LED encendido = señal OK
            if (!T2CONbits.TMR2ON && ultimo_pr2 != 0)
                T2CONbits.TMR2ON = 1;
        }
    }
}

/*
 * 
 * #include <xc.h>

// Configuración de fusibles
#pragma config OSC = HS      // Cristal HS 20 MHz
#pragma config WDT = OFF      // Watchdog OFF
#pragma config LVP = OFF      // Low Voltage Programming OFF
#pragma config DEBUG = OFF    // Debug OFF
// #pragma config PBADEN = OFF   // (comentar si no compila)
// #pragma config MCLRE = ON     // (comentar si no compila)

//#pragma config CCP2MX = OFF   // usar RB3


#define _XTAL_FREQ 20000000


// ======================== VARIABLES GLOBALES ========================
// Para medición de frecuencia externa (CCP2 en RB3)
volatile unsigned int period_ticks = 0;   // Periodo medido en ticks de Timer1
volatile unsigned char new_period = 0;    // Bandera: 1 cuando hay nuevo periodo
volatile unsigned char no_signal = 0;     // Bandera de timeout (sin señal)

// Para generación de señal en RB0 (Timer2)
volatile unsigned int frecuencia_objetivo = 2400;   // Hz, valor inicial
volatile unsigned char toggle = 0;                  // estado de la salida RB0

// ======================== FUNCIONES AUXILIARES ========================
void delay_ms(unsigned int ms) {
    while(ms--) __delay_ms(1);
}

// Configura Timer2 para generar interrupción a 2*freq (toggle cada medio periodo)
void configurar_timer2(unsigned int freq) {
    // Fosc/4 = 5 MHz, prescaler 1:4 -> 1.25 MHz (0.8 us por tick)
    // PR2 = (625000 / freq) - 1  (derivado de: periodo medio = 500000/freq us)
    unsigned long pr2_val = (625000UL / freq) - 1;
    if(pr2_val > 255) pr2_val = 255;
    
    T2CONbits.TMR2ON = 0;          // Detener Timer2
    PR2 = (unsigned char)pr2_val;
    T2CONbits.T2CKPS = 0b01;       // Prescaler 1:4
    T2CONbits.TOUTPS = 0b0000;     // Postscaler 1:1
    PIR1bits.TMR2IF = 0;
    T2CONbits.TMR2ON = 1;          // Arrancar Timer2
}

// ======================== INTERRUPCIÓN DE ALTA PRIORIDAD ========================
void __interrupt(high_priority) isr_high(void) {
    // --- 1. Captura CCP2 (flanco de subida en RB3) ---
    if(PIR2bits.CCP2IF) {
        static unsigned int prev = 0;
        unsigned int curr = ((unsigned int)CCPR2H << 8) | CCPR2L;
        if(prev != 0) {
            period_ticks = curr - prev;
            new_period = 1;
        }
        prev = curr;
        PIR2bits.CCP2IF = 0;
        no_signal = 0;          // Hay señal, reiniciar timeout
    }
    
    // --- 2. Timer0: timeout de 200 ms (detecta ausencia de señal) ---
    if(INTCONbits.TMR0IF) {
        no_signal = 1;          // No hubo captura en los últimos 200 ms
        // Recargar Timer0
        TMR0H = 0xF0;
        TMR0L = 0xBE;
        INTCONbits.TMR0IF = 0;
    }
    
    // --- 3. Timer2: generación de señal en RB0 (frecuencia variable) ---
    if(PIR1bits.TMR2IF) {
        toggle = !toggle;
        LATBbits.LATB0 = toggle;   // Salida en RB0
        PIR1bits.TMR2IF = 0;
    }
}

// ======================== PROGRAMA PRINCIPAL ========================
void main(void) {
    // ----- Prueba de oscilador (3 parpadeos rápidos) -----
    TRISBbits.TRISB5 = 0;
    for(int i = 0; i < 3; i++) {
        LATBbits.LATB5 = 1;
        __delay_ms(200);
        LATBbits.LATB5 = 0;
        __delay_ms(200);
    }
    
    // ----- Configuración de pines -----
    ADCON1 = 0x0F;              // Todos los pines digitales
    
    TRISBbits.TRISB0 = 0;       // RB0 salida (señal generada)
    TRISBbits.TRISB3 = 1;       // RB3 entrada (señal externa a medir)
    TRISBbits.TRISB5 = 0;       // RB5 salida (LED de parpadeo)
    TRISAbits.TRISA4 = 1;       // RA4 entrada (pulsador)
    
    // ----- Configuración de CCP2 (captura de frecuencia externa) -----
    CCP2CON = 0x05;             // Captura en flanco de subida
    T1CON = 0x81;               // Timer1: 16-bit, interno Fosc/4 = 5 MHz, prescaler 1:1, ON
    
    // ----- Configuración de Timer0 (timeout 200 ms) -----
    T0CON = 0x87;               // 16-bit, prescaler 1:256, timer0 ON, interno
    TMR0H = 0xF0;
    TMR0L = 0xBE;
    
    // ----- Configuración de Timer2 (generación de señal en RB0) -----
    configurar_timer2(frecuencia_objetivo);
    
    // ----- Configuración de interrupciones (todas alta prioridad) -----
    RCONbits.IPEN = 0;          // Prioridad única
    PIE2bits.CCP2IE = 1;        // Habilitar interrupción de CCP2
    PIR2bits.CCP2IF = 0;
    INTCONbits.TMR0IE = 1;      // Habilitar interrupción de Timer0
    INTCONbits.TMR0IF = 0;
    PIE1bits.TMR2IE = 1;        // Habilitar interrupción de Timer2
    PIR1bits.TMR2IF = 0;
    INTCONbits.PEIE = 1;        // Periféricos
    INTCONbits.GIE = 1;         // Global
    
    // ----- Variables locales para el bucle principal -----
    unsigned char last_ra4 = 1;
    unsigned int tiempo_parpadeo = 500;   // por defecto (sin señal o ?1 kHz)
    
    // ----- Bucle infinito -----
    while(1) {
        // ---------- CONTROL DEL PULSADOR EN RA4 (cambia frecuencia de salida RB0) ----------
        unsigned char ra4_actual = PORTAbits.RA4;
        if(ra4_actual == 0 && last_ra4 == 1) {
            __delay_ms(50);   // antirrebote
            if(PORTAbits.RA4 == 0) {
                frecuencia_objetivo += 200;
                if(frecuencia_objetivo > 4000) {
                    frecuencia_objetivo = 2400;
                }
                configurar_timer2(frecuencia_objetivo);   // actualizar Timer2
                while(PORTAbits.RA4 == 0);                // esperar que suelte
                __delay_ms(50);
            }
        }
        last_ra4 = ra4_actual;
        
        // ---------- ACTUALIZAR PARPADEO DEL LED SEGÚN FRECUENCIA MEDIDA (CCP2) ----------
        if(new_period) {
            new_period = 0;
            unsigned int p = period_ticks;   // copia segura
            // Calcular tiempo de parpadeo según periodo medido
            // Frecuencia = 5,000,000 / p (Hz)
            if(p < 1250)        tiempo_parpadeo = 50;   // >4 kHz
            else if(p < 1667)   tiempo_parpadeo = 150;  // 3-4 kHz
            else if(p < 2500)   tiempo_parpadeo = 250;  // 2-3 kHz
            else if(p < 5000)   tiempo_parpadeo = 350;  // 1-2 kHz
            else                tiempo_parpadeo = 500;  // ?1 kHz
        }
        
        // Si no hay señal, usar parpadeo por defecto (500 ms)
        if(no_signal) {
            tiempo_parpadeo = 500;
        }
        
        // ---------- PARPADEO DEL LED EN RB5 ----------
        if(tiempo_parpadeo > 0) {
            LATBbits.LATB5 = 1;
            delay_ms(tiempo_parpadeo);
            LATBbits.LATB5 = 0;
            delay_ms(tiempo_parpadeo);
        } else {
            LATBbits.LATB5 = 0;
        }
    }
}*/