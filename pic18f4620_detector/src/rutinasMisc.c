#include "rutinasMisc.h"  // Header de rutinas misceláneas
#include "eeprom.h"       // Gestión de EEPROM
#include "timers.h"       // Variables de temporización

#pragma warning disable 520  // Desactiva warning 520 (función sin prototipo)

uint8_t Debug;         // Modo debug: 1 = activado, 0 = desactivado
uint8_t Sensibilidad;  // Nivel de sensibilidad actual (0-50)
uint8_t Menu;          // Número de menú activo (0 = detector, 1-6 = menús de ajuste)
uint8_t Backlight;     // Estado del backlight: 1 = encendido, 0 = apagado

word ValorZero;         // Valor de referencia cero (línea base del suelo)
word ValorDefinitivo;   // Valor procesado final para visualización en el vúmetro

//--- Funciones auxiliares para LCD en modo 4 bits ---
static void LCD_nibble(uint8_t nibble)  // Envía un nibble (4 bits) al LCD
{
    LATD = (LATD & 0xF0) | (nibble & 0x0F);  // Coloca el nibble en los bits bajos de PORTD
    LCD_E = 1;          // Activa el pulso de Enable del LCD
    __delay_us(1);      // Espera 1µs
    LCD_E = 0;          // Desactiva el pulso de Enable (el LCD captura el dato en flanco descendente)
    __delay_us(1);      // Espera 1µs
}

void ENVIARCOMANDO(byte comando)  // Envía un comando al LCD (RS=0)
{
    LCD_RS = 0;             // Modo comando (Register Select = 0)
    LCD_nibble(comando >> 4);  // Envía primero el nibble alto
    LCD_nibble(comando);       // Luego envía el nibble bajo
    __delay_us(40);         // Espera 40µs para que el LCD procese el comando
}

void ENVIARDATO(byte dato)  // Envía un dato (carácter) al LCD (RS=1)
{
    LCD_RS = 1;             // Modo dato (Register Select = 1)
    LCD_nibble(dato >> 4);  // Envía primero el nibble alto
    LCD_nibble(dato);       // Luego envía el nibble bajo
    __delay_us(40);         // Espera 40µs para que el LCD procese el dato
}

void INITLCD(void)  // Inicializa el LCD en modo 4 bits
{
    uint8_t i, CGRAM;  // Variables para el bucle de inicialización de caracteres personalizados
    retardo50ms();     // Espera 50ms para que el LCD se estabilice después del encendido
    LCD_RS = 0;        // Modo comando
    LCD_nibble(0x03);  // Envía 0x03 tres veces para inicializar en modo 8 bits (secuencia de reset)
    __delay_ms(5);     // Espera 5ms
    LCD_nibble(0x03);  // Segundo comando de reset
    __delay_us(100);   // Espera 100µs
    LCD_nibble(0x03);  // Tercer comando de reset
    __delay_us(100);   // Espera 100µs
    LCD_nibble(0x02);  // Cambia a modo 4 bits
    __delay_us(100);   // Espera 100µs
    ENVIARCOMANDO(0x28);  // Configura: 2 líneas, matriz 5x7, modo 4 bits
    ENVIARCOMANDO(0x01);  // Limpia la pantalla
    __delay_ms(2);        // Espera 2ms (el LCD necesita tiempo para limpiar)
    ENVIARCOMANDO(0x0C);  // Display encendido, cursor apagado, parpadeo apagado
    ENVIARCOMANDO(0x06);  // Incrementa dirección automáticamente, sin desplazamiento
    CGRAM = 1;            // Selecciona la posición 1 de CGRAM
    for (i = 0; i < 8; i++) {       // Itera para crear 8 caracteres personalizados
        ENVIARCOMANDO(0x40 + CGRAM * 8 + i);  // Direcciona la posición de CGRAM
        ENVIARDATO(0x18);  // Escribe el patrón del carácter personalizado (barra vertical)
    }
}

void BACKLIGHT_ON(void)  // Enciende el backlight del LCD
{
    LCD_BACK = 1;     // Activa el pin de backlight
    Backlight = 1;    // Actualiza la variable de estado del backlight
}

void BACKLIGHT_OFF(void)  // Apaga el backlight del LCD
{
    LCD_BACK = 0;     // Desactiva el pin de backlight
    Backlight = 0;    // Actualiza la variable de estado del backlight
}

