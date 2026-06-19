/**
 * protocol.cpp  –  Protocolo de red seguro PIC32
 */

#include "protocol.hpp"
#include "uart.hpp"
#include "gpio.hpp"
#include "mrf24j40.hpp"

namespace PROTO {

// ── Statics ───────────────────────────────────────────────────────────────────
uint16_t         Protocol::_own_id      { 0 };
uint8_t          Protocol::_seq         { 0 };
NodeEntry        Protocol::_nodes[MAX_NODES] {};
uint8_t          Protocol::_node_count  { 0 };
volatile bool    Protocol::_ack_received{ false };
volatile uint8_t Protocol::_ack_seq     { 0 };

void (*Protocol::on_data_cb)(uint16_t, const uint8_t*, uint8_t)  = nullptr;
void (*Protocol::on_join_req_cb)(uint16_t)                        = nullptr;
void (*Protocol::on_command_cb)(uint16_t, const uint8_t*, uint8_t)= nullptr;

// ─────────────────────────────────────────────
//  init
// ─────────────────────────────────────────────
void Protocol::init(uint16_t own_id,
                    const uint8_t aes_key[CRYPTO::KEY_LEN],
                    const uint8_t hmac_key[CRYPTO::KEY_LEN],
                    const uint8_t nonce[CRYPTO::AES_BLOCK]) {
    _own_id = own_id;
    _seq    = 0;
    _node_count = 0;

    CRYPTO::Crypto::set_aes_key(aes_key);
    CRYPTO::Crypto::set_hmac_key(hmac_key);
    CRYPTO::Crypto::set_nonce(nonce);

    HAL::Uart::logf(HAL::LogLevel::NET,
        "Protocol init – ID=0x%04x", own_id);
}

// ─────────────────────────────────────────────
//  Lista blanca de nodos
// ─────────────────────────────────────────────
bool Protocol::add_node(uint16_t id) {
    if (_node_count >= MAX_NODES) return false;
    // Evitar duplicados
    for (uint8_t i = 0; i < _node_count; i++)
        if (_nodes[i].id == id) { _nodes[i].active = true; return true; }
    _nodes[_node_count].id     = id;
    _nodes[_node_count].active = true;
    _node_count++;
    HAL::Uart::logf(HAL::LogLevel::NET, "Nodo 0x%04x agregado a lista blanca", id);
    return true;
}

bool Protocol::remove_node(uint16_t id) {
    for (uint8_t i = 0; i < _node_count; i++) {
        if (_nodes[i].id == id) {
            _nodes[i].active = false;
            HAL::Uart::logf(HAL::LogLevel::NET, "Nodo 0x%04x desactivado", id);
            return true;
        }
    }
    return false;
}

bool Protocol::is_valid_node(uint16_t id) {
    // El propio nodo siempre es válido
    if (id == _own_id) return true;
    for (uint8_t i = 0; i < _node_count; i++)
        if (_nodes[i].id == id && _nodes[i].active) return true;
    return false;
}

NodeEntry* Protocol::find_node(uint16_t id) {
    for (uint8_t i = 0; i < _node_count; i++)
        if (_nodes[i].id == id) return &_nodes[i];
    return nullptr;
}

void Protocol::print_node_list() {
    HAL::Uart::log(HAL::LogLevel::NET, "=== Lista blanca de nodos ===");
    for (uint8_t i = 0; i < _node_count; i++) {
        HAL::Uart::logf(HAL::LogLevel::NET,
            "  [%d] 0x%04x  %s  RX=%u TX=%u",
            i, _nodes[i].id,
            _nodes[i].active ? "ACTIVO" : "INACT ",
            _nodes[i].rx_count,
            _nodes[i].tx_count);
    }
}

// ─────────────────────────────────────────────
//  build_packet: cifra y serializa
// ─────────────────────────────────────────────
uint8_t Protocol::build_packet(uint16_t dst_id, MsgType type,
                               const uint8_t* plain, uint8_t plain_len,
                               uint8_t* out_buf, uint8_t max_buf)
{
    if (plain_len > MAX_PAYLOAD) plain_len = MAX_PAYLOAD;

    Packet pkt;
    pkt.magic  = MAGIC;
    pkt.src_id = _own_id;
    pkt.dst_id = dst_id;
    pkt.seq    = ++_seq;
    pkt.type   = (uint8_t)type;
    pkt.plen   = plain_len;

    // Cifrar payload + calcular HMAC
    uint8_t cipher_len = CRYPTO::Crypto::encrypt_and_sign(
        plain, plain_len, pkt.payload, pkt.hmac);
    pkt.plen = cipher_len;

    uint8_t sz = pkt.wire_size();
    if (sz > max_buf) return 0;

    // Serializar manualmente (sin memcpy de struct para evitar padding issues)
    uint8_t* p = out_buf;
    *p++ = (uint8_t)(pkt.magic  & 0xFF);
    *p++ = (uint8_t)(pkt.magic  >> 8);
    *p++ = (uint8_t)(pkt.src_id & 0xFF);
    *p++ = (uint8_t)(pkt.src_id >> 8);
    *p++ = (uint8_t)(pkt.dst_id & 0xFF);
    *p++ = (uint8_t)(pkt.dst_id >> 8);
    *p++ = pkt.seq;
    *p++ = pkt.type;
    *p++ = pkt.plen;
    memcpy(p, pkt.payload, pkt.plen); p += pkt.plen;
    memcpy(p, pkt.hmac, CRYPTO::HMAC_LEN); p += CRYPTO::HMAC_LEN;

    return (uint8_t)(p - out_buf);
}

// ─────────────────────────────────────────────
//  send_reliable: envía con ACK de aplicación
// ─────────────────────────────────────────────
bool Protocol::send_reliable(uint16_t dst_id, MsgType type,
                              const uint8_t* data, uint8_t len,
                              void (*send_fn)(uint16_t, const uint8_t*, uint8_t))
{
    uint8_t buf[115];
    uint8_t sz = build_packet(dst_id, type, data, len, buf, sizeof(buf));
    if (sz == 0) {
        HAL::Uart::log(HAL::LogLevel::ERROR, "build_packet: tamaño 0");
        return false;
    }

    uint8_t expected_seq = _seq;

    for (uint8_t attempt = 0; attempt < MAX_RETRIES; attempt++) {
        _ack_received = false;

        // Enviar vía RF
        send_fn(dst_id, buf, sz);

        HAL::Uart::log_tx_frame(dst_id, data, len);
        HAL::Gpio::led_on(HAL::Led::TX_ACTIVE);
        HAL::Gpio::beep(20);  // Beep corto al transmitir

        // Esperar ACK (polling con timeout ~ACK_TIMEOUT ms)
        // Basado en PBCLK 80 MHz ~800 iter/ms
        for (uint32_t t = 0; t < (uint32_t)(ACK_TIMEOUT * 8000UL); t++) {
            DRV::Mrf24j40::poll();
            if (_ack_received && _ack_seq == expected_seq) {
                HAL::Gpio::led_off(HAL::Led::TX_ACTIVE);
                NodeEntry* ne = find_node(dst_id);
                if (ne) ne->tx_count++;
                return true;
            }
        }

        HAL::Uart::logf(HAL::LogLevel::WARN,
            "Sin ACK intento %d/%d", attempt+1, MAX_RETRIES);
        HAL::Gpio::led_off(HAL::Led::TX_ACTIVE);
    }

    HAL::Uart::logf(HAL::LogLevel::ERROR, "FALLO TX a 0x%04x", dst_id);
    HAL::Gpio::led_on(HAL::Led::CRYPTO_ERR);
    return false;
}

// ─────────────────────────────────────────────
//  parse_packet: deserializa, verifica HMAC, descifra
// ─────────────────────────────────────────────
bool Protocol::parse_packet(const uint8_t* raw, uint8_t raw_len,
                             Packet& out, uint8_t* plain, uint8_t& plain_len)
{
    // Mínimo: 2+2+2+1+1+1+32 = 41 bytes
    if (raw_len < 41) return false;

    const uint8_t* p = raw;
    out.magic  = (uint16_t)(p[0] | (p[1] << 8)); p += 2;
    out.src_id = (uint16_t)(p[0] | (p[1] << 8)); p += 2;
    out.dst_id = (uint16_t)(p[0] | (p[1] << 8)); p += 2;
    out.seq    = *p++;
    out.type   = *p++;
    out.plen   = *p++;

    if (!out.valid_magic()) {
        HAL::Uart::log(HAL::LogLevel::ERROR, "Magic inválido");
        return false;
    }
    if (out.plen > MAX_PAYLOAD) {
        HAL::Uart::log(HAL::LogLevel::ERROR, "plen demasiado grande");
        return false;
    }
    if ((uint8_t)(9 + out.plen + CRYPTO::HMAC_LEN) > raw_len) {
        HAL::Uart::log(HAL::LogLevel::ERROR, "Paquete truncado");
        return false;
    }

    memcpy(out.payload, p, out.plen); p += out.plen;
    memcpy(out.hmac, p, CRYPTO::HMAC_LEN);

    // Verificar origen
    if (!is_valid_node(out.src_id)) {
        HAL::Uart::logf(HAL::LogLevel::ERROR,
            "Nodo desconocido: 0x%04x – rechazado", out.src_id);
        return false;
    }

    // Anti-replay: verificar que el seq sea nuevo
    NodeEntry* ne = find_node(out.src_id);
    if (ne) {
        if (out.seq == ne->last_seq) {
            HAL::Uart::logf(HAL::LogLevel::WARN,
                "Replay detectado (seq=%d) de 0x%04x", out.seq, out.src_id);
            return false;
        }
        ne->last_seq = out.seq;
        ne->rx_count++;
    }

    // Verificar HMAC + descifrar
    bool ok = CRYPTO::Crypto::verify_and_decrypt(
        out.payload, out.plen, out.hmac, plain, plain_len);

    if (ok) {
        HAL::Gpio::led_on(HAL::Led::CRYPTO_OK);
        HAL::Gpio::led_off(HAL::Led::CRYPTO_ERR);
        HAL::Uart::log_crypto(true, "HMAC OK");
    } else {
        HAL::Gpio::led_on(HAL::Led::CRYPTO_ERR);
        HAL::Gpio::led_off(HAL::Led::CRYPTO_OK);
        HAL::Uart::log_crypto(false, "HMAC FAIL – paquete descartado");
    }
    return ok;
}

// ─────────────────────────────────────────────
//  handle_rx: dispatcher de paquetes recibidos
// ─────────────────────────────────────────────
void Protocol::handle_rx(const uint8_t* raw, uint8_t raw_len) {
    HAL::Gpio::led_toggle(HAL::Led::NET_ACTIVITY);
    HAL::Gpio::beep(10);  // Beep al recibir

    Packet pkt;
    uint8_t plain[MAX_PAYLOAD];
    uint8_t plain_len = 0;

    if (!parse_packet(raw, raw_len, pkt, plain, plain_len)) return;

    HAL::Uart::log_rx_frame(pkt.src_id, plain, plain_len);

    MsgType type = (MsgType)pkt.type;

    // Tracking de ACK
    if (type == MsgType::ACK) {
        _ack_received = true;
        _ack_seq      = pkt.seq;
        return;
    }

    // Dispatching
    switch (type) {
        case MsgType::DATA:
            if (on_data_cb) on_data_cb(pkt.src_id, plain, plain_len);
            break;
        case MsgType::JOIN_REQ:
            // Auto-agregar el nodo que solicita unirse
            add_node(pkt.src_id);
            if (on_join_req_cb) on_join_req_cb(pkt.src_id);
            break;
        case MsgType::COMMAND:
            if (on_command_cb) on_command_cb(pkt.src_id, plain, plain_len);
            break;
        case MsgType::HEARTBEAT:
        case MsgType::HEARTBEAT_R:
            // Log de keep-alive
            HAL::Uart::logf(HAL::LogLevel::NET,
                "HEARTBEAT de 0x%04x", pkt.src_id);
            break;
        default:
            HAL::Uart::logf(HAL::LogLevel::WARN,
                "Tipo desconocido: 0x%02x de 0x%04x", pkt.type, pkt.src_id);
            break;
    }
}

void Protocol::delay_ms(uint16_t ms) {
    for (uint16_t m = 0; m < ms; m++)
        for (volatile uint32_t c = 0; c < 80000UL; c++) ;
}

} // namespace PROTO
