
#include "eprom.h"



// Escribir un byte en EEPROM
void EEPROM_Write(uint8_t address, uint8_t data) {
    EEADR = address;       // Dirección de EEPROM
    EEDATA = data;         // Dato a escribir
    EECON1bits.EEPGD = 0;  // Acceso a EEPROM de datos, no programa
    EECON1bits.WREN = 1;   // Habilita escritura
    INTCONbits.GIE = 0;    // Deshabilita interrupciones
    EECON2 = 0x55;         // Secuencia obligatoria
    EECON2 = 0xAA;
    EECON1bits.WR = 1;     // Inicia la escritura
    while(EECON1bits.WR);  // Espera a que termine
    EECON1bits.WREN = 0;   // Deshabilita escritura
    INTCONbits.GIE = 1;    // Rehabilita interrupciones
}


void EEPROM_WriteString(uint8_t startAddress, const char* str) {
    while(*str) {
        EEPROM_Write(startAddress++, *str++);
    }
    EEPROM_Write(startAddress, 0); // Fin de cadena
}

void EEPROM_ReadString(uint8_t startAddress, char* buffer, uint8_t maxLen) {
    uint8_t i = 0;
    uint8_t c;
    do {
        c = EEPROM_Read(startAddress++);
        buffer[i++] = c;
    } while(c != 0 && i < maxLen);
    buffer[maxLen-1] = 0; // Asegura terminación
}


// Leer un byte de EEPROM


uint8_t EEPROM_Read(uint8_t address) {
    EEADR = address;       // Dirección de la EEPROM
    EECON1bits.RD = 1;     // Inicia lectura
    return EEDATA;         // Retorna el valor leído
}