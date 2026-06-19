/**
 * crypto.cpp  –  AES-128-CTR + HMAC-SHA256 para PIC32MX795F512L
 *
 * AES S-Box: tabla de 256 bytes en ROM (Flash) → no consume RAM.
 * SHA-256: implementación compacta (un solo bloque de procesamiento en stack).
 * Sin uso de malloc, sin dependencias del sistema.
 */

#include "crypto.hpp"

namespace CRYPTO {

// ─────────────────────────────────────────────
//  AES S-Box (ROM)
// ─────────────────────────────────────────────
const uint8_t Aes128::SBOX[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

const uint8_t Aes128::RCON[11] = {
    0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36
};

// ─────────────────────────────────────────────
//  AES Key Expansion
// ─────────────────────────────────────────────
uint32_t Aes128::sub_word(uint32_t w) {
    return ((uint32_t)SBOX[(w>>24)&0xFF]<<24) |
           ((uint32_t)SBOX[(w>>16)&0xFF]<<16) |
           ((uint32_t)SBOX[(w>> 8)&0xFF]<< 8) |
           ((uint32_t)SBOX[(w    )&0xFF]);
}

uint32_t Aes128::rot_word(uint32_t w) { return (w<<8)|(w>>24); }

void Aes128::key_expand(const uint8_t key[KEY_LEN]) {
    for (int i = 0; i < 4; i++) {
        _rk[i] = ((uint32_t)key[4*i  ]<<24) | ((uint32_t)key[4*i+1]<<16) |
                 ((uint32_t)key[4*i+2]<< 8) |  (uint32_t)key[4*i+3];
    }
    for (int i = 4; i < 44; i++) {
        uint32_t temp = _rk[i-1];
        if (i % 4 == 0) temp = sub_word(rot_word(temp)) ^ ((uint32_t)RCON[i/4] << 24);
        _rk[i] = _rk[i-4] ^ temp;
    }
}

void Aes128::set_key(const uint8_t key[KEY_LEN]) { key_expand(key); }

// ─────────────────────────────────────────────
//  AES Encrypt block (10 rondas AES-128)
// ─────────────────────────────────────────────
void Aes128::encrypt_block(uint8_t b[AES_BLOCK]) {
    // Estado como 4×4 matrix en column-major
    uint32_t s[4];
    for (int i = 0; i < 4; i++)
        s[i] = ((uint32_t)b[4*i]<<24)|((uint32_t)b[4*i+1]<<16)|
               ((uint32_t)b[4*i+2]<<8)|(uint32_t)b[4*i+3];

    // AddRoundKey inicial
    for (int i = 0; i < 4; i++) s[i] ^= _rk[i];

    // 10 rondas
    for (int rnd = 1; rnd <= 10; rnd++) {
        // SubBytes
        for (int i = 0; i < 4; i++)
            s[i] = ((uint32_t)SBOX[(s[i]>>24)&0xFF]<<24)|
                   ((uint32_t)SBOX[(s[i]>>16)&0xFF]<<16)|
                   ((uint32_t)SBOX[(s[i]>> 8)&0xFF]<< 8)|
                   ((uint32_t)SBOX[(s[i]    )&0xFF]);

        // ShiftRows (en row-major equivale a rotar columnas)
        uint32_t t0 = (s[0]&0xFF000000)|((s[1]>>8)&0x00FF0000)|
                      ((s[2]>>16)&0x0000FF00)|((s[3]>>24)&0x000000FF);
        uint32_t t1 = (s[1]&0xFF000000)|((s[2]>>8)&0x00FF0000)|
                      ((s[3]>>16)&0x0000FF00)|((s[0]>>24)&0x000000FF);
        uint32_t t2 = (s[2]&0xFF000000)|((s[3]>>8)&0x00FF0000)|
                      ((s[0]>>16)&0x0000FF00)|((s[1]>>24)&0x000000FF);
        uint32_t t3 = (s[3]&0xFF000000)|((s[0]>>8)&0x00FF0000)|
                      ((s[1]>>16)&0x0000FF00)|((s[2]>>24)&0x000000FF);

        if (rnd < 10) {
            // MixColumns (operaciones en GF(2^8))
            auto mc = [](uint32_t w) -> uint32_t {
                uint8_t a0=(w>>24)&0xFF, a1=(w>>16)&0xFF,
                        a2=(w>>8)&0xFF,  a3=w&0xFF;
                auto x2 = [](uint8_t a) -> uint8_t {
                    return (uint8_t)((a<<1)^(a&0x80?0x1b:0));
                };
                uint8_t r0=(uint8_t)(x2(a0)^(x2(a1)^a1)^a2^a3);
                uint8_t r1=(uint8_t)(a0^x2(a1)^(x2(a2)^a2)^a3);
                uint8_t r2=(uint8_t)(a0^a1^x2(a2)^(x2(a3)^a3));
                uint8_t r3=(uint8_t)((x2(a0)^a0)^a1^a2^x2(a3));
                return ((uint32_t)r0<<24)|((uint32_t)r1<<16)|
                       ((uint32_t)r2<<8)|(uint32_t)r3;
            };
            t0=mc(t0); t1=mc(t1); t2=mc(t2); t3=mc(t3);
        }

        // AddRoundKey
        int base = rnd * 4;
        s[0] = t0 ^ _rk[base];
        s[1] = t1 ^ _rk[base+1];
        s[2] = t2 ^ _rk[base+2];
        s[3] = t3 ^ _rk[base+3];
    }

    // Serializar
    for (int i = 0; i < 4; i++) {
        b[4*i  ] = (uint8_t)(s[i]>>24);
        b[4*i+1] = (uint8_t)(s[i]>>16);
        b[4*i+2] = (uint8_t)(s[i]>> 8);
        b[4*i+3] = (uint8_t)(s[i]);
    }
}

// ─────────────────────────────────────────────
//  SHA-256
// ─────────────────────────────────────────────
static const uint32_t SHA256_H0[8] = {
    0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
    0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
};

void Sha256::init() {
    for (int i = 0; i < 8; i++) _state[i] = SHA256_H0[i];
    _bit_count_lo = _bit_count_hi = _buf_idx = 0;
}

void Sha256::process_block(const uint8_t blk[64]) {
    uint32_t w[64];
    for (int i = 0; i < 16; i++)
        w[i] = ((uint32_t)blk[4*i]<<24)|((uint32_t)blk[4*i+1]<<16)|
               ((uint32_t)blk[4*i+2]<<8)|(uint32_t)blk[4*i+3];
    for (int i = 16; i < 64; i++)
        w[i] = sig1(w[i-2]) + w[i-7] + sig0(w[i-15]) + w[i-16];

    uint32_t a=_state[0],b=_state[1],c=_state[2],d=_state[3],
             e=_state[4],f=_state[5],g=_state[6],h=_state[7];

    for (int i = 0; i < 64; i++) {
        uint32_t T1 = h + Sig1(e) + Ch(e,f,g) + K[i] + w[i];
        uint32_t T2 = Sig0(a) + Maj(a,b,c);
        h=g; g=f; f=e; e=d+T1; d=c; c=b; b=a; a=T1+T2;
    }
    _state[0]+=a; _state[1]+=b; _state[2]+=c; _state[3]+=d;
    _state[4]+=e; _state[5]+=f; _state[6]+=g; _state[7]+=h;
}

void Sha256::update(const uint8_t* data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        _buf[_buf_idx++] = data[i];
        if (_buf_idx == 64) { process_block(_buf); _buf_idx = 0; }
        _bit_count_lo += 8;
        if (_bit_count_lo == 0) _bit_count_hi++;
    }
}

void Sha256::final(uint8_t hash[HMAC_LEN]) {
    _buf[_buf_idx++] = 0x80;
    if (_buf_idx > 56) {
        while (_buf_idx < 64) _buf[_buf_idx++] = 0;
        process_block(_buf);
        _buf_idx = 0;
    }
    while (_buf_idx < 56) _buf[_buf_idx++] = 0;
    // Append bit length (big-endian 64-bit)
    for (int i = 0; i < 4; i++) _buf[56+i] = (uint8_t)(_bit_count_hi >> (24-8*i));
    for (int i = 0; i < 4; i++) _buf[60+i] = (uint8_t)(_bit_count_lo >> (24-8*i));
    process_block(_buf);
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 4; j++)
            hash[4*i+j] = (uint8_t)(_state[i] >> (24-8*j));
}

