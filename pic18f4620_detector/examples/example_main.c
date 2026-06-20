#include "config.h"      // Configuración general y definiciones del sistema
#include "eeprom.h"      // Gestión de memoria EEPROM
#include "timers.h"      // Funciones de temporización e interrupciones
#include "rutinasMisc.h" // Funciones misceláneas (LCD, retardos, etc.)

// Variables globales del programa principal
uint8_t Botones, Indice;            // Estado de botones leídos, índice de bucle
uint8_t CicloDeteccion;             // Ciclo de detección actual
uint8_t SegmentosAnterior, SegmentosTemp;  // Segmentos previos y temporales
uint8_t batres, APAGAR;             // Resultado de batería, bandera de apagado
uint8_t FlagAutoReset;              // Bandera de auto-reset (0=inactivo, 1=activo, 2=ejecutar)
uint8_t MuestrasPromedio;           // Número de muestras para el promedio
uint8_t target1, target2, target2anterior;  // Variables objetivo para detección

word ValorTemp;             // Valor temporal para cálculos
word TempSegmentos;         // Valor temporal de segmentos
word TIMERTARGET;           // Valor objetivo del timer
word ValoresAnteriores[8];  // Buffer circular de valores anteriores para promediado
word ValorMinimo, ValorMaximo, ValorRuido;  // Valores de referencia para detección
word ValorTemporal;         // Valor temporal adicional

void CalcularSegmentos(void)  // Calcula el número de segmentos para el vúmetro
{
    ValorTemp = ValorCalculado;              // Toma el último valor calculado del ADC
    if (ValorTemp > ValorZero) ValorTemp = 0;  // Si es mayor que el cero, descarta (ruido)
    else ValorTemp = ValorZero - ValorTemp;     // Si no, calcula la diferencia positiva
    MuestrasPromedio = 4;                       // Configura 4 muestras para promediar
    for (Indice = 0; Indice < (MuestrasPromedio - 1); Indice++)  // Desplaza buffer circular
        ValoresAnteriores[Indice] = ValoresAnteriores[Indice + 1];
    ValoresAnteriores[MuestrasPromedio - 1] = ValorTemp;  // Agrega nuevo valor al buffer
    ValorTemp = 0;                              // Reinicia acumulador
    for (Indice = 0; Indice < MuestrasPromedio; Indice++)  // Suma todas las muestras
        ValorTemp += ValoresAnteriores[Indice];
    ValorTemp /= MuestrasPromedio;              // Promedia las muestras
    ValorTemp = ValorTemp * Ganancia / 2;       // Aplica ganancia (divide entre 2)
    if (ValorTemp > (Discrimination * 10));     // Si supera el umbral de discriminación
    else ValorTemp = 0;                         // Si no, descarta (es ruido)
    ValorDefinitivo = ValorTemp;                // Almacena el valor definitivo
    SegmentosTemp = (uint8_t)(ValorTemp / 10);  // Convierte a número de segmentos
    Segmentos = SegmentosTemp;                  // Actualiza variable global de segmentos
    if (Segmentos > 40) Segmentos = 40;         // Limita a 40 segmentos máximo
    if (Segmentos > 8) FlagAutoReset = 1;       // Si hay señal fuerte, activa auto-reset
    if ((Segmentos < 6) && (FlagAutoReset == 1)) FlagAutoReset = 2;  // Si baja, ejecuta reset
}

