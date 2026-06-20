/*
 * mrf24j40.c  - Driver MRF24J40 para PIC18F4620 XC8
 *
 * Mismos registros y secuencia de init que el driver
 * de Raspberry Pi para maxima compatibilidad de red.
 *
 * SPI formato:
 *   Short addr (6b): byte = [A5 A4 A3 A2 A1 A0 RW 0]
 *     RW=0 lectura, RW=1 escritura
 *
 *   Long addr (10b): byte0 = [1 A9 A8 A7 A6 A5 A4 A3]
 *                    byte1 = [A2 A1 A0 RW 0  0  0  0]
 *                    byte2 = dato (escritura) o dummy (lectura)
 */
#include "mrf24j40.h"
#include "spi.h"
#include <string.h>

/* ---- Variables internas --------------------------------- */
static uint8_t  tx_seq   = 0;
static uint8_t  tx_ok    = 0;
static uint8_t  tx_retry = 0;
static uint16_t my_pan   = MRF_PAN_ID;
static uint16_t my_addr  = MRF_MY_ADDR;

/* ---- Primitivas SPI ------------------------------------- */

/* Leer registro short (6 bits de direccion) */
static uint8_t MRF_ReadShort(uint8_t addr)
{
    uint8_t val;
    SPI_CS_LOW();
    SPI_Transfer((uint8_t)((addr & 0x3Fu) << 1u));  /* RW=0 */
    val = SPI_Transfer(0x00);
    SPI_CS_HIGH();
    return val;
}

/* Escribir registro short */
static void MRF_WriteShort(uint8_t addr, uint8_t val)
{
    SPI_CS_LOW();
    SPI_Transfer((uint8_t)(((addr & 0x3Fu) << 1u) | 0x01u));  /* RW=1 */
    SPI_Transfer(val);
    SPI_CS_HIGH();
}

/* Leer registro long (10 bits de direccion) */
static uint8_t MRF_ReadLong(uint16_t addr)
{
    uint8_t val;
    uint8_t b0 = (uint8_t)(0x80u | ((addr >> 3u) & 0x7Fu));
    uint8_t b1 = (uint8_t)((addr & 0x07u) << 5u);           /* RW=0 */
    SPI_CS_LOW();
    SPI_Transfer(b0);
    SPI_Transfer(b1);
    val = SPI_Transfer(0x00);
    SPI_CS_HIGH();
    return val;
}

/* Escribir registro long */
static void MRF_WriteLong(uint16_t addr, uint8_t val)
{
    uint8_t b0 = (uint8_t)(0x80u | ((addr >> 3u) & 0x7Fu));
    uint8_t b1 = (uint8_t)(((addr & 0x07u) << 5u) | 0x10u); /* RW=1 */
    SPI_CS_LOW();
    SPI_Transfer(b0);
    SPI_Transfer(b1);
    SPI_Transfer(val);
    SPI_CS_HIGH();
}

/* ---- RF Reset (aplicar configuracion de canal/PLL) ------ */
static void MRF_RFReset(void)
{
    MRF_WriteShort(REG_RFCTL, 0x04);
    __delay_us(200);
    MRF_WriteShort(REG_RFCTL, 0x00);
    __delay_ms(1);
}

/* ---- Limpiar FIFO RX ------------------------------------ */
static void MRF_FlushRx(void)
{
    MRF_WriteShort(REG_BBREG1, 0x04);  /* RXDECINV=1: deshabilitar RX */
    MRF_WriteShort(REG_RXFLUSH, 0x01);
    __delay_us(100);
    MRF_WriteShort(REG_BBREG1, 0x00);  /* Rehabilitar RX */
}

/* ---- Inicializar MRF24J40 ------------------------------- */
/*
 * Secuencia identica al driver Raspberry Pi:
 *   1. Soft reset
 *   2. PACON2, TXSTBL
 *   3. Registros RF long (RFCON1..8, SLPCON1)
 *   4. Configurar canal (RFCON0)
 *   5. Baseband (BBREG2, CCAEDTH, BBREG6)
 *   6. Interrupciones (INTCON)
 *   7. Flush RX
 *   8. RF Reset
 */
