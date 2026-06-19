#pragma once
/**
 * protocol.hpp  –  Protocolo de red seguro para PIC32 ↔ Raspberry Pi
 *
 * Estructura del paquete:
 *   ┌─────────┬────────┬────────┬──────────────────┬──────────────┐
 *   │ MAGIC   │ SRC_ID │ DST_ID │ PAYLOAD CIFRADO  │ HMAC-SHA256  │
 *   │ 2 bytes │ 2 bytes│ 2 bytes│  (≤ MAX_PAYLOAD) │   32 bytes   │
 *   │ 0xA55A  │        │        │ [LEN 1B][DATOS]  │              │
 *   └─────────┴────────┴────────┴──────────────────┴──────────────┘
 *   Tamaño máximo de trama: 2+2+2+1+76+32 = 115 bytes (≤ 127 aMaxPHY)
 *
 * Características:
 *   - Lista blanca de IDs válidos (rechaza nodos desconocidos)
 *   - Número de secuencia por nodo (anti-replay)
 *   - Compatible con el protocolo ZigBee de RPi del proyecto anterior
 */

#include <cstdint>
#include <cstring>
#include "crypto.hpp"

namespace PROTO {

constexpr uint16_t MAGIC         = 0xA55A;
constexpr uint8_t  MAX_PAYLOAD   = 76;    // Bytes máx de datos de usuario
constexpr uint8_t  MAX_NODES     = 16;    // Nodos en la lista blanca
constexpr uint8_t  MAX_RETRIES   = 3;
constexpr uint16_t ACK_TIMEOUT   = 300;   // ms

// Tipos de mensaje (compatibles con zigbee_proto.h de la RPi)
enum class MsgType : uint8_t {
    DATA       = 0x01,
    ACK        = 0x02,
    NACK       = 0x03,
    JOIN_REQ   = 0x10,
    JOIN_ACK   = 0x11,
    HEARTBEAT  = 0x20,
    HEARTBEAT_R= 0x21,
    COMMAND    = 0x30,
    COMMAND_R  = 0x31,
};

// ─────────────────────────────────────────────────────────────────────────────
//  Paquete de red (serializado en wire)
// ─────────────────────────────────────────────────────────────────────────────
#pragma pack(push, 1)
struct Packet {
    uint16_t magic  { MAGIC };
    uint16_t src_id { 0 };
    uint16_t dst_id { 0 };
    uint8_t  seq    { 0 };
    uint8_t  type   { (uint8_t)MsgType::DATA };
    uint8_t  plen   { 0 };                        // longitud del payload cifrado
    uint8_t  payload[MAX_PAYLOAD] {};              // payload cifrado
    uint8_t  hmac[CRYPTO::HMAC_LEN] {};            // HMAC-SHA256 (32 bytes)

    // Tamaño serializado real (sin los bytes no usados del payload)
    uint8_t wire_size() const {
        return (uint8_t)(9 + plen + CRYPTO::HMAC_LEN);
    }

    bool valid_magic() const { return magic == MAGIC; }
};
#pragma pack(pop)

// ─────────────────────────────────────────────────────────────────────────────
//  Entrada en la lista de nodos válidos
// ─────────────────────────────────────────────────────────────────────────────
struct NodeEntry {
    uint16_t id       { 0 };
    bool     active   { false };
    uint8_t  last_seq { 0xFF };   // Para detección de replay
    uint32_t rx_count { 0 };
    uint32_t tx_count { 0 };
};

// ─────────────────────────────────────────────────────────────────────────────
//  Stack de protocolo
// ─────────────────────────────────────────────────────────────────────────────
class Protocol {
public:
    // Inicialización: ID propio + claves AES/HMAC
    static void init(uint16_t own_id,
                     const uint8_t aes_key[CRYPTO::KEY_LEN],
                     const uint8_t hmac_key[CRYPTO::KEY_LEN],
                     const uint8_t nonce[CRYPTO::AES_BLOCK]);

    // ── Lista blanca de nodos ─────────────────
    static bool add_node(uint16_t id);
    static bool remove_node(uint16_t id);
    static bool is_valid_node(uint16_t id);
    static NodeEntry* find_node(uint16_t id);
    static void print_node_list();     // vía UART

    // ── Construcción y envío ──────────────────
    // Construye paquete cifrado → lo mete en out_buf → devuelve tamaño
    static uint8_t build_packet(uint16_t dst_id, MsgType type,
                                const uint8_t* plain, uint8_t plain_len,
                                uint8_t* out_buf, uint8_t max_buf);

    // Envía con ACK de aplicación (retries automáticos)
    // send_fn: función de bajo nivel que envía bytes por RF
    static bool send_reliable(uint16_t dst_id, MsgType type,
                               const uint8_t* data, uint8_t len,
                               void (*send_fn)(uint16_t, const uint8_t*, uint8_t));

    // ── Recepción y parsing ───────────────────
    // Parsea raw_buf, verifica HMAC, descifra y valida nodo
    // Devuelve true si el paquete es válido y confiable
    static bool parse_packet(const uint8_t* raw_buf, uint8_t raw_len,
                              Packet& out_pkt,
                              uint8_t* out_plain, uint8_t& out_plain_len);

    // ── Callbacks ─────────────────────────────
    // Asignar para recibir notificaciones de paquetes válidos
    static void (*on_data_cb)(uint16_t src, const uint8_t* data, uint8_t len);
    static void (*on_join_req_cb)(uint16_t src);
    static void (*on_command_cb)(uint16_t src, const uint8_t* cmd, uint8_t len);

    // ── Handler de datos RX raw (llamar desde el callback del driver RF) ──
    static void handle_rx(const uint8_t* raw, uint8_t raw_len);

    // ── Getters ───────────────────────────────
    static uint16_t own_id()  { return _own_id; }
    static uint8_t  next_seq(){ return ++_seq; }

private:
    static uint16_t   _own_id;
    static uint8_t    _seq;
    static NodeEntry  _nodes[MAX_NODES];
    static uint8_t    _node_count;

    // ACK tracking
    static volatile bool    _ack_received;
    static volatile uint8_t _ack_seq;

    static void delay_ms(uint16_t ms);
};

} // namespace PROTO
