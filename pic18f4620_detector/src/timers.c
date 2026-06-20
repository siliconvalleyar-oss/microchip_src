#include "timers.h"  // Header de temporización
#include "eeprom.h"  // Gestión de EEPROM

#pragma warning disable 520  // Desactiva warning 520 (función sin prototipo)

#define TONEOFFSET 110  // Offset base para el cálculo del tono PWM

static word ADC_GetValue(uint8_t channel)  // Lee el valor del ADC en el canal especificado
{
    ADCON0bits.CHS = channel;  // Selecciona el canal analógico
    __delay_us(10);            // Espera 10µs para estabilizar el capacitor de muestreo
    ADCON0bits.GO = 1;         // Inicia la conversión AD
    while (ADCON0bits.GO);     // Espera a que la conversión termine
    return ((word)ADRESH << 8) | ADRESL;  // Combina los registros ADRESH y ADRESL en un word de 16 bits
}

// Variables volátiles para la ISR y comunicación con el programa principal
volatile uint8_t contador100ms;       // Se incrementa cada 100ms
volatile uint8_t contador10ms;        // Se incrementa cada 10ms
volatile uint8_t contadordecontador;  // Contador auxiliar
volatile uint8_t TimeoutMenu;         // Cuenta regresiva para timeout del menú
volatile uint8_t FlagMenuOFF;         // Bandera: 1 = ocultar menú por timeout
volatile uint8_t ContadorF;           // Conteo de ciclos para detección ferrosa
volatile uint8_t ContadorNF;          // Conteo de ciclos para detección no ferrosa
volatile uint8_t ContadorM;           // Contador de muestras para promedio
volatile uint8_t Contador10seg;       // Contador de intervalos de 10 segundos
volatile uint8_t Flag10seg;           // Bandera de estado para evento de 10 segundos
volatile uint8_t Contador500Hz;       // Contador de eventos a 500Hz
volatile uint8_t FlagCalcular;        // Bandera: 1 = calcular segmentos en main loop

volatile uint8_t Segmentos;           // Número de segmentos detectados (intensidad de señal)
volatile uint8_t SegmentosMostrados;  // Segmentos actualmente mostrados en el display
volatile uint8_t Mute;                // Estado de mute: 1 = silenciado, 0 = sonido activo
volatile uint8_t Tone;                // Frecuencia base del tono de audio
volatile uint8_t OffsetTono;          // Offset adicional para ajuste fino del tono
volatile uint8_t Nofuncional;         // Bandera: 1 = modo no funcional (pausa)

volatile word Medicion;               // Acumulador de lecturas ADC para promediar
volatile word ValorCalculado;         // Último valor calculado del ADC (promediado)
volatile word ADCBateria;             // Última lectura ADC del nivel de batería

void initTIM2(void)  // Configura e inicia el Timer2 para interrupciones periódicas
{
    T2CON = 0x07;            // Prescaler 1:16, postscaler 1:16, Timer2 habilitado
    TMR2 = 0;                // Limpia el registro del timer
    PR2 = 249;               // Período: 250 ciclos -> interrupción cada 1ms (aprox)
    PIR1bits.TMR2IF = 0;     // Limpia la bandera de interrupción del Timer2
    PIE1bits.TMR2IE = 1;     // Habilita la interrupción del Timer2
    IPR1bits.TMR2IP = 1;     // Prioridad alta para la interrupción del Timer2
    T2CONbits.TMR2ON = 1;    // Enciende el Timer2
}