uint8_t MRF_Init(void)
{
    uint8_t timeout;

    /* Asegurar CS alto */
    SPI_CS_HIGH();
    __delay_ms(2);

    /* ---- Soft reset ---- */
    MRF_WriteShort(REG_SOFTRST, 0x07);     /* Reset: RF+BB+MAC */
    __delay_ms(2);

    timeout = 200;
    while (timeout--) {
        if ((MRF_ReadShort(REG_SOFTRST) & 0x07u) == 0x00u) break;
        __delay_ms(1);
    }
    if (timeout == 0) return 0;             /* Fallo: no responde */

    /* ---- Paso 1: PACON2 ---- */
    MRF_WriteShort(REG_PACON2, 0x98);

    /* ---- Paso 2: TXSTBL ---- */
    MRF_WriteShort(REG_TXSTBL, 0x95);

    /* ---- Paso 3: Registros RF long ---- */
    MRF_WriteLong(LREG_RFCON1, 0x02);      /* VCO optimizado */
    MRF_WriteLong(LREG_RFCON2, 0x80);      /* PLL ON */
    MRF_WriteLong(LREG_RFCON3, 0x00);      /* TX power = 0 dBm (maximo) */
    MRF_WriteLong(LREG_RFCON6, 0x90);      /* TXFIL=1, 20MRECVR=1 */
    MRF_WriteLong(LREG_RFCON7, 0x80);      /* Sleep clock 100kHz */
    MRF_WriteLong(LREG_RFCON8, 0x10);      /* RFVCO=1 */
    MRF_WriteLong(LREG_SLPCON1, 0x21);     /* CLKOUTDIS=1, SLPCLKDIV=1 */

    /* ---- Paso 4: Canal ---- */
    MRF_SetChannel(MRF_CHANNEL);

    /* ---- Paso 5: Baseband ---- */
    MRF_WriteShort(REG_BBREG2,  0x80);     /* CCAMODE=ED */
    MRF_WriteShort(REG_CCAEDTH, 0x60);     /* Umbral CCA */
    MRF_WriteShort(REG_BBREG6,  0x40);     /* Append RSSI */

    /* ---- Paso 6: Interrupciones ---- */
    /* 0xF6 = solo RXIF(bit3) y TXNIF(bit0) habilitados */
    MRF_WriteShort(REG_INTCON, 0xF6);

    /* ---- Paso 7: Flush RX ---- */
    MRF_FlushRx();

    /* ---- Paso 8: RF Reset ---- */
    MRF_RFReset();

    __delay_ms(5);
    return 1;
}

/* ---- Configurar PAN y direccion ------------------------- */
void MRF_SetPAN(uint16_t pan)
{
    my_pan = pan;
    MRF_WriteShort(REG_PANIDL,  (uint8_t)(pan & 0xFFu));
    MRF_WriteShort(REG_PANIDH,  (uint8_t)(pan >> 8u));
}

void MRF_SetAddr(uint16_t addr)
{
    my_addr = addr;
    MRF_WriteShort(REG_SADRL, (uint8_t)(addr & 0xFFu));
    MRF_WriteShort(REG_SADRH, (uint8_t)(addr >> 8u));
}

uint16_t MRF_GetPAN(void)
{
    return (uint16_t)MRF_ReadShort(REG_PANIDL) |
           ((uint16_t)MRF_ReadShort(REG_PANIDH) << 8u);
}

uint16_t MRF_GetAddr(void)
{
    return (uint16_t)MRF_ReadShort(REG_SADRL) |
           ((uint16_t)MRF_ReadShort(REG_SADRH) << 8u);
}

/* ---- Cambiar canal (11-26) ------------------------------ */
uint8_t MRF_SetChannel(uint8_t ch)
{
    uint8_t val;
    if (ch < 11u || ch > 26u) return 0;
    /* RFCON0: bits[7:4]=(ch-11)<<4, bits[1:0]=0x03 */
    val = (uint8_t)(((ch - 11u) << 4u) | 0x03u);
    MRF_WriteLong(LREG_RFCON0, val);
    MRF_RFReset();
    return 1;
}

/* ---- Transmitir ----------------------------------------- */
/*
 * Layout TX FIFO (desde offset TXNFIFO=0x000):
 *   [0]   = header length = MAC_HDR_LEN (9)
 *   [1]   = frame length  = MAC_HDR_LEN + payload_len
 *   [2]   = FCF_LO (0x61)
 *   [3]   = FCF_HI (0x88)
 *   [4]   = Sequence number
 *   [5]   = PAN_ID low
 *   [6]   = PAN_ID high
 *   [7]   = Dest addr low
 *   [8]   = Dest addr high
 *   [9]   = Src addr low
 *   [10]  = Src addr high
 *   [11+] = Payload
 */
uint8_t MRF_Send(uint16_t dest_addr, const uint8_t *data, uint8_t len)
{
    uint8_t i;
    uint8_t frm_len;
    uint16_t reg;
    uint8_t timeout;
    uint8_t txstat;

    if (len == 0u || len > MRF_MAX_PAYLOAD) return 0;

    frm_len = MAC_HDR_LEN + len;
    reg     = TXNFIFO;

    /* Escribir FIFO */
    MRF_WriteLong(reg++, MAC_HDR_LEN);
    MRF_WriteLong(reg++, frm_len);
    MRF_WriteLong(reg++, FCF_LO);
    MRF_WriteLong(reg++, FCF_HI);
    MRF_WriteLong(reg++, tx_seq++);
    MRF_WriteLong(reg++, (uint8_t)(my_pan & 0xFFu));
    MRF_WriteLong(reg++, (uint8_t)(my_pan >> 8u));
    MRF_WriteLong(reg++, (uint8_t)(dest_addr & 0xFFu));
    MRF_WriteLong(reg++, (uint8_t)(dest_addr >> 8u));
    MRF_WriteLong(reg++, (uint8_t)(my_addr & 0xFFu));
    MRF_WriteLong(reg++, (uint8_t)(my_addr >> 8u));
    for (i = 0; i < len; i++) {
        MRF_WriteLong(reg++, data[i]);
    }

    /* Disparar TX: pedir ACK */
    MRF_WriteShort(REG_TXNCON, TXNACKREQ | TXNTRIG);

    /* Esperar TXNIF o timeout 500ms */
    timeout = 100;
    while (timeout--) {
        if (MRF_ReadShort(REG_INTSTAT) & INT_TXNIF) break;
        __delay_ms(5);
    }
    if (timeout == 0) {
        tx_ok = 0;
        return 0;
    }

    txstat   = MRF_ReadShort(REG_TXSTAT);
    tx_ok    = (txstat & 0x01u) ? 0u : 1u;   /* bit0=1 -> fallo */
    tx_retry = (txstat >> 6u) & 0x03u;

    /* Limpiar flag */
    MRF_WriteShort(REG_INTSTAT, INT_TXNIF);

    return tx_ok;
}

