#pragma once
/**
 * crypto.hpp  –  AES-128-CTR  +  HMAC-SHA256 para PIC32MX795F512L
 *
 * Implementación "bare-metal" sin dependencias externas.
 * Optimizada para microcontroladores (mínimo uso de stack/RAM).
 *
 * Uso:
 *   1. Configurar clave AES de 128-bit y clave HMAC
 *   2. Para cifrar + autenticar:
 *        Crypto::encrypt_and_sign(plain, plen, out_cipher, out_hmac)
 *   3. Para descifrar + verificar:
 *        Crypto::verify_and_decrypt(cipher, clen, hmac, out_plain)
 *
 * AES-128 en modo CTR (Counter):
 *   - No requiere padding (CTR = stream cipher over block cipher)
 *   - IV/nonce de 16 bytes (los primeros 4 bytes son el contador)
 *   - Nonce pre-configurado con set_nonce()
 *
 * HMAC-SHA256:
 *   - 32 bytes de etiqueta de autenticidad
 *   - Calculado sobre el ciphertext (Encrypt-then-MAC)
 */

#include <cstdint>
#include <cstring>

namespace CRYPTO {

constexpr uint8_t AES_BLOCK   = 16;
constexpr uint8_t HMAC_LEN    = 32;
constexpr uint8_t KEY_LEN     = 16;   // AES-128

// ─────────────────────────────────────────────────────────────────────────────
//  AES-128 (solo cifrado; CTR usa solo el path de cifrado del bloque)
// ─────────────────────────────────────────────────────────────────────────────
class Aes128 {
public:
    void set_key(const uint8_t key[KEY_LEN]);

    // Cifra/descifra un bloque de 16 bytes in-place (ECB)
    void encrypt_block(uint8_t block[AES_BLOCK]);

private:
    uint32_t _rk[44];   // Round keys (44 words × 4 bytes = 176 bytes)

    void key_expand(const uint8_t key[KEY_LEN]);
    static uint32_t sub_word(uint32_t w);
    static uint32_t rot_word(uint32_t w);
    static uint32_t xtime(uint32_t v);

    static const uint8_t SBOX[256];
    static const uint8_t RCON[11];
};

// ─────────────────────────────────────────────────────────────────────────────
//  SHA-256 (bloque de 64 bytes, hash de 32 bytes)
// ─────────────────────────────────────────────────────────────────────────────
class Sha256 {
public:
    void init();
    void update(const uint8_t* data, uint32_t len);
    void final(uint8_t hash[HMAC_LEN]);

private:
    uint32_t _state[8];
    uint8_t  _buf[64];
    uint32_t _bit_count_lo;
    uint32_t _bit_count_hi;
    uint32_t _buf_idx;

    void process_block(const uint8_t block[64]);

    static constexpr uint32_t K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
        0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
        0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
        0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
        0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
        0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
        0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
        0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
        0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    static uint32_t rotr(uint32_t x, uint8_t n) { return (x >> n) | (x << (32 - n)); }
    static uint32_t Ch (uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static uint32_t Sig0(uint32_t x) { return rotr(x,2)^rotr(x,13)^rotr(x,22); }
    static uint32_t Sig1(uint32_t x) { return rotr(x,6)^rotr(x,11)^rotr(x,25); }
    static uint32_t sig0(uint32_t x) { return rotr(x,7)^rotr(x,18)^(x>>3);  }
    static uint32_t sig1(uint32_t x) { return rotr(x,17)^rotr(x,19)^(x>>10); }
};

// ─────────────────────────────────────────────────────────────────────────────
//  HMAC-SHA256
// ─────────────────────────────────────────────────────────────────────────────
class HmacSha256 {
public:
    void init(const uint8_t* key, uint8_t klen);
    void update(const uint8_t* data, uint32_t len);
    void final(uint8_t mac[HMAC_LEN]);

private:
    Sha256  _sha {};
    uint8_t _opad_key[64] {};
};

// ─────────────────────────────────────────────────────────────────────────────
//  Capa de aplicación: cifrado + autenticación
// ─────────────────────────────────────────────────────────────────────────────
class Crypto {
public:
    // Configurar claves (llamar una vez al inicio)
    static void set_aes_key  (const uint8_t key[KEY_LEN]);
    static void set_hmac_key (const uint8_t key[KEY_LEN]);
    static void set_nonce    (const uint8_t nonce[AES_BLOCK]);

    // Cifrar plain_len bytes; out_cipher debe tener ≥ plain_len bytes
    // out_hmac recibe los 32 bytes de MAC
    // Retorna bytes cifrados escritos
    static uint8_t encrypt_and_sign(
        const uint8_t* plain,     uint8_t plain_len,
              uint8_t* out_cipher,
              uint8_t  out_hmac[HMAC_LEN]);

    // Descifrar y verificar; out_plain debe tener ≥ cipher_len bytes
    // Retorna true si el HMAC es válido
    static bool verify_and_decrypt(
        const uint8_t* cipher,    uint8_t cipher_len,
        const uint8_t  in_hmac[HMAC_LEN],
              uint8_t* out_plain,
              uint8_t& out_plain_len);

private:
    static void aes_ctr_crypt(
        const uint8_t* in, uint8_t len,
              uint8_t* out,
              uint8_t  ctr_offset = 0);

    static bool hmac_equal(const uint8_t a[HMAC_LEN], const uint8_t b[HMAC_LEN]);

    static Aes128  _aes;
    static uint8_t _nonce[AES_BLOCK];
    static uint8_t _hmac_key[KEY_LEN];
    static bool    _initialized;
};

} // namespace CRYPTO
