// PIC18F4620 - Regenerador de señal con frecuencia variable en RB3 (pulsador RA5)
// Comparador C1: RA2=señal(3V max) vs RA3=Vref(2.5V) ? salida RA4
// RA4 conectado fisicamente con cable corto a RC1
// RC2 (CCP1) = señal replicada en 5V, misma frecuencia
// RB3 = frecuencia ajustable por pulsador (2.3kHz a 4kHz, paso 150Hz)
// RB5 = LED (ON = señal presente)

#include <xc.h>
#pragma config OSC = HS, WDT = OFF, LVP = OFF, DEBUG = OFF
#define _XTAL_FREQ 20000000

// Timer1 free-running a 5MHz (Fosc/4, prescaler 1:1) ? tick = 0.2 µs
// CCP1 Compare/Toggle: toggle en RC2 cada half_period ticks
// freq = 5.000.000 / (2 × half_period)  ?  half_period = 2.500.000 / freq

volatile unsigned int  period_ticks = 0;
volatile unsigned char new_period   = 0;
volatile unsigned char signal_lost  = 1;
volatile unsigned int  timeout_cnt  = 0;
volatile unsigned char toggle_rb3   = 0;
volatile unsigned int  half_period  = 2500;  // default 1 kHz (para CCP1)

// Variables para la frecuencia variable de RB3 (Timer0)
volatile unsigned char rb3_reload_h = 0xFB;   // 2.3 kHz inicial: 64450 ? 0xFBC2
volatile unsigned char rb3_reload_l = 0xC2;
unsigned int current_rb3_freq = 2300;         // arranca en 2.3 kHz

// Calcula los registros de recarga de Timer0 para una frecuencia dada en Hz
void set_rb3_freq(unsigned int freq_hz) {
    if (freq_hz < 100) freq_hz = 100;
    if (freq_hz > 10000) freq_hz = 10000;
    // ticks = (Fosc/4) / (2 * freq) = 5e6 / (2*freq) = 2.5e6 / freq
    unsigned int ticks = 2500000UL / freq_hz;
    if (ticks < 1) ticks = 1;
    if (ticks > 65535) ticks = 65535;
    unsigned int reload = 65536 - ticks;
    rb3_reload_h = (reload >> 8) & 0xFF;
    rb3_reload_l = reload & 0xFF;
}

void __interrupt() isr(void)
{
    // ?? CCP1: Compare/Toggle - actualizar CCPR1 para el próximo medio ciclo
    if (PIR1bits.CCP1IF)
    {
        unsigned int next = ((unsigned int)CCPR1H << 8) | CCPR1L;
        next += half_period;
        CCPR1H = (unsigned char)(next >> 8);
        CCPR1L = (unsigned char)(next & 0xFF);
        PIR1bits.CCP1IF = 0;
    }

    // ?? CCP2: captura periodo de la señal en RC1
    if (PIR2bits.CCP2IF)
    {
        static unsigned int prev = 0;
        unsigned int curr = ((unsigned int)CCPR2H << 8) | CCPR2L;
        if (prev != 0) {
            unsigned int diff = curr - prev;
            if (diff >= 250 && diff <= 50000) {
                period_ticks = diff;
                new_period   = 1;
            }
        }
        prev        = curr;
        signal_lost = 0;
        timeout_cnt = 0;
        PIR2bits.CCP2IF = 0;
    }

    // ?? Timer0: genera frecuencia variable en RB3 + timeout 500 ms
    if (INTCONbits.TMR0IF)
    {
        // Recarga con los valores calculados según la frecuencia deseada
        TMR0H = rb3_reload_h;
        TMR0L = rb3_reload_l;
        toggle_rb3 ^= 1;
        LATBbits.LATB3 = toggle_rb3;
        if (!signal_lost && (++timeout_cnt >= 1000))
            signal_lost = 1;
        INTCONbits.TMR0IF = 0;
    }
}

void set_frecuencia_salida(unsigned int freq_hz)
{
    if (freq_hz < 100 || freq_hz > 20000) return;
    unsigned int hp = (unsigned int)(2500000UL / (unsigned long)freq_hz);
    if (hp < 1) hp = 1;

    INTCONbits.GIE = 0;
    half_period = hp;
    unsigned int now  = ((unsigned int)TMR1H << 8) | TMR1L;
    unsigned int next = now + hp;
    CCPR1H = (unsigned char)(next >> 8);
    CCPR1L = (unsigned char)(next & 0xFF);
    PIR1bits.CCP1IF = 0;
    INTCONbits.GIE = 1;
}