//--- Control de sensibilidad (potenciómetro digital) ---
void SENSIB_CERO(void)  // Lleva la sensibilidad al mínimo (cero pasos)
{
    byte i;                     // Variable de iteración
    SENSMA = 1; SENSME = 1;     // Asegura ambos pines en reposo (nivel alto)
    for (i = 0; i < 65; i++) SENSIB_MENOS();  // Envía 65 pulsos de decremento para asegurar mínimo
    SENSMA = 1; SENSME = 1;     // Vuelve a poner los pines en reposo
}

void SENSIB_MAS(void)  // Incrementa la sensibilidad en un paso
{
    SENSMA = 0;          // Activa el pin de incremento (activo bajo)
    retardo1ms();        // Espera 1ms
    SENSMA = 1;          // Desactiva el pin de incremento
    retardo1ms();        // Espera 1ms
}

void SENSIB_MENOS(void)  // Decrementa la sensibilidad en un paso
{
    SENSME = 0;          // Activa el pin de decremento (activo bajo)
    retardo1ms();        // Espera 1ms
    SENSME = 1;          // Desactiva el pin de decremento
    retardo1ms();        // Espera 1ms
}

void FORZAR_SENSIBILIDAD(byte valor)  // Fija la sensibilidad a un valor específico
{
    byte i;                     // Variable de iteración
    SENSIB_CERO();              // Primero lleva la sensibilidad a cero
    for (i = 0; i < valor; i++) SENSIB_MAS();  // Luego incrementa hasta el valor deseado
}

//--- Control de volumen (potenciómetro digital) ---
void VOLUMEN_CERO(void)  // Lleva el volumen al mínimo (cero)
{
    byte i;                     // Variable de iteración
    VOLUMA = 1; VOLUME = 1;     // Asegura ambos pines en reposo
    for (i = 0; i < 65; i++) VOLUMEN_MENOS();  // Envía 65 pulsos de decremento
    VOLUMA = 1; VOLUME = 1;     // Vuelve a poner los pines en reposo
}

void VOLUMEN_MAS(void)  // Incrementa el volumen en un paso
{
    VOLUMA = 0;          // Activa el pin de incremento
    retardo1ms();        // Espera 1ms
    VOLUMA = 1;          // Desactiva el pin de incremento
    retardo1ms();        // Espera 1ms
}

void VOLUMEN_MENOS(void)  // Decrementa el volumen en un paso
{
    VOLUME = 0;          // Activa el pin de decremento
    retardo1ms();        // Espera 1ms
    VOLUME = 1;          // Desactiva el pin de decremento
    retardo1ms();        // Espera 1ms
}

void FORZAR_VOLUMEN(byte valor)  // Fija el volumen a un valor específico
{
    byte i;                     // Variable de iteración
    VOLUMEN_CERO();             // Primero lleva el volumen a cero
    for (i = 0; i < valor; i++) VOLUMEN_MAS();  // Luego incrementa hasta el valor deseado
}

//--- Funciones de visualización en LCD ---
void MostrarVolumen(uint8_t Numero)  // Muestra el nivel de volumen en el LCD
{
    uint8_t Decena;                         // Almacena el dígito de las decenas
    ENVIARCOMANDO(0xDC);                    // Posiciona el cursor en la línea 2, columna 4
    ENVIARDATO('V'); ENVIARDATO('o'); ENVIARDATO('l'); ENVIARDATO(126);  // Muestra "Vol" con flecha
    Decena = (uint8_t)(Numero / 10); Numero %= 10;  // Extrae decenas y unidades
    if (Decena == 0) { ENVIARDATO((byte)(Numero + 0x30)); ENVIARDATO(' '); }  // Si es 1 dígito
    else { ENVIARDATO((byte)(Decena + 0x30)); ENVIARDATO((byte)(Numero + 0x30)); }  // 2 dígitos
}

void MostrarGanancia(uint8_t Numero)  // Muestra el nivel de ganancia en el LCD
{
    uint8_t Decena;                         // Almacena el dígito de las decenas
    ENVIARCOMANDO(0x80);                    // Posiciona el cursor en la línea 1, columna 0
    ENVIARDATO('S'); ENVIARDATO('e'); ENVIARDATO('n'); ENVIARDATO(126);  // Muestra "Sen" con flecha
    Decena = (uint8_t)(Numero / 10); Numero %= 10;  // Extrae decenas y unidades
    if (Decena == 0) { ENVIARDATO((byte)(Numero + 0x30)); ENVIARDATO(' '); }
    else { ENVIARDATO((byte)(Decena + 0x30)); ENVIARDATO((byte)(Numero + 0x30)); }
}