void GestionBobina(void)  // Gestiona el pulso de la bobina y la lectura del ADC
{
    if (Nofuncional == 0) {         // Solo opera si el sistema está en modo funcional
        PULSO_BOBINA = 0;           // Activa el pulso de la bobina (activo bajo)
        __delay_us(48);             // Espera 48µs para estabilizar la bobina
        PULSO_BOBINA = 1;           // Desactiva el pulso de la bobina

        // Ajusta el tiempo de sampleo según la posición configurada
        if (SamplePosition == 1) __delay_us(85);   // Sample en posición 1
        if (SamplePosition == 2) __delay_us(90);   // Sample en posición 2
        if (SamplePosition == 3) __delay_us(95);   // Sample en posición 3
        if (SamplePosition == 4) __delay_us(100);  // Sample en posición 4 (por defecto)
        if (SamplePosition == 5) __delay_us(105);  // Sample en posición 5
        if (SamplePosition == 6) __delay_us(110);  // Sample en posición 6
        if (SamplePosition == 7) __delay_us(115);  // Sample en posición 7
        SAMPLE = 0;                 // Activa la señal de sample (activo bajo)
        __delay_us(48);             // Espera 48µs para estabilizar
        SAMPLE = 1;                 // Desactiva la señal de sample

        if (Flag10seg == 2) {               // Si es el momento de medir batería
            ADCBateria = ADC_GetValue(ADC_CH_BATERIA);  // Lee el canal de batería
            Flag10seg = 3;                  // Avanza al siguiente estado
        } else {                            // Si no, mide la señal del detector
            Medicion += ADC_GetValue(ADC_CH_SENHAL);  // Acumula la lectura de señal
        }

        if (Flag10seg == 1) {  // Si la bandera indica inicio de ciclo de 10 segundos
            Flag10seg = 2;     // Avanza al estado de medición de batería
        }

        ContadorM++;                  // Incrementa el contador de muestras
        if (ContadorM >= 40) {        // Cuando se acumulan 40 muestras
            ContadorM = 0;            // Reinicia el contador de muestras
            ValorCalculado = Medicion / 40;  // Calcula el promedio de 40 muestras
            Medicion = 0;             // Reinicia el acumulador de mediciones
            FlagCalcular = 1;         // Activa la bandera para procesar en main loop
        }
    } else {                    // Si está en modo no funcional
        PULSO_BOBINA = 1;       // Mantiene la bobina desactivada
    }
}

void __interrupt() isr(void)  // Rutina de servicio de interrupción principal
{
    if (PIR1bits.TMR2IF) {          // Verifica que la interrupción sea del Timer2
        PIR1bits.TMR2IF = 0;        // Limpia la bandera de interrupción del Timer2

        Contador500Hz++;            // Incrementa contador de 500Hz
        if (Contador500Hz > 1) {    // Cada 2 interrupciones (~2ms)
            Contador500Hz = 0;      // Reinicia el contador
            GestionBobina();        // Ejecuta la gestión de la bobina (~500Hz)
        }

        contador100ms++;            // Incrementa contador de 100ms
        if (contador100ms >= 100) { // Cuando alcanza 100 (100ms reales)
            contador100ms = 0;      // Reinicia el contador de 100ms
            Contador10seg++;        // Incrementa contador de 10 segundos

            ContadorF++;            // Incrementa contador de detección ferrosa
            ContadorNF++;           // Incrementa contador de detección no ferrosa
            if (ContadorF > 250) ContadorF = 250;    // Satura a 250
            if (ContadorNF > 250) ContadorNF = 250;  // Satura a 250

            if (Contador10seg >= 100) {  // Cuando pasan 10 segundos
                Contador10seg = 0;       // Reinicia el contador de 10 segundos
                Flag10seg = 1;           // Activa bandera de evento de 10 segundos
            }

            if (TimeoutMenu != 0) {     // Si el timeout del menú está activo
                TimeoutMenu--;          // Decrementa el contador de timeout
                if (TimeoutMenu == 0) FlagMenuOFF = 1;  // Si llega a cero, oculta el menú
            }
        }
    }
}

void TonoON(void)  // Activa la generación de tono PWM
{
    CCP1CON = 0x3C;          // Configura CCP1 como PWM de 10 bits
    PR2 = 249;               // Período del PWM (base para la frecuencia)
    T2CONbits.TMR2ON = 1;    // Asegura que Timer2 esté encendido
    TRISCbits.TRISC2 = 0;    // Pin RC2 como salida (PWM de audio)
    LATCbits.LATC2 = 0;      // Inicia en nivel bajo
}

void TonoOFF(void)  // Desactiva la generación de tono PWM
{
    CCP1CON = 0;             // Deshabilita el módulo CCP1
    LATCbits.LATC2 = 0;      // Pin de audio a nivel bajo
}

void AjustarPWM(word valor, uint8_t offset)  // Ajusta el ciclo de trabajo del PWM
{
    word ToneOffset;            // Variable temporal para el offset calculado
    ToneOffset = 3 * OffsetTono + 10;  // Calcula el offset basado en OffsetTono
    valor = valor + offset + ToneOffset;  // Suma el valor, offset de tono y offset de tono global
    if (valor > 700) valor = 700;  // Limita el valor máximo del PWM a 700
    if (Mute == 1) valor = 0;      // Si está en mute, fuerza el valor a cero (sin sonido)
    CCPR1L = (uint8_t)(valor >> 2);       // Carga los 8 bits más significativos del duty cycle
    CCP1CONbits.DC1B = (uint8_t)(valor & 0x03);  // Carga los 2 bits menos significativos
}