/* Transmitir string (sin '\0') */
uint8_t MRF_SendStr(uint16_t dest_addr, const char *str)
{
    uint8_t len = 0;
    const char *p = str;
    while (*p && len < MRF_MAX_PAYLOAD) { len++; p++; }
    return MRF_Send(dest_addr, (const uint8_t *)str, len);
}

/* ---- Poll interrupciones -------------------------------- */
/*
 * RX FIFO layout (desde RXFIFO=0x300):
 *   offset 0          : frame_len (FCF+Seq+addr+payload+FCS)
 *   offset 1..N       : frame bytes
 *   offset 1+frame_len: LQI
 *   offset 2+frame_len: RSSI (raw)
 *
 * Header = FCF(2)+Seq(1)+PAN(2)+Dest(2)+Src(2) = 9 bytes
 * FCS    = 2 bytes (hardware)
 * Payload = frame_len - 9 - 2
 */
uint8_t MRF_Poll(MRF_RxPacket *pkt)
{
    uint8_t irq, frame_len, i;
    uint8_t hdr = 9u, fcs = 2u;
    int8_t payload_len;
    uint16_t base;
    uint8_t raw_rssi;

    irq = MRF_ReadShort(REG_INTSTAT);

    /* Procesar TXNIF si quedo pendiente */
    if (irq & INT_TXNIF) {
        uint8_t txstat   = MRF_ReadShort(REG_TXSTAT);
        tx_ok    = (txstat & 0x01u) ? 0u : 1u;
        tx_retry = (txstat >> 6u) & 0x03u;
        MRF_WriteShort(REG_INTSTAT, INT_TXNIF);
    }

    /* Procesar RXIF */
    if (!(irq & INT_RXIF)) return 0;

    /* Deshabilitar RX mientras leemos */
    MRF_WriteShort(REG_BBREG1, 0x04);

    base       = RXFIFO;
    frame_len  = MRF_ReadLong(base);

    /* Validar longitud minima: hdr(9)+FCS(2)+1 byte payload = 12 */
    payload_len = (int8_t)((int8_t)frame_len - (int8_t)hdr - (int8_t)fcs);
    if (payload_len <= 0 || frame_len > 127u) {
        MRF_FlushRx();
        MRF_WriteShort(REG_INTSTAT, INT_RXIF);
        return 0;
    }
    if (payload_len > MRF_MAX_PAYLOAD) payload_len = MRF_MAX_PAYLOAD;

    if (pkt != 0) {
        pkt->len = (uint8_t)payload_len;
        /* Payload: base + 1(frame_len_byte) + hdr */
        for (i = 0; i < (uint8_t)payload_len; i++) {
            pkt->data[i] = MRF_ReadLong((uint16_t)(base + 1u + hdr + i));
        }
        /* Leer direccion fuente del header: offset 1+7=8, 1+8=9 (src addr) */
        pkt->src_addr = (uint16_t)MRF_ReadLong((uint16_t)(base + 8u)) |
                        ((uint16_t)MRF_ReadLong((uint16_t)(base + 9u)) << 8u);
        /* LQI y RSSI despues del FCS */
        pkt->lqi      = MRF_ReadLong((uint16_t)(base + 1u + frame_len));
        raw_rssi      = MRF_ReadLong((uint16_t)(base + 2u + frame_len));
        /* Conversion aproximada: dBm = -90 + raw/3 */
        pkt->rssi_dbm = (int8_t)(-90 + (int8_t)(raw_rssi / 3u));
        pkt->data[(uint8_t)payload_len] = '\0';   /* null-terminate */
    }

    MRF_WriteShort(REG_RXFLUSH, 0x01);
    __delay_us(50);
    MRF_WriteShort(REG_BBREG1, 0x00);
    MRF_WriteShort(REG_INTSTAT, INT_RXIF);

    return 1;
}

/* ---- Estado TX ------------------------------------------ */
uint8_t MRF_TxSuccess(void) { return tx_ok;    }
uint8_t MRF_TxRetries(void) { return tx_retry; }