void MostrarTone(uint8_t Numero)  // Muestra la frecuencia del tono en el LCD
{
    uint8_t Decena;                         // Almacena el dígito de las decenas
    ENVIARCOMANDO(0xD4);                    // Posiciona el cursor en la línea 2, columna 4
    ENVIARDATO('F'); ENVIARDATO('r'); ENVIARDATO('q'); ENVIARDATO(126);  // Muestra "Frq" con flecha
    Decena = (uint8_t)(Numero / 10); Numero %= 10;
    if (Decena == 0) { ENVIARDATO((byte)(Numero + 0x30)); ENVIARDATO(' '); }
    else { ENVIARDATO((byte)(Decena + 0x30)); ENVIARDATO((byte)(Numero + 0x30)); }
}

void MostrarDiscrimination(uint8_t Numero)  // Muestra el nivel de discriminación en el LCD
{
    uint8_t Decena;
    ENVIARCOMANDO(0x88);                    // Posiciona el cursor en la línea 1, columna 8
    ENVIARDATO('D'); ENVIARDATO('i'); ENVIARDATO('s'); ENVIARDATO(126);  // Muestra "Dis" con flecha
    Decena = (uint8_t)(Numero / 10); Numero %= 10;
    if (Decena == 0) { ENVIARDATO((byte)(Numero + 0x30)); ENVIARDATO(' '); }
    else { ENVIARDATO((byte)(Decena + 0x30)); ENVIARDATO((byte)(Numero + 0x30)); }
}

void MostrarSample(uint8_t Numero)  // Muestra la posición de sample en el LCD (alias de discrimination)
{
    MostrarDiscrimination(Numero);  // Reutiliza la misma función de visualización
}

void MostrarAjuste(uint8_t Numero)  // Muestra el valor de ajuste de sensibilidad en el LCD
{
    uint8_t Decena;
    ENVIARCOMANDO(0x95);                    // Posiciona en línea 1, columna 5
    ENVIARDATO('A'); ENVIARDATO('j'); ENVIARDATO('u'); ENVIARDATO('.');  // Muestra "Aju."
    Decena = (uint8_t)(Numero / 10); Numero %= 10;
    if (Decena == 0) ENVIARDATO(' '); else ENVIARDATO((byte)(Decena + 0x30));  // Decena o espacio
    ENVIARDATO((byte)(Numero + 0x30));  // Unidad
}

void MostrarOffset(uint8_t Numero)  // Muestra el offset de tono en el LCD
{
    uint8_t Decena;
    ENVIARCOMANDO(0x95);                    // Posiciona en línea 1, columna 5
    ENVIARDATO('O'); ENVIARDATO('f'); ENVIARDATO('f'); ENVIARDATO('.');  // Muestra "Off."
    Decena = (uint8_t)(Numero / 10); Numero %= 10;
    if (Decena == 0) ENVIARDATO(' '); else ENVIARDATO((byte)(Decena + 0x30));
    ENVIARDATO((byte)(Numero + 0x30));
}

