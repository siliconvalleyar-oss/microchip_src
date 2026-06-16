#ifndef MRF24J40_H
#define MRF24J40_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PAYLOAD 100

#define MRF_CS_PIN   LATC0
#define MRF_CS_L()   (MRF_CS_PIN = 0)
#define MRF_CS_H()   (MRF_CS_PIN = 1)

// Opcional: si usas reset por hardware, define este pin
// #define MRF_RESET_PIN   LATA1
// #define MRF_RESET_HW()

bool MRF_Init(uint8_t channel);
void MRF_SetPan(uint16_t pan);
void MRF_SetShortAddress(uint16_t addr);
uint16_t MRF_GetPan(void);
uint16_t MRF_GetShortAddress(void);

bool MRF_Send(uint16_t dest_addr, uint16_t dest_pan, const uint8_t* data, uint8_t len);
bool MRF_IsTxPending(void);
bool MRF_TxSuccess(void);
uint8_t MRF_TxRetries(void);

// Las funciones de manejo de eventos ahora serán llamadas desde la ISR
void MRF_HandleInterrupt(void);
bool MRF_HasPacket(void);
void MRF_GetRx(uint8_t* buf, uint8_t* len, uint8_t* lqi, int8_t* rssi);

// Para uso interno
void MRF_EnableRx(void);

#endif