void AccionReset(void)  // Ejecuta la acción de reset y auto-ajuste del detector
{
    uint8_t tempo;          // Variable temporal
    Segmentos = Tone;       // Guarda momentáneamente el tono en segmentos
    Tone = 0;               // Desactiva el tono temporalmente
    AjustarPWM(0, Tone);    // Ajusta PWM a cero
    Tone = Segmentos;       // Restaura el tono
    Segmentos = 0;          // Reinicia los segmentos
    Menu = 0;               // Sale del menú
    DibujarVumetro(Menu);   // Redibuja el vúmetro
    Menu = 10;              // Valor temporal para ocultar barras
    ENVIARCOMANDO(0xA1);    // Comando LCD: posiciona cursor
    ENVIARDATO('Z'); ENVIARDATO('E'); ENVIARDATO('R'); ENVIARDATO('O');  // Muestra "ZERO"

    // Ajusta la sensibilidad para que ValorCalculado esté entre 400 y 800
    while ((ValorCalculado < 400) && (Sensibilidad < 45)) {  // Si señal muy baja, aumenta sensibilidad
        Sensibilidad++;             // Incrementa sensibilidad
        SENSIB_MAS();               // Envía pulso de incremento
        if (Debug == 1) MostrarAjuste(Sensibilidad);  // Muestra ajuste en debug
        retardo50ms();              // Espera 50ms para estabilizar
        if (Debug == 1) { ENVIARCOMANDO(0xCC); MostrarWord(ValorCalculado); }  // Muestra valor
    }
    while ((ValorCalculado > 800) && (Sensibilidad != 0)) {  // Si señal muy alta, disminuye sensibilidad
        Sensibilidad--;             // Decrementa sensibilidad
        SENSIB_MENOS();             // Envía pulso de decremento
        if (Debug == 1) MostrarAjuste(Sensibilidad);
        retardo50ms();
        if (Debug == 1) { ENVIARCOMANDO(0xCC); MostrarWord(ValorCalculado); }
    }
    if ((ValorCalculado < 400) && (Sensibilidad < 45)) {  // Corrección fina si aún baja
        Sensibilidad++; SENSIB_MAS();
        if (Debug == 1) MostrarAjuste(Sensibilidad);
    }
    if ((ValorCalculado > 800) && (Sensibilidad != 0)) {  // Corrección fina si aún alta
        Sensibilidad--; SENSIB_MENOS();
        retardo50ms(); retardo50ms();  // Doble retardo para estabilizar
        if (Debug == 1) MostrarAjuste(Sensibilidad);
    }
    if ((ValorCalculado > 800) && (Sensibilidad != 0)) {  // Segunda corrección si persiste
        Sensibilidad--; SENSIB_MENOS();
        retardo50ms();
        if (Debug == 1) MostrarAjuste(Sensibilidad);
    }
    if (Debug == 1) MostrarAjuste(Sensibilidad);  // Muestra sensibilidad final en debug
    CalcularZero();  // Recalcula el valor de referencia cero
}