void MostrarTitulo(uint8_t Numero)  // Muestra el título del menú actual en la línea superior del LCD
{
    uint8_t Decena, Resto = 0;  // Decena para formateo, Resto para el valor numérico a mostrar
    ENVIARCOMANDO(0xC0);        // Posiciona el cursor al inicio de la línea 2
    if (Numero == 0) {          // Menú 0: modo detector (limpia la línea)
        for (Decena = 0; Decena < 16; Decena++) ENVIARDATO(' ');  // Borra toda la línea 2
        Resto = 255;  // Valor especial que evita mostrar número
    }
    if (Numero == 1) {  // Menú 1: Discriminación
        ENVIARDATO('D'); ENVIARDATO('i'); ENVIARDATO('s'); ENVIARDATO('c');
        ENVIARDATO('r'); ENVIARDATO('i'); ENVIARDATO('m'); ENVIARDATO('i');
        ENVIARDATO('n'); ENVIARDATO('a'); ENVIARDATO('t'); ENVIARDATO('i');
        ENVIARDATO('o'); ENVIARDATO('n'); ENVIARDATO(':'); ENVIARDATO(' ');
        Resto = Discrimination;  // Muestra el valor de discriminación
    }
    if (Numero == 2) {  // Menú 2: Discriminación (SamplePosition)
        ENVIARDATO('D'); ENVIARDATO('i'); ENVIARDATO('s'); ENVIARDATO('c');
        ENVIARDATO('r'); ENVIARDATO('i'); ENVIARDATO('m'); ENVIARDATO('i');
        ENVIARDATO('n'); ENVIARDATO('a'); ENVIARDATO('t'); ENVIARDATO('i');
        ENVIARDATO('o'); ENVIARDATO('n'); ENVIARDATO(':'); ENVIARDATO(' ');
        Resto = SamplePosition;  // Muestra la posición de sample
    }
    if (Numero == 3) {  // Menú 3: Volumen
        ENVIARDATO('V'); ENVIARDATO('o'); ENVIARDATO('l'); ENVIARDATO('u');
        ENVIARDATO('m'); ENVIARDATO('e'); ENVIARDATO(':'); ENVIARDATO(' ');
        for (Decena = 0; Decena < 8; Decena++) ENVIARDATO(' ');  // Espacios de relleno
        Resto = Volumen;
    }
    if (Numero == 4) {  // Menú 4: Sensibilidad (Ganancia)
        ENVIARDATO('S'); ENVIARDATO('e'); ENVIARDATO('n'); ENVIARDATO('s');
        ENVIARDATO('i'); ENVIARDATO('t'); ENVIARDATO('i'); ENVIARDATO('v');
        ENVIARDATO('i'); ENVIARDATO('t'); ENVIARDATO('y'); ENVIARDATO(':');
        ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' ');
        Resto = Ganancia;
    }
    if (Numero == 5) {  // Menú 5: Frecuencia (Tono)
        ENVIARDATO('F'); ENVIARDATO('r'); ENVIARDATO('e'); ENVIARDATO('q');
        ENVIARDATO('u'); ENVIARDATO('e'); ENVIARDATO('n'); ENVIARDATO('c');
        ENVIARDATO('y'); ENVIARDATO(':'); ENVIARDATO(' '); ENVIARDATO(' ');
        ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' ');
        Resto = Tone - TONEINITIAL;  // Muestra tono relativo al inicial
    }
    if (Numero == 6) {  // Menú 6: Offset de Tono
        ENVIARDATO('O'); ENVIARDATO('f'); ENVIARDATO('f'); ENVIARDATO('s');
        ENVIARDATO('e'); ENVIARDATO('t'); ENVIARDATO(' '); ENVIARDATO('T');
        ENVIARDATO('o'); ENVIARDATO('n'); ENVIARDATO('e'); ENVIARDATO(':');
        ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' ');
        Resto = OffsetTono;
    }
    if (Resto < 100) {  // Formatea el número si es menor que 100
        Decena = (uint8_t)(Resto / 10); Resto %= 10;
        if (Decena == 0) ENVIARDATO(' '); else ENVIARDATO((byte)(Decena + 0x30));
        ENVIARDATO((byte)(Resto + 0x30));
    } else {
        ENVIARDATO(' '); ENVIARDATO(' ');  // Si es >=100, solo muestra espacios
    }
    ENVIARDATO(' ');  // Espacio final
}

void MostrarMute(void)  // Muestra u oculta el indicador de Mute en el LCD
{
    ENVIARCOMANDO(0x90);  // Posiciona en línea 1, columna 0 (esquina superior izquierda)
    if (Mute == 1) {
        ENVIARDATO('M'); ENVIARDATO('u'); ENVIARDATO('t'); ENVIARDATO('e');  // Muestra "Mute"
    } else {
        ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' ');  // Borra el indicador
    }
}