// ─────────────────────────────────────────────
//  HMAC-SHA256
// ─────────────────────────────────────────────
void HmacSha256::init(const uint8_t* key, uint8_t klen) {
    uint8_t ipad_key[64] {};
    memset(_opad_key, 0, 64);
    if (klen > 64) klen = 64;
    memcpy(ipad_key,  key, klen);
    memcpy(_opad_key, key, klen);
    for (int i = 0; i < 64; i++) { ipad_key[i] ^= 0x36; _opad_key[i] ^= 0x5C; }
    _sha.init();
    _sha.update(ipad_key, 64);
}

void HmacSha256::update(const uint8_t* data, uint32_t len) { _sha.update(data, len); }

void HmacSha256::final(uint8_t mac[HMAC_LEN]) {
    uint8_t inner[HMAC_LEN];
    _sha.final(inner);
    _sha.init();
    _sha.update(_opad_key, 64);
    _sha.update(inner, HMAC_LEN);
    _sha.final(mac);
}

// ─────────────────────────────────────────────
//  Crypto – statics
// ─────────────────────────────────────────────
Aes128  Crypto::_aes {};
uint8_t Crypto::_nonce[AES_BLOCK] {};
uint8_t Crypto::_hmac_key[KEY_LEN] {};
bool    Crypto::_initialized { false };