void main(void)  // Función principal del programa
{
    uint8_t boton_anterior = 0xFF;  // Estado anterior de todos los botones (todos inactivos)
    uint8_t botones_leidos;          // Variable para lectura de botones
    uint8_t i;                       // Índice de bucle general

    Nofuncional = 1;                 // Inicia en modo no funcional
    Mute = 1;                        // Mute activado al inicio
    initMCU();                       // Inicializa el microcontrolador (puertos, ADC, PLL)
    FORZAR_VOLUMEN(0);               // Volumen a cero inicialmente
    retardo100();                    // Espera 100ms para estabilizar
    if (KEY_SENSE == 1) {            // Si se detecta pulsación de encendido
        RETENCION_POWER = 1;         // Activa el latch de retención de alimentación
    } else {
        RETENCION_POWER = 0;         // No retiene alimentación
        while (RETENCION_POWER == 0) RETENCION_POWER = 0;  // Espera a que se active
    }
    INITLCD();                       // Inicializa el LCD en modo 4 bits
    FORZAR_VOLUMEN(0);               // Vuelve a forzar volumen a cero
    Presentacion();                  // Muestra pantalla de presentación
    retardo1s();                     // Espera 1 segundo
    while (KEY_SENSE == 1) {}        // Espera a que se suelte el botón de encendido
    INITLCD();                       // Reinicializa el LCD
    initTIM2();                      // Inicializa el Timer2 (interrupciones periódicas)
    INTCONbits.GIE = 1;              // Habilita interrupciones globales
    INTCONbits.PEIE = 1;             // Habilita interrupciones periféricas

    Debug = 0;                       // Modo debug desactivado por defecto
    if ((BOTON_MENOS == 0) && (BOTON_RESET == 0)) {  // Si Menos y Reset presionados al iniciar
        Debug = 1;                   // Activa modo debug
        ENVIARCOMANDO(0xA1);         // Posiciona cursor
        ENVIARDATO('D'); ENVIARDATO('E'); ENVIARDATO('B'); ENVIARDATO('U');
        ENVIARDATO('G');             // Muestra "DEBUG"
        ENVIARCOMANDO(0x96);         // Posiciona cursor
        ENVIARDATO('S'); ENVIARDATO('w'); ENVIARDATO(':'); ENVIARDATO('3');
        ENVIARDATO('.'); ENVIARDATO('0');  // Muestra "Sw:3.0"
        retardo1s(); retardo1s(); retardo1s(); retardo1s();  // Espera 4 segundos
        ENVIARCOMANDO(0xA1);         // Limpia línea
        for (i = 0; i < 5; i++) ENVIARDATO(' ');  // Espacios de limpieza
        ENVIARCOMANDO(0x96);         // Posiciona cursor
        for (i = 0; i < 6; i++) ENVIARDATO(' ');  // Espacios de limpieza
    }

    VerEEPROM();                     // Verifica y carga valores desde EEPROM
    Medicion = 0;                    // Reinicia acumulador de medición
    Backlight = 1;                   // Backlight encendido
    Menu = 0;                        // Menú principal (modo detector)
    SegmentosMostrados = 0;          // Sin segmentos mostrados al inicio
    Mute = 1;                        // Mute activado
    ValorZero = 10;                  // Valor cero inicial por defecto
    Sensibilidad = SENSIBILIDAD_DEFAULT;  // Sensibilidad por defecto
    Discrimination = DISCRIMINATION_DEFAULT;  // Discriminación por defecto
    FORZAR_VOLUMEN(Volumen);         // Aplica el volumen cargado de EEPROM
    FORZAR_SENSIBILIDAD(Sensibilidad);  // Aplica la sensibilidad cargada
    MostrarVolumen(Volumen);         // Muestra el volumen en LCD
    if (Debug == 1) MostrarAjuste(Sensibilidad);  // Muestra sensibilidad si debug
    MostrarGanancia(Ganancia);       // Muestra la ganancia en LCD
    MostrarDiscrimination(SamplePosition);  // Muestra discriminación en LCD
    MostrarTone(Tone - TONEINITIAL);  // Muestra el tono en LCD
    MostrarTitulo(Menu);             // Muestra el título del menú
    TonoON();                        // Activa la generación de tono PWM
    Flag10seg = 1;                   // Inicia bandera de 10 segundos
    Nofuncional = 0;                 // Activa modo funcional
    APAGAR = 0;                      // Apagado no solicitado

    for (;;) {  // Bucle principal infinito
        botones_leidos = 0xFF;       // Inicializa con todos los bits en 1 (botones no presionados)
        if (Menu == 0) {             // Solo en modo detector (menú 0)
            if (FlagCalcular == 1) { // Si hay un nuevo valor calculado disponible
                FlagCalcular = 0;    // Limpia la bandera
                CalcularSegmentos(); // Procesa el valor y calcula segmentos
                AjustarPWM(ValorDefinitivo, Tone);  // Ajusta el PWM según el valor
            }
            if (Debug == 1) {        // En modo debug, muestra valores en tiempo real
                ENVIARCOMANDO(0xC6);  // Posiciona cursor para primer valor
                MostrarWord(ValorDefinitivo);  // Muestra valor definitivo
                ENVIARCOMANDO(0xCC);  // Posiciona cursor para segundo valor
                MostrarWord(ValorRuido);  // Muestra valor de ruido
            }
            if (SegmentosMostrados != Segmentos) {  // Si los segmentos cambiaron
                if (SegmentosMostrados < Segmentos) {  // Si hay más segmentos (subiendo)
                    SegmentosMostrados++;               // Incrementa gradualmente
                    if ((Segmentos - SegmentosMostrados) > 2) SegmentosMostrados++;  // Salto doble si lejos
                } else {                                // Si hay menos segmentos (bajando)
                    SegmentosMostrados--;               // Decrementa gradualmente
                    if ((SegmentosMostrados - Segmentos) > 2) SegmentosMostrados--;  // Salto doble
                }
                if (SegmentosMostrados < 4) {  // Menos de 4 segmentos: sin detección
                    LED_FERROSO = 0;            // Apaga LED ferroso
                    LED_NO_FERROSO = 0;         // Apaga LED no ferroso
                    ContadorF = 0;              // Reinicia contador ferroso
                    ContadorNF = 0;             // Reinicia contador no ferroso
                }
                if (SegmentosMostrados < 32) {  // Menos de 32 segmentos
                    LED_NO_FERROSO = 0;          // Apaga LED no ferroso
                    ContadorNF = 0;              // Reinicia contador no ferroso
                }
                if (ContadorF > 2) LED_FERROSO = 1;    // Si hay suficientes muestras ferrosas, enciende LED
                if (ContadorF > 6) LED_FERROSO = 0;    // Si pasan muchas muestras, apaga
                if (ContadorNF > 4) { LED_FERROSO = 0; LED_NO_FERROSO = 1; }  // Detección no ferrosa
                if (ContadorNF > 6) { LED_FERROSO = 0; LED_NO_FERROSO = 0; }  // Apaga ambos
                DibujarVumetro(Menu);  // Redibuja el vúmetro en el LCD
            }
        }

        // Lectura de botones (activos en bajo, se enmascaran los bits)
        if (BOTON_MUTE == 0) botones_leidos &= ~BOTON_MUTE_MASK;   // Botón Mute presionado
        if (BOTON_RESET == 0) botones_leidos &= ~BOTON_RESET_MASK; // Botón Reset presionado
        if (BOTON_MAS == 0) botones_leidos &= ~BOTON_MAS_MASK;     // Botón Más presionado
        if (BOTON_MENOS == 0) botones_leidos &= ~BOTON_MENOS_MASK; // Botón Menos presionado
        if (BOTON_MENU == 0) botones_leidos &= ~BOTON_MENU_MASK;   // Botón Menú presionado
        if (BOTON_LIGHT == 0) botones_leidos &= ~BOTON_LIGHT_MASK; // Botón Luz presionado
        Botones = botones_leidos;  // Almacena el estado de botones en variable global

        // Auto Reset
        if (FlagAutoReset == 2) {   // Si se debe ejecutar auto-reset
            FlagAutoReset = 0;      // Limpia la bandera
            AccionReset();          // Ejecuta la acción de reset
            Menu = 0;               // Vuelve al menú principal
            ContadorM = 0;          // Reinicia contador de muestras
        }

        // Boton MAS
        if ((Botones & BOTON_MAS_MASK) == 0) {  // Si el botón Más está presionado
            if (boton_anterior & BOTON_MAS_MASK) {  // Y antes no lo estaba (flanco de bajada)
                SubirVariable();                // Incrementa la variable del menú actual
                if (Menu != 0) TimeoutMenu = TIEMPOTIMEOUTMENU;  // Reinicia timeout si no está en menú 0
                DibujarVumetro(Menu);           // Redibuja vúmetro
                MostrarTitulo(Menu);            // Muestra título del menú
                retardo100();                   // Espera 100ms (antirrebote)
                retardo100();                   // Otro retardo
            }
        }

        // Boton MENOS
        if ((Botones & BOTON_MENOS_MASK) == 0) {  // Si el botón Menos está presionado
            if (boton_anterior & BOTON_MENOS_MASK) {  // Y antes no lo estaba
                BajarVariable();                 // Decrementa la variable del menú actual
                if (Menu != 0) TimeoutMenu = TIEMPOTIMEOUTMENU;
                DibujarVumetro(Menu);
                MostrarTitulo(Menu);
                retardo100();
                retardo100();
            }
        }

        // Boton RESET
        if ((Botones & BOTON_RESET_MASK) == 0) {  // Si el botón Reset está presionado
            if (boton_anterior & BOTON_RESET_MASK) {  // Y antes no lo estaba
                AccionReset();                   // Ejecuta reset/auto-ajuste
                RecargarValoresDefault();        // Verifica si se cargan defaults (5s presionado)
                Menu = 0;                        // Vuelve al menú principal
                ContadorM = 0;                   // Reinicia contador de muestras
            }
        }

        // Boton MENU
        if ((Botones & BOTON_MENU_MASK) == 0) {  // Si el botón Menú está presionado
            if (boton_anterior & BOTON_MENU_MASK) {  // Y antes no lo estaba
                Menu++;                          // Avanza al siguiente menú
                TimeoutMenu = TIEMPOTIMEOUTMENU;  // Reinicia timeout
                if (Menu == 1) Menu = 2;          // Salta menú 1 (no usado)
                if (Debug) {                       // En modo debug hay un menú extra
                    if (Menu == 7) { Menu = 0; FlagMenuOFF = 1; }  // Vuelve a inicio
                } else {
                    if (Menu == 6) { Menu = 0; FlagMenuOFF = 1; }  // Vuelve a inicio
                }
                MostrarTitulo(Menu);             // Muestra título del nuevo menú
                DibujarVumetro(Menu);            // Dibuja vúmetro según el menú
                retardo100();
                retardo100();
            }
        }

        // Boton LIGHT
        if ((Botones & BOTON_LIGHT_MASK) == 0) {  // Si el botón Luz está presionado
            if (boton_anterior & BOTON_LIGHT_MASK) {  // Y antes no lo estaba
                if (Backlight == 0) BACKLIGHT_ON(); else BACKLIGHT_OFF();  // Alterna backlight
                retardo100();
                retardo100();
            }
        }

        // Boton MUTE
        if ((Botones & BOTON_MUTE_MASK) == 0) {  // Si el botón Mute está presionado
            if (boton_anterior & BOTON_MUTE_MASK) {  // Y antes no lo estaba
                if (Mute == 0) Mute = 1; else Mute = 0;  // Alterna estado de mute
                MostrarMute();                   // Muestra/oculta indicador de mute
                AjustarPWM(ValorDefinitivo, Tone);  // Ajusta PWM (se silencia si mute activo)
                retardo100();
                retardo100();
            }
        }

        boton_anterior = Botones;  // Guarda el estado actual para detectar flancos en el próximo ciclo

        // Auto-off menu (timeout del menú)
        if (FlagMenuOFF == 1) {    // Si la bandera de ocultar menú está activa
            FlagMenuOFF = 0;       // Limpia la bandera
            Menu = 0;              // Vuelve al menú principal
            MostrarTitulo(Menu);   // Muestra título
            DibujarVumetro(10);    // Fuerza limpieza del vúmetro
        }

        // Battery check (cada 10 segundos)
        if (Flag10seg == 3) {      // Cuando la bandera indica que hay lectura de batería
            Flag10seg = 0;         // Limpia la bandera
            APAGAR = 0;            // Reinicia bandera de apagado
            batres = MostrarBat(); // Muestra nivel de batería y obtiene resultado
            if (batres == 2) {     // Si la batería está crítica
                ENVIARCOMANDO(0x94 + 7);  // Posiciona cursor para mensaje
                ENVIARDATO('L'); ENVIARDATO('o'); ENVIARDATO('w'); ENVIARDATO(' ');
                ENVIARDATO('B'); ENVIARDATO('a'); ENVIARDATO('t');  // Muestra "Low Bat"
                retardo1s(); retardo1s(); retardo1s();  // Espera 3 segundos
                retardo1s(); retardo1s(); retardo1s(); retardo1s();  // Espera 4 segundos más
                APAGAR = 1;         // Activa bandera de apagado
            }
        }

        // Power off (apagado por falta de alimentación o batería baja)
        if ((KEY_SENSE == 1) || (APAGAR == 1)) {  // Si se suelta el botón de encendido o batería crítica
            retardo100();                           // Antirrebote
            retardo100();                           // Antirrebote adicional
            if ((KEY_SENSE == 1) || (APAGAR == 1)) {  // Verifica que la condición persista
                FORZAR_VOLUMEN(0);                  // Volumen a cero
                Mute = 1;                           // Activa mute
                AjustarPWM(ValorDefinitivo, Tone);  // Ajusta PWM (en silencio)
                retardo100();                        // Espera 100ms
                GrabarEEPROM();                     // Guarda configuración actual en EEPROM
                Nofuncional = 1;                    // Activa modo no funcional
                T2CONbits.TMR2ON = 0;               // Apaga Timer2
                LATCbits.LATC2 = 0;                 // Pin de audio a cero
                PULSO_BOBINA = 1;                   // Bobina desactivada
                Backlight = 0;                      // Backlight apagado
                BACKLIGHT_OFF();                    // Apaga backlight
                RETENCION_POWER = 0;                // Libera retención de alimentación
                while (RETENCION_POWER == 0) {      // Espera a que se corte la alimentación
                    RETENCION_POWER = 0;            // Mantiene liberado
                    LATCbits.LATC2 = 0;             // Mantiene audio en cero
                }
            }
        }
    }
}