void MostrarWord(word Numero)  // Muestra un número de 16 bits (5 dígitos) en el LCD
{
    uint8_t DM, UM, Centena, Decena;  // Dígitos: decenas de mil, unidades de mil, centenas, decenas
    DM = (uint8_t)(Numero / 10000); Numero %= 10000;  // Extrae decenas de mil
    UM = (uint8_t)(Numero / 1000); Numero %= 1000;    // Extrae unidades de mil
    Centena = (uint8_t)(Numero / 100); Numero %= 100; // Extrae centenas
    Decena = (uint8_t)(Numero / 10); Numero %= 10;    // Extrae decenas
    // Convierte cada dígito a ASCII y lo envía al LCD
    ENVIARDATO((byte)(DM + 0x30));
    ENVIARDATO((byte)(UM + 0x30));
    ENVIARDATO((byte)(Centena + 0x30));
    ENVIARDATO((byte)(Decena + 0x30));
    ENVIARDATO((byte)(Numero + 0x30));
}

void MostrarNumero(uint8_t Numero)  // Muestra un número de 8 bits (3 dígitos) en el LCD
{
    uint8_t Centena, Decena;  // Dígitos: centenas y decenas
    Centena = (uint8_t)(Numero / 100); Numero %= 100;  // Extrae centenas
    Decena = (uint8_t)(Numero / 10); Numero %= 10;     // Extrae decenas
    // Convierte cada dígito a ASCII y lo envía al LCD
    ENVIARDATO((byte)(Centena + 0x30));
    ENVIARDATO((byte)(Decena + 0x30));
    ENVIARDATO((byte)(Numero + 0x30));
}

uint8_t MostrarBat(void)  // Lee y muestra el nivel de batería, retorna estado (0=ok, 1=alarma, 2=mínimo)
{
    uint8_t VBat, Centena, Decena, resultado = 0;  // Variables de cálculo y resultado
    word tempo = ADCBateria;                       // Toma la última lectura ADC de batería
    tempo = tempo * 5 * 11;                        // Escala el valor ADC a milivoltios
    VBat = (uint8_t)(tempo / 102);                 // Convierte a porcentaje aproximado
    if (VBat < ALARMA_BAT) resultado = 1;          // Si está por debajo de alarma, resultado=1
    if (VBat < MINIMO_BAT) resultado = 2;          // Si está por debajo del mínimo, resultado=2
    if (VBat >= MAXIMO_BAT) VBat = 100;            // Satura al 100% si está al máximo
    else if (VBat <= MINIMO_BAT) VBat = 0;         // Muestra 0% si está al mínimo
    else {                                          // Si está entre mínimo y máximo
        tempo = (VBat - MINIMO_BAT) * 100;          // Calcula porcentaje relativo
        VBat = (uint8_t)(tempo / (MAXIMO_BAT - MINIMO_BAT));  // Normaliza a 0-100%
    }
    Centena = (uint8_t)(VBat / 100); VBat %= 100;   // Extrae centenas
    Decena = (uint8_t)(VBat / 10); VBat %= 10;      // Extrae decenas y unidades
    ENVIARCOMANDO(0xE4);                             // Posiciona en línea 2, columna 4
    ENVIARDATO((byte)(Centena + 0x30));             // Muestra centenas
    ENVIARDATO((byte)(Decena + 0x30));             // Muestra decenas
    ENVIARDATO((byte)(VBat + 0x30));               // Muestra unidades
    ENVIARDATO('%');                                // Muestra el símbolo de porcentaje
    return resultado;                               // Retorna el estado de la batería
}