void Crypto::set_aes_key(const uint8_t key[KEY_LEN])  { _aes.set_key(key); _initialized = true; }
void Crypto::set_hmac_key(const uint8_t key[KEY_LEN]) { memcpy(_hmac_key, key, KEY_LEN); }
void Crypto::set_nonce(const uint8_t n[AES_BLOCK])    { memcpy(_nonce, n, AES_BLOCK); }

// AES-CTR: cifra/descifra (misma operación)
void Crypto::aes_ctr_crypt(const uint8_t* in, uint8_t len, uint8_t* out, uint8_t ctr_off) {
    uint8_t ctr_block[AES_BLOCK];
    uint32_t counter = ctr_off;
    for (uint8_t pos = 0; pos < len; ) {
        memcpy(ctr_block, _nonce, AES_BLOCK);
        // Contador en los últimos 4 bytes (big-endian)
        ctr_block[12] = (uint8_t)(counter >> 24);
        ctr_block[13] = (uint8_t)(counter >> 16);
        ctr_block[14] = (uint8_t)(counter >>  8);
        ctr_block[15] = (uint8_t)(counter);
        _aes.encrypt_block(ctr_block);  // keystream block
        counter++;
        uint8_t chunk = (len - pos) > AES_BLOCK ? AES_BLOCK : (len - pos);
        for (uint8_t b = 0; b < chunk; b++) out[pos + b] = in[pos + b] ^ ctr_block[b];
        pos += chunk;
    }
}

uint8_t Crypto::encrypt_and_sign(
    const uint8_t* plain, uint8_t plain_len,
          uint8_t* out_cipher,
          uint8_t  out_hmac[HMAC_LEN])
{
    // 1. Cifrar
    aes_ctr_crypt(plain, plain_len, out_cipher, 0);

    // 2. HMAC sobre ciphertext (Encrypt-then-MAC)
    HmacSha256 hmac;
    hmac.init(_hmac_key, KEY_LEN);
    hmac.update(out_cipher, plain_len);
    hmac.final(out_hmac);

    return plain_len;
}

bool Crypto::hmac_equal(const uint8_t a[HMAC_LEN], const uint8_t b[HMAC_LEN]) {
    // Comparación en tiempo constante (evita timing attacks)
    uint8_t diff = 0;
    for (int i = 0; i < HMAC_LEN; i++) diff |= (a[i] ^ b[i]);
    return diff == 0;
}

bool Crypto::verify_and_decrypt(
    const uint8_t* cipher, uint8_t cipher_len,
    const uint8_t  in_hmac[HMAC_LEN],
          uint8_t* out_plain,
          uint8_t& out_plain_len)
{
    // 1. Verificar HMAC
    uint8_t computed[HMAC_LEN];
    HmacSha256 hmac;
    hmac.init(_hmac_key, KEY_LEN);
    hmac.update(cipher, cipher_len);
    hmac.final(computed);

    if (!hmac_equal(computed, in_hmac)) {
        out_plain_len = 0;
        return false;
    }

    // 2. Descifrar (CTR = mismo que cifrar)
    aes_ctr_crypt(cipher, cipher_len, out_plain, 0);
    out_plain_len = cipher_len;
    return true;
}

} // namespace CRYPTO