void main(void)
{
    // Test: 3 parpadeos RB5
    TRISBbits.TRISB5 = 0;
    LATBbits.LATB5   = 0;
    for (int i = 0; i < 3; i++) {
        LATBbits.LATB5 = 1; __delay_ms(150);
        LATBbits.LATB5 = 0; __delay_ms(150);
    }

    // Configurar pines digitales/analógicos
    ADCON1 = 0x09;   // RA0-RA3 analógicos, RA4 y resto digitales

    TRISAbits.TRISA2 = 1;   // RA2 = C1IN+ (señal generador)
    TRISAbits.TRISA3 = 1;   // RA3 = C1IN- (2.5V)
    TRISAbits.TRISA4 = 0;   // RA4 = C1OUT (salida comparador ? RC1)
    TRISAbits.TRISA5 = 1;   // RA5 = pulsador (activo a nivel bajo)
    TRISCbits.TRISC1 = 1;   // RC1 = entrada CCP2
    TRISCbits.TRISC2 = 0;   // RC2 = salida CCP1 (señal replicada 5V)
    TRISBbits.TRISB3 = 0;   // RB3 = salida de frecuencia variable
    TRISBbits.TRISB5 = 0;   // RB5 = LED

    LATBbits.LATB3  = 0;
    LATCbits.LATC2  = 0;

    // Comparador C1
    CMCON = 0x02;    // Comparador independiente, C1OUT en RA4

    // Timer1: free-running, Fosc/4, prescaler 1:1, 16-bit ON
    T1CON = 0x81;

    // CCP2: captura flanco de subida
    CCP2CON = 0x05;

    // CCP1: Compare Toggle, valor inicial cualquiera (se ajustará luego)
    CCP1CON = 0x02;
    set_frecuencia_salida(1000);  // valor inicial mientras no hay señal

    // Timer0: 16-bit, Fosc/4, prescaler 1:1, inicialmente apagado para cargar
    T0CON = 0x88;    // 16-bit, reloj interno, prescaler 1:1, aún sin arrancar
    // Cargar valores para 2.3 kHz (valor por defecto)
    set_rb3_freq(2300);
    TMR0H = rb3_reload_h;
    TMR0L = rb3_reload_l;
    T0CONbits.TMR0ON = 1;   // arrancar Timer0

    // Interrupciones
    RCONbits.IPEN     = 0;
    PIE1bits.CCP1IE   = 1; PIR1bits.CCP1IF  = 0;
    PIE2bits.CCP2IE   = 1; PIR2bits.CCP2IF  = 0;
    INTCONbits.TMR0IE = 1; INTCONbits.TMR0IF = 0;
    INTCONbits.PEIE   = 1;
    INTCONbits.GIE    = 1;

    unsigned int last_freq = 0;

    // Variables para debounce del pulsador (activo bajo)
    unsigned char button_pressed = 0;
    unsigned char debounce = 0;

    while (1)
    {
        // Detectar y procesar el pulsador RA5
        if (!PORTAbits.RA5) {          // pulsador presionado (nivel bajo)
            if (!debounce) {
                __delay_ms(20);        // esperar a que se estabilice
                if (!PORTAbits.RA5) {  // confirmar
                    // Incrementar frecuencia
                    current_rb3_freq += 150;
                    if (current_rb3_freq > 4000)
                        current_rb3_freq = 2300;
                    // Actualizar registros de Timer0
                    INTCONbits.GIE = 0;
                    set_rb3_freq(current_rb3_freq);
                    // No es necesario recargar TMR0 ahora, la ISR usará los nuevos valores
                    INTCONbits.GIE = 1;
                    debounce = 1;
                }
            }
        } else {
            debounce = 0;   // reset al soltar
        }

        // Procesar captura de periodo de la señal externa
        if (new_period)
        {
            new_period = 0;
            unsigned int ticks = period_ticks;
            if (ticks >= 250 && ticks <= 50000)
            {
                unsigned int freq = (unsigned int)(5000000UL / (unsigned long)ticks);
                if (freq >= 100 && freq <= 20000 && freq != last_freq)
                {
                    last_freq = freq;
                    set_frecuencia_salida(freq);
                }
            }
        }

        // Manejo de pérdida de señal
        if (signal_lost)
        {
            CCP1CON        = 0x00;   // Apagar salida CCP1
            LATCbits.LATC2 = 0;
            LATBbits.LATB5 = 0;      // LED apagado
            last_freq      = 0;
        }
        else
        {
            if (CCP1CON == 0x00)     // Reactivar CCP1 cuando vuelve la señal
            {
                CCP1CON = 0x02;
                set_frecuencia_salida(last_freq > 0 ? last_freq : 1000);
            }
            LATBbits.LATB5 = 1;      // LED encendido
        }
    }
}