void DibujarVumetro(uint8_t Menuvalor)  // Dibuja el vúmetro de barras en el LCD
{
    uint8_t Index, Index2, Impar, Segm;  // Índices de bucle, paridad y segmentos
    if (Menuvalor == 0) Segm = SegmentosMostrados;           // Modo detector: segmentos reales
    else if (Menuvalor == 1) Segm = Discrimination * 2;      // Menú 1: discriminación * 2
    else if (Menuvalor == 2) Segm = SamplePosition * 2;      // Menú 2: sample position * 2
    else if (Menuvalor == 3) Segm = Volumen;                  // Menú 3: volumen directo
    else if (Menuvalor == 4) Segm = Ganancia * 2;            // Menú 4: ganancia * 2
    else if (Menuvalor == 5) Segm = Tone - TONEINITIAL;      // Menú 5: tono relativo
    else if (Menuvalor == 6) Segm = OffsetTono / 2;          // Menú 6: offset/2
    else Segm = 0;                                            // Otros: cero
    Impar = Segm % 2; Segm /= 2;  // Determina si hay un segmento impar y divide entre 2
    for (Index = 0; Index < Segm; Index++) {  // Dibuja los segmentos completos
        ENVIARCOMANDO(0x94 + Index);          // Posiciona en línea 1 (barra superior)
        ENVIARDATO(255);                      // Carácter 255 (barra llena)
        if ((Debug == 0) && (Menu == 0)) {    // En modo normal y detector activo
            ENVIARCOMANDO(0xC0 + Index);      // También dibuja en línea 2
            ENVIARDATO(255);                  // Barra llena en línea 2
        }
    }
    if (Impar == 1) {  // Si hay segmento impar, dibuja medio segmento
        ENVIARCOMANDO(0x94 + Index);
        ENVIARDATO(1);  // Carácter 1 (medio lleno)
        if ((Debug == 0) && (Menu == 0)) {
            ENVIARCOMANDO(0xC0 + Index);
            ENVIARDATO(1);
        }
    } else if (Index < 20) {  // Si no hay impar, dibuja espacio vacío
        ENVIARCOMANDO(0x94 + Index);
        ENVIARDATO(' ');  // Carácter espacio
        if ((Debug == 0) && (Menu == 0)) {
            ENVIARCOMANDO(0xC0 + Index);
            ENVIARDATO(' ');
        }
    }
    Index++;  // Avanza al siguiente índice
    for (Index2 = Index; Index2 < (Index + 19 - Segm); Index2++) {  // Limpia el resto de la línea
        ENVIARCOMANDO(0x94 + Index2);
        ENVIARDATO(' ');
        if ((Debug == 0) && (Menu == 0)) {
            ENVIARCOMANDO(0xC0 + Index2);
            ENVIARDATO(' ');
        }
    }
}

void Presentacion(void)  // Muestra la pantalla de presentación al encender el detector
{
    BACKLIGHT_ON();                                     // Enciende el backlight
    ENVIARCOMANDO(0x80);                                // Inicio de línea 1
    ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' ');
    ENVIARDATO('P'); ENVIARDATO('u'); ENVIARDATO('l'); ENVIARDATO('s');
    ENVIARDATO('e'); ENVIARDATO(' ');
    ENVIARDATO(' '); ENVIARDATO('M'); ENVIARDATO('a'); ENVIARDATO('t');
    ENVIARDATO('i'); ENVIARDATO('c'); ENVIARDATO(' '); ENVIARDATO(' ');
    ENVIARDATO(' '); ENVIARDATO(' ');                   // "    Pulse Mat  "
    ENVIARCOMANDO(0xC0);                                // Inicio de línea 2
    ENVIARDATO(' '); ENVIARDATO('9'); ENVIARDATO('0'); ENVIARDATO('0');
    ENVIARDATO('0'); ENVIARDATO(' '); ENVIARDATO(' ');
    ENVIARDATO('F'); ENVIARDATO('u'); ENVIARDATO('l'); ENVIARDATO('l');
    ENVIARDATO(' '); ENVIARDATO('D'); ENVIARDATO('i'); ENVIARDATO('g');
    ENVIARDATO('i'); ENVIARDATO('t'); ENVIARDATO('a'); ENVIARDATO('l');
    ENVIARCOMANDO(0x94);                                // Inicio de línea 3
    ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' ');
    ENVIARDATO('A'); ENVIARDATO('C'); ENVIARDATO('E'); ENVIARDATO(' ');
    ENVIARDATO(' '); ENVIARDATO('D'); ENVIARDATO('E');
    ENVIARDATO('T'); ENVIARDATO('E'); ENVIARDATO('C'); ENVIARDATO('T');
    ENVIARDATO('O'); ENVIARDATO('R'); ENVIARDATO('S'); ENVIARDATO(' ');
    ENVIARDATO(' '); ENVIARDATO(' ');                   // "   ACE DETECTORS  "
}

void CalcularZero(void)  // Calcula el valor de referencia cero del terreno
{
    uint8_t i;                        // Variable de iteración
    retardo20ms(); retardo20ms();     // Espera 40ms para estabilizar la lectura
    ValorZero = 0;                    // Inicializa el acumulador
    for (i = 0; i < 20; i++) {        // Toma 20 muestras
        retardo20ms(); retardo20ms(); // Espera 40ms entre muestras
        ValorZero += ValorCalculado;  // Acumula el valor calculado
    }
    ValorZero /= 20;                  // Promedia para obtener el valor de referencia
    if (Debug == 1) {                 // En modo debug, muestra el valor calculado
        ENVIARCOMANDO(0xC0);
        MostrarWord(ValorZero);
    }
    ENVIARCOMANDO(0xA1);              // Limpia la línea de debug
    ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' '); ENVIARDATO(' ');
}

