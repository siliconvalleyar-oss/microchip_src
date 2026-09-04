/*
 * mrf24j40.h  - Driver MRF24J40 para PIC18F4620 XC8
 *
 * Protocolo IEEE 802.15.4 con intra-PAN compression
 * Frame: FCF(2)+Seq(1)+PAN(2)+Dest(2)+Src(2)+Payload+FCS(HW)
 *
 * Compatible con el driver de Raspberry Pi (mismo PAN, canal,
 * formato de trama) para comunicacion directa.
 */
#ifndef MRF24J40_H
#define MRF24J40_H

#include "config.h"

/* ----------------------------------------------------------
 * Registros SHORT address (6 bits)
 * Trama SPI lectura:  [A5..A0 | 0 | 0] + dummy
 * Trama SPI escritura:[A5..A0 | 1 | 0] + dato
 * ---------------------------------------------------------- */
#define REG_RXMCR    0x00
#define REG_PANIDL   0x01
#define REG_PANIDH   0x02
#define REG_SADRL    0x03
#define REG_SADRH    0x04
#define REG_EADR0    0x05
#define REG_EADR1    0x06
#define REG_EADR2    0x07
#define REG_EADR3    0x08
#define REG_EADR4    0x09
#define REG_EADR5    0x0A
#define REG_EADR6    0x0B
#define REG_EADR7    0x0C
#define REG_RXFLUSH  0x0D
#define REG_ORDER    0x10
#define REG_TXMCR    0x11
#define REG_ACKTMOUT 0x12
#define REG_PACON0   0x16
#define REG_PACON1   0x17
#define REG_PACON2   0x18
#define REG_TXNCON   0x1B
#define REG_TXSTAT   0x24
#define REG_SOFTRST  0x2A
#define REG_TXSTBL   0x2E
#define REG_INTSTAT  0x31
#define REG_INTCON   0x32
#define REG_RFCTL    0x36
#define REG_BBREG1   0x39
#define REG_BBREG2   0x3A
#define REG_BBREG6   0x3E
#define REG_CCAEDTH  0x3F

/* ----------------------------------------------------------
 * Registros LONG address (10 bits)
 * Trama SPI lectura:  [1|A9..A3][A2..A0|0|0|0|0|0] + dummy
 * Trama SPI escritura:[1|A9..A3][A2..A0|1|0|0|0|0] + dato
 * ---------------------------------------------------------- */
#define LREG_RFCON0  0x200
#define LREG_RFCON1  0x201
#define LREG_RFCON2  0x202
#define LREG_RFCON3  0x203
#define LREG_RFCON6  0x206
#define LREG_RFCON7  0x207
#define LREG_RFCON8  0x208
#define LREG_SLPCON1 0x220

/* FIFOs */
#define TXNFIFO      0x000
#define RXFIFO       0x300

/* Bits INTSTAT */
#define INT_TXNIF    0x01
#define INT_RXIF     0x08

/* Bits TXNCON */
#define TXNTRIG      0x01
#define TXNACKREQ    0x04

/* Frame Control Field - Data frame, intra-PAN, ACK req, short addr */
#define FCF_LO       0x61
#define FCF_HI       0x88

/* Header: FCF(2)+Seq(1)+PAN(2)+Dest(2)+Src(2) = 9 bytes */
#define MAC_HDR_LEN  9

/* ----------------------------------------------------------
 * Estructura de resultado de RX
 * ---------------------------------------------------------- */
typedef struct {
    uint8_t  data[MRF_MAX_PAYLOAD];
    uint8_t  len;
    uint8_t  lqi;
    int8_t   rssi_dbm;
    uint16_t src_addr;
} MRF_RxPacket;

/* ----------------------------------------------------------
 * API publica
 * ---------------------------------------------------------- */

/* Inicializar: SPI ya debe estar inicializado.
 * Retorna 1 si OK, 0 si fallo (timeout reset) */
uint8_t MRF_Init(void);

/* Configurar PAN ID y direccion corta */
void    MRF_SetPAN(uint16_t pan);
void    MRF_SetAddr(uint16_t addr);
uint16_t MRF_GetPAN(void);
uint16_t MRF_GetAddr(void);

/* Cambiar canal (11-26). Llama RF reset internamente. */
uint8_t MRF_SetChannel(uint8_t ch);

/* Transmitir payload.
 * dest_addr: direccion destino
 * data: puntero a datos
 * len: longitud (max MRF_MAX_PAYLOAD)
 * Retorna 1 si TX OK (ACK recibido), 0 si fallo */
uint8_t MRF_Send(uint16_t dest_addr, const uint8_t *data, uint8_t len);

/* Transmitir string (sin terminador NULL) */
uint8_t MRF_SendStr(uint16_t dest_addr, const char *str);

/* Polling de interrupciones. Llamar en el loop principal.
 * Retorna 1 si hay paquete RX nuevo, 0 si no */
uint8_t MRF_Poll(MRF_RxPacket *pkt);

/* Leer ultimo estado TX (1=ok, 0=fallo) */
uint8_t MRF_TxSuccess(void);

/* Leer numero de reintentos TX del ultimo envio */
uint8_t MRF_TxRetries(void);

#endif /* MRF24J40_H */
