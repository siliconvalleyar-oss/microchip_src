#include "eeprom.h"      // Header de gestión de EEPROM
#include "rutinasMisc.h" // Funciones misceláneas (retardos, etc.)

// Variables globales que se persisten en EEPROM
uint8_t Volumen;          // Nivel de volumen del audio
uint8_t Discrimination;   // Nivel de discriminación ferroso/no-ferroso
uint8_t SamplePosition;   // Posición de muestreo de la señal del detector
uint8_t Ganancia;         // Ganancia del detector
uint8_t OtroParam;        // Otro parámetro configurable

// --- Acceso personalizado a EEPROM para PIC18F4620 ---
// Las funciones incorporadas eeprom_read/eeprom_write ya no están disponibles en XC8 v2.45

static uint8_t EEPROM_Read(uint8_t addr)  // Lee un byte de la EEPROM en la dirección dada
{
    while (EECON1bits.WR);  // Espera a que termine cualquier escritura pendiente
    EEADR = addr;           // Carga la dirección a leer
    EECON1bits.EEPGD = 0;   // Selecciona memoria EEPROM (no Flash de programa)
    EECON1bits.CFGS = 0;    // Selecciona memoria de datos (no configuración)
    EECON1bits.RD = 1;      // Inicia la lectura
    return EEDATA;          // Retorna el dato leído
}

static void EEPROM_Write(uint8_t addr, uint8_t data)  // Escribe un byte en la EEPROM
{
    while (EECON1bits.WR);  // Espera a que termine cualquier escritura pendiente
    EEADR = addr;           // Carga la dirección a escribir
    EEDATA = data;          // Carga el dato a escribir
    EECON1bits.EEPGD = 0;   // Selecciona memoria EEPROM
    EECON1bits.CFGS = 0;    // Selecciona memoria de datos
    EECON1bits.WREN = 1;    // Habilita las escrituras en EEPROM
    // Secuencia de desbloqueo requerida por el hardware del PIC
    INTCONbits.GIE = 0;     // Deshabilita interrupciones durante la escritura
    EECON2 = 0x55;          // Primer byte de la secuencia de desbloqueo
    EECON2 = 0xAA;          // Segundo byte de la secuencia de desbloqueo
    EECON1bits.WR = 1;      // Inicia la escritura
    INTCONbits.GIE = 1;     // Rehabilita las interrupciones
    EECON1bits.WREN = 0;    // Deshabilita las escrituras por seguridad
}

void LoadRAM_Default(void)  // Carga los valores por defecto en las variables en RAM
{
    Volumen = VOLUMEN_DEFAULT;                // Volumen por defecto
    Discrimination = DISCRIMINATION_DEFAULT;  // Discriminación por defecto
    SamplePosition = SAMPLEPOSITION_DEFAULT;  // Posición de muestreo por defecto
    Ganancia = GANANCIA_DEFAULT;              // Ganancia por defecto
    Tone = TONE_DEFAULT;                      // Tono por defecto
    OffsetTono = OFFSETTONO_DEFAULT;          // Offset de tono por defecto
    OtroParam = OTROPARAM_DEFAULT;            // Otro parámetro por defecto
}

void LoadRAM(void)  // Carga los valores desde la EEPROM a las variables en RAM
{
    Volumen = EEPROM_Read(EEPROM_ADDR_VOLUMEN);                // Lee volumen de EEPROM
    Discrimination = EEPROM_Read(EEPROM_ADDR_DISCRIMINATION);  // Lee discriminación de EEPROM
    SamplePosition = EEPROM_Read(EEPROM_ADDR_SAMPLEPOS);      // Lee sample position de EEPROM
    Ganancia = EEPROM_Read(EEPROM_ADDR_GANANCIA);             // Lee ganancia de EEPROM
    Tone = EEPROM_Read(EEPROM_ADDR_TONE);                     // Lee tono de EEPROM
    OffsetTono = EEPROM_Read(EEPROM_ADDR_OFFSETTONO);         // Lee offset de tono de EEPROM
    OtroParam = EEPROM_Read(EEPROM_ADDR_OTROPARAM);           // Lee otro parámetro de EEPROM
}

void GrabarEEPROM(void)  // Guarda todas las variables actuales en EEPROM
{
    EEPROM_Write(EEPROM_ADDR_VOLUMEN, Volumen);                // Guarda volumen
    EEPROM_Write(EEPROM_ADDR_DISCRIMINATION, Discrimination);  // Guarda discriminación
    EEPROM_Write(EEPROM_ADDR_SAMPLEPOS, SamplePosition);      // Guarda sample position
    EEPROM_Write(EEPROM_ADDR_GANANCIA, Ganancia);             // Guarda ganancia
    EEPROM_Write(EEPROM_ADDR_TONE, Tone);                     // Guarda tono
    EEPROM_Write(EEPROM_ADDR_OFFSETTONO, OffsetTono);         // Guarda offset de tono
    EEPROM_Write(EEPROM_ADDR_OTROPARAM, OtroParam);           // Guarda otro parámetro
    EEPROM_Write(EEPROM_ADDR_FLASHFLAG, FLASHFLAG_VALID);     // Guarda flag de validación
}

void VerEEPROM(void)  // Verifica si la EEPROM contiene datos válidos y carga valores
{
    uint8_t flag = EEPROM_Read(EEPROM_ADDR_FLASHFLAG);  // Lee el flag de validación
    if (flag != FLASHFLAG_VALID) {  // Si el flag no es válido, carga valores por defecto
        LoadRAM_Default();          // Carga valores por defecto en RAM
        OffsetTono = 30;            // Ajuste especial de offset de tono para primer arranque
        GrabarEEPROM();             // Guarda los valores por defecto en EEPROM
        retardo1s();                // Espera 1 segundo para completar la escritura
        retardo1s();                // Espera otro segundo como margen de seguridad
    }
    LoadRAM();  // Carga los valores desde EEPROM a RAM
}

void ReLoadEEPROM(void)  // Recarga los valores por defecto preservando el offset de tono
{
    uint8_t tempo = OffsetTono;  // Guarda temporalmente el offset de tono actual
    LoadRAM_Default();           // Carga valores por defecto en RAM
    OffsetTono = tempo;          // Restaura el offset de tono previo
    GrabarEEPROM();              // Guarda los valores en EEPROM
    retardo1s();                 // Espera 1 segundo
    retardo1s();                 // Espera otro segundo
    LoadRAM();                   // Recarga los valores desde EEPROM a RAM
}