//--- Funciones de retardo (recalculadas para 4MHz Fcy = 1us/instrucción) ---
void retardo01ms(void)   { __delay_us(100); }     // Retardo de 100 microsegundos
void retardo1ms(void)    { __delay_ms(1); }       // Retardo de 1 milisegundo
void retardo50ms(void)   { __delay_ms(50); }      // Retardo de 50 milisegundos
void retardo100ms(void)  { __delay_ms(100); }     // Retardo de 100 milisegundos
void retardo20ms(void)   { __delay_ms(20); }      // Retardo de 20 milisegundos
void retardo500(void)    { __delay_ms(500); }     // Retardo de 500 milisegundos
void retardo1s(void)     { __delay_ms(1000); }    // Retardo de 1 segundo
void retardo100(void)    { __delay_ms(100); }     // Retardo de 100 milisegundos
void retardo10(void)     { __delay_ms(10); }      // Retardo de 10 milisegundos
void retardo5(void)      { __delay_ms(5); }       // Retardo de 5 milisegundos
void retardo1(void)      { __delay_ms(1); }       // Retardo de 1 milisegundo

//--- Inicialización de puertos y MCU ---
void initPorts(void)  // Configura todos los puertos del PIC (direcciones y estado inicial)
{
    LATA = 0; TRISA = 0x38;  // Puerto A: salidas, RA3-RA5 entradas (botones)
    LATB = 0; TRISB = 0x80;  // Puerto B: salidas, RB7 entrada (LCD)
    LATC = 0; TRISC = 0xE0;  // Puerto C: salidas, RC5-RC7 entradas (botones y UART)
    LATD = 0; TRISD = 0x00;  // Puerto D: todas salidas (LCD data)
    LATE = 0; TRISE = 0x07;  // Puerto E: entradas (RE0-RE2)
    PULSO_BOBINA = 1;        // Bobina desactivada (activo bajo)
}

void initPLL(void)  // Configura el PLL para operar a 16MHz
{
    OSCCON = 0x70;    // Configura el oscilador: HSPLL, CPU a 16MHz
}

void initMCU(void)  // Inicialización completa del microcontrolador
{
    initPLL();                  // Configura el PLL
    initPorts();                // Configura los puertos
    ADCON0 = 0x01;              // ADC encendido, canal 0, módulo operativo
    ADCON1 = 0x8E;              // Vref+=VDD, Vref-=VSS, solo AN0-AN2 analógicos
    ADCON2 = 0x39;              // Justificación derecha, Tad=16Tosc, adquisición=20Tad
}

//--- Manejo de variables del menú ---
void SubirVariable(void)  // Incrementa la variable correspondiente al menú activo
{
    if (Menu == 0 && Debug == 1) {              // Menú 0 debug: ajusta sensibilidad
        if (Sensibilidad < 50) { Sensibilidad++; SENSIB_MAS(); }  // Incrementa sensibilidad
        MostrarAjuste(Sensibilidad);            // Muestra el valor actual
        CalcularZero();                         // Recalcula el cero
    }
    if (Menu == 2) {                            // Menú 2: SamplePosition
        if (SamplePosition < SAMPLEMAXIMO) SamplePosition++;  // Incrementa posición de sample
        MostrarSample(SamplePosition);          // Muestra el valor
    }
    if (Menu == 3) {                            // Menú 3: Volumen
        if (Volumen < VOLUMENMAXIMO) { Volumen++; VOLUMEN_MAS(); }  // Incrementa volumen
        MostrarVolumen(Volumen);                // Muestra el valor
    }
    if (Menu == 4) {                            // Menú 4: Ganancia
        if (Ganancia < GANANCIAMAXIMA) Ganancia++;  // Incrementa ganancia
        MostrarGanancia(Ganancia);              // Muestra el valor
    }
    if (Menu == 5) {                            // Menú 5: Tono
        if (Tone < TONOMAXIMO) Tone++;          // Incrementa tono
        MostrarTone(Tone - TONEINITIAL);        // Muestra tono relativo
        AjustarPWM(0, Tone);                    // Ajusta el PWM con el nuevo tono
    }
    if (Menu == 6) {                            // Menú 6: Offset de Tono
        if (OffsetTono < OFFSETTONOMAXIMO) OffsetTono++;  // Incrementa offset
        AjustarPWM(0, Tone);                    // Ajusta el PWM
    }
}

void BajarVariable(void)  // Decrementa la variable correspondiente al menú activo
{
    if (Menu == 0 && Debug == 1) {              // Menú 0 debug: ajusta sensibilidad
        if (Sensibilidad > 0) Sensibilidad--;   // Decrementa sensibilidad
        SENSIB_MENOS();                          // Envía pulso de decremento
        MostrarAjuste(Sensibilidad);             // Muestra el valor
        CalcularZero();                          // Recalcula el cero
    }
    if (Menu == 2) {                             // Menú 2: SamplePosition
        if (SamplePosition > 1) SamplePosition--;  // Decrementa posición de sample
        MostrarSample(SamplePosition);
    }
    if (Menu == 3) {                             // Menú 3: Volumen
        if (Volumen > 0) Volumen--;              // Decrementa volumen
        VOLUMEN_MENOS();                          // Envía pulso de decremento
        MostrarVolumen(Volumen);
    }
    if (Menu == 4) {                             // Menú 4: Ganancia
        if (Ganancia > 1) Ganancia--;            // Decrementa ganancia
        MostrarGanancia(Ganancia);
    }
    if (Menu == 5) {                             // Menú 5: Tono
        if (Tone > TONEINITIAL) Tone--;          // Decrementa tono
        MostrarTone(Tone - TONEINITIAL);
        AjustarPWM(0, Tone);                     // Ajusta el PWM
    }
    if (Menu == 6) {                             // Menú 6: Offset de Tono
        if (OffsetTono > 0) OffsetTono--;        // Decrementa offset
        AjustarPWM(0, Tone);                     // Ajusta el PWM
    }
}

void RecargarValoresDefault(void)  // Recarga valores por defecto si el botón Reset se mantiene 5 segundos
{
    if (BOTON_RESET == 0) {         // Verifica que el botón Reset esté presionado
        retardo1s();                // Espera 1 segundo
        if (BOTON_RESET == 0) {     // Verifica que aún esté presionado
            retardo1s();            // Espera otro segundo
            if (BOTON_RESET == 0) { // Verifica nuevamente
                retardo1s();        // Espera otro segundo
                if (BOTON_RESET == 0) {  // Verifica nuevamente
                    retardo1s();         // Espera otro segundo
                    if (BOTON_RESET == 0) {  // Verifica por última vez
                        retardo1s();         // Espera otro segundo
                        if (BOTON_RESET == 0) {  // Si aún está presionado (5 segundos total)
                            CargarDefaults();     // Carga los valores por defecto
                        }
                    }
                }
            }
        }
    }
}

void CargarDefaults(void)  // Carga valores por defecto y muestra mensaje en LCD
{
    ENVIARCOMANDO(0x96);                               // Posiciona cursor
    ENVIARDATO('L'); ENVIARDATO('o'); ENVIARDATO('a'); ENVIARDATO('d');
    ENVIARDATO('i'); ENVIARDATO('n'); ENVIARDATO('g'); ENVIARDATO(' ');
    ENVIARDATO('D'); ENVIARDATO('e'); ENVIARDATO('f'); ENVIARDATO('a');
    ENVIARDATO('u'); ENVIARDATO('l'); ENVIARDATO('t'); ENVIARDATO('s');  // "Loading Defaults"
    ReLoadEEPROM();                                    // Recarga EEPROM con valores por defecto
    retardo1s(); retardo1s();                          // Espera 2 segundos
    ENVIARCOMANDO(0x96);                               // Limpia la línea
    for (uint8_t i = 0; i < 16; i++) ENVIARDATO(' '); // Borra 16 caracteres
    MostrarAjuste(Sensibilidad);                       // Muestra sensibilidad
    MostrarDiscrimination(SamplePosition);             // Muestra discriminación
    MostrarVolumen(Volumen);                           // Muestra volumen
    MostrarGanancia(Ganancia);                         // Muestra ganancia
    MostrarTone(Tone - TONEINITIAL);                   // Muestra tono
}
