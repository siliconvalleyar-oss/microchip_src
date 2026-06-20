/**
 * main.cpp  –  Firmware PIC32MX795F512L + MRF24J40 + SSD1306
 *              Protocolo ZigBee-like con AES-128-CTR + HMAC-SHA256
 *
 * Compilar en MPLAB X con XC32, configurar:
 *   - PBCLK = 80 MHz (SYSCLK = 80 MHz, prescaler /1)
 *   - Fuse bits: FNOSC=FRCPLL o similar (según tu oscilador)
 *
 * Rol seleccionado por la macro ROLE_COORDINATOR / ROLE_END_DEVICE
 * (definir en Project Properties → XC32 → Preprocessor macros)
 *
 * ─── FLUJO DE DATOS ────────────────────────────────────────────────
 *
 * TX (Envío cifrado):
 *   [App] datos planos
 *     → CRYPTO::Crypto::encrypt_and_sign()
 *     → PROTO::Protocol::build_packet()     (añade header + HMAC)
 *     → DRV::Mrf24j40::send()              (cabecera MAC 802.15.4)
 *     → HAL::Spi4                           (bytes al MRF24J40 por SPI4)
 *     → RF 2.4 GHz
 *
 * RX (Recepción y validación):
 *   RF → MRF24J40 ISR (INT1) → DRV::Mrf24j40::isr_handler()
 *     → PROTO::Protocol::handle_rx()
 *     → PROTO::Protocol::parse_packet()
 *         → verifica lista blanca de nodos
 *         → verifica anti-replay (seq)
 *         → CRYPTO::Crypto::verify_and_decrypt()  (HMAC + descifrado)
 *     → callback on_data_cb / on_command_cb
 *     → App procesa datos planos
 */

#include <xc.h>
#include <sys/attribs.h>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "gpio.hpp"
#include "spi4.hpp"
#include "i2c.hpp"
#include "uart.hpp"
#include "mrf24j40.hpp"
#include "ssd1306.hpp"
#include "crypto.hpp"
#include "protocol.hpp"

// ── Fuse Bits (ajustar según hardware) ──────────────────────────────────────
#pragma config FPLLIDIV = DIV_2     // PLL Input Divider (8MHz/2=4MHz)
#pragma config FPLLMUL  = MUL_20   // PLL Multiplier (4MHz×20=80MHz)
#pragma config FPLLODIV = DIV_1    // PLL Output Divider
#pragma config FNOSC    = PRIPLL   // Oscillator selection: Primary PLL
#pragma config FSOSCEN  = OFF
#pragma config IESO     = OFF
#pragma config POSCMOD  = HS       // HS crystal
#pragma config OSCIOFNC = OFF
#pragma config FPBDIV   = DIV_1    // Peripheral Bus Clock = SYSCLK / 1 = 80 MHz
#pragma config FCKSM    = CSDCMD
#pragma config WDTPS    = PS1
#pragma config FWDTEN   = OFF      // Watchdog Timer OFF
#pragma config ICESEL   = ICS_PGx2
#pragma config DEBUG    = OFF

// ─────────────────────────────────────────────
//  Configuración de red (compartida entre ambos roles)
// ─────────────────────────────────────────────
namespace CFG {
    // AES-128 key (compartida con la RPi)
    const uint8_t AES_KEY[CRYPTO::KEY_LEN] = {
        0x2b,0x7e,0x15,0x16, 0x28,0xae,0xd2,0xa6,
        0xab,0xf7,0x15,0x88, 0x09,0xcf,0x4f,0x3c
    };
    // HMAC-SHA256 key (compartida con la RPi)
    const uint8_t HMAC_KEY[CRYPTO::KEY_LEN] = {
        0xde,0xad,0xbe,0xef, 0xca,0xfe,0xba,0xbe,
        0x01,0x23,0x45,0x67, 0x89,0xab,0xcd,0xef
    };
    // Nonce CTR (debe ser único por sesión; en producción: aleatorio o contador)
    const uint8_t NONCE[CRYPTO::AES_BLOCK] = {
        0xf0,0xf1,0xf2,0xf3, 0xf4,0xf5,0xf6,0xf7,
        0xf8,0xf9,0xfa,0xfb, 0xfc,0xfd,0xfe,0xff
    };

    constexpr uint16_t PAN_ID    = 0xCAFE;
    constexpr uint8_t  CHANNEL   = 15;

#if defined(ROLE_COORDINATOR)
    constexpr uint16_t OWN_ID   = 0x0001;
    constexpr uint16_t PEER_ID  = 0x0002;   // End Device conocido
    const DRV::NodeRole ROLE    = DRV::NodeRole::COORDINATOR;
#elif defined(ROLE_END_DEVICE)
    constexpr uint16_t OWN_ID   = 0x0002;
    constexpr uint16_t PEER_ID  = 0x0001;   // Coordinator
    const DRV::NodeRole ROLE    = DRV::NodeRole::END_DEVICE;
#else
    #error "Definir ROLE_COORDINATOR o ROLE_END_DEVICE"
#endif
}

// ─────────────────────────────────────────────
//  Estado global de la aplicación
// ─────────────────────────────────────────────
static char g_last_msg[22] = "---";
static bool g_crypto_ok    = true;
static bool g_joined       = false;

// ─────────────────────────────────────────────
//  ISR del MRF24J40 (INT1 → RD8, flanco descendente)
// ─────────────────────────────────────────────
void __ISR(_EXTERNAL_1_VECTOR, IPL4AUTO) INT1_ISR() {
    DRV::Mrf24j40::isr_handler();
    IFS0CLR = _IFS0_INT1IF_MASK;
}

// ─────────────────────────────────────────────
//  Función de envío RF (adaptador Protocol → Mrf24j40)
// ─────────────────────────────────────────────
static void rf_send(uint16_t dest, const uint8_t* data, uint8_t len) {
    DRV::Mrf24j40::send(dest, data, len);
}

// ─────────────────────────────────────────────
//  Callbacks de recepción del stack de protocolo
// ─────────────────────────────────────────────
static void on_data(uint16_t src, const uint8_t* data, uint8_t len) {
    // Actualizar pantalla
    uint8_t copy_len = len < 21 ? len : 21;
    memcpy(g_last_msg, data, copy_len);
    g_last_msg[copy_len] = '\0';
    g_crypto_ok = true;

    HAL::Uart::logf(HAL::LogLevel::RX, "DATA de 0x%04x: %s", src, g_last_msg);
    HAL::Gpio::led_toggle(HAL::Led::NET_ACTIVITY);

    // Responder ACK
    const uint8_t ack[] = {'O','K'};
    PROTO::Protocol::send_reliable(src, PROTO::MsgType::ACK, ack, 2, rf_send);
}

static void on_join_req(uint16_t src) {
    HAL::Uart::logf(HAL::LogLevel::NET, "JOIN_REQ de 0x%04x", src);

    // Responder JOIN_ACK con la dirección asignada
    uint8_t payload[2] = { (uint8_t)(src & 0xFF), (uint8_t)(src >> 8) };
    PROTO::Protocol::send_reliable(src, PROTO::MsgType::JOIN_ACK,
                                   payload, 2, rf_send);
    g_joined = true;
    DRV::Ssd1306::show_net_status("JOINED          ");
    DRV::Ssd1306::display();
}

static void on_command(uint16_t src, const uint8_t* cmd, uint8_t len) {
    char buf[22] = {};
    uint8_t n = len < 21 ? len : 21;
    memcpy(buf, cmd, n);

    HAL::Uart::logf(HAL::LogLevel::RX, "CMD de 0x%04x: %s", src, buf);

    char reply[22] = "ERR:UNKNOWN";
    if (strncmp(buf, "LED:TOGGLE", 10) == 0) {
        HAL::Gpio::led_toggle(HAL::Led::NODE_STATUS);
        memcpy(reply, "LED:TOGGLED", 11); reply[11] = '\0';
    } else if (strncmp(buf, "STATUS", 6) == 0) {
        snprintf(reply, sizeof(reply), "ID=0x%04x OK", CFG::OWN_ID);
    } else if (strncmp(buf, "RESET", 5) == 0) {
        HAL::Uart::log(HAL::LogLevel::WARN, "Reiniciando...");
        // SoftReset: escribir en RSWRST
        volatile uint32_t* rswrst = (volatile uint32_t*)0xBF80F600;
        *rswrst = 1;
        volatile uint32_t dummy = *rswrst; (void)dummy;
    }

    PROTO::Protocol::send_reliable(src, PROTO::MsgType::COMMAND_R,
                                   (const uint8_t*)reply,
                                   (uint8_t)strlen(reply), rf_send);
}

// ─────────────────────────────────────────────
//  Callback de MRF24J40 → delega al protocolo
// ─────────────────────────────────────────────
static void mrf_rx_callback() {
    auto* rxi  = DRV::Mrf24j40::get_rxinfo();
    int   dlen = DRV::Mrf24j40::rx_datalength();
    if (dlen <= 0) return;
    PROTO::Protocol::handle_rx(rxi->rx_data, (uint8_t)dlen);

    // Actualizar RSSI en pantalla
    int8_t rssi = DRV::Mrf24j40::rssi_dbm(rxi->rssi);
    DRV::Ssd1306::show_crypto_status(g_crypto_ok, rssi);
    DRV::Ssd1306::show_last_msg(g_last_msg);
    DRV::Ssd1306::display();
}

static void mrf_tx_callback() {
    auto* txi = DRV::Mrf24j40::get_txinfo();
    if (txi->tx_ok)
        HAL::Uart::log(HAL::LogLevel::TX, "MAC TX OK");
    else
        HAL::Uart::logf(HAL::LogLevel::ERROR,
            "MAC TX FAIL retries=%d busy=%d", txi->retries, txi->channel_busy);
}

// ─────────────────────────────────────────────
//  Delay simple (80 MHz PBCLK)
// ─────────────────────────────────────────────
static void delay_ms(uint32_t ms) {
    for (uint32_t m = 0; m < ms; m++)
        for (volatile uint32_t c = 0; c < 80000UL; c++) ;
}

// ─────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────
int main(void) {
    // 1. Interrupciones globales apagadas hasta terminar init
    INTCONbits.MVEC = 1;    // Multi-vector mode

    // 2. HAL init
    HAL::Gpio::init();
    HAL::Gpio::leds_set_byte(0xFF);   // Test LEDs al inicio
    delay_ms(200);
    HAL::Gpio::leds_all_off();

    HAL::Uart::init(80000000UL, 115200UL);
    HAL::Uart::log(HAL::LogLevel::INFO, "=== PIC32 ZigBee Secure Node ===");

    // 3. OLED
    DRV::Ssd1306::init();
    DRV::Ssd1306::show_all(CFG::ROLE, "INIT...", "---", false, 0);

    // 4. Protocolo + Crypto
    PROTO::Protocol::init(CFG::OWN_ID, CFG::AES_KEY, CFG::HMAC_KEY, CFG::NONCE);

    // Registrar nodos conocidos en la lista blanca
    PROTO::Protocol::add_node(CFG::PEER_ID);
    PROTO::Protocol::add_node(0x0001);  // Coordinator siempre permitido
    PROTO::Protocol::add_node(0x0002);  // End device conocido

    // Asignar callbacks del protocolo
    PROTO::Protocol::on_data_cb      = on_data;
    PROTO::Protocol::on_join_req_cb  = on_join_req;
    PROTO::Protocol::on_command_cb   = on_command;

    // 5. Driver RF
    DRV::Mrf24j40::on_rx_cb = mrf_rx_callback;
    DRV::Mrf24j40::on_tx_cb = mrf_tx_callback;
    DRV::Mrf24j40::init(CFG::ROLE, CFG::PAN_ID, CFG::OWN_ID, CFG::CHANNEL);

    // EUI-64
    const uint8_t eui[8] = {
        (uint8_t)(CFG::OWN_ID & 0xFF), 0x00, 0x00, 0x00,
        0x00, 0x00, (uint8_t)(CFG::OWN_ID >> 8), 0xAA
    };
    DRV::Mrf24j40::set_long_addr(eui);
    DRV::Mrf24j40::set_txpower(0x00);   // Máxima potencia

    // 6. Habilitar interrupciones
    __builtin_enable_interrupts();

    HAL::Uart::log(HAL::LogLevel::INFO, "Sistema listo");
    HAL::Gpio::led_on(HAL::Led::NODE_STATUS);

#if defined(ROLE_COORDINATOR)
    DRV::Ssd1306::show_net_status("ESPERANDO JOIN  ");
    HAL::Gpio::led_on(HAL::Led::COORDINATOR);

    // Coordinator: mostrar tabla de nodos por UART cada 30 s
    uint32_t node_print_cnt = 0;
    uint32_t hb_cnt = 0;

    while (1) {
        DRV::Mrf24j40::poll();
        delay_ms(10);

        hb_cnt++;
        if (hb_cnt >= 500) {   // ~5 s
            hb_cnt = 0;
            if (g_joined) {
                const uint8_t hb[] = {'H','B'};
                PROTO::Protocol::send_reliable(CFG::PEER_ID,
                    PROTO::MsgType::HEARTBEAT, hb, 2, rf_send);
            }
        }

        node_print_cnt++;
        if (node_print_cnt >= 3000) {  // ~30 s
            node_print_cnt = 0;
            PROTO::Protocol::print_node_list();
        }

        DRV::Ssd1306::show_node_mode(CFG::ROLE);
        DRV::Ssd1306::display();
    }

#elif defined(ROLE_END_DEVICE)
    HAL::Gpio::led_on(HAL::Led::END_DEVICE);
    DRV::Ssd1306::show_net_status("JOINING...      ");
    DRV::Ssd1306::display();

    // End Device: enviar JOIN_REQ al coordinator
    const uint8_t join_pl[] = {'J','O','I','N'};
    bool joined = false;
    for (int i = 0; i < 5 && !joined; i++) {
        HAL::Uart::logf(HAL::LogLevel::NET, "JOIN intento %d/5", i+1);
        joined = PROTO::Protocol::send_reliable(
            CFG::PEER_ID, PROTO::MsgType::JOIN_REQ,
            join_pl, 4, rf_send);
        if (!joined) delay_ms(2000);
    }

    if (joined) {
        g_joined = true;
        DRV::Ssd1306::show_net_status("JOINED          ");
        HAL::Uart::log(HAL::LogLevel::NET, "JOIN OK");
    } else {
        DRV::Ssd1306::show_net_status("JOIN FAILED     ");
        HAL::Uart::log(HAL::LogLevel::ERROR, "JOIN FAIL");
    }
    DRV::Ssd1306::display();

    // Loop: enviar datos de sensor cada 8 s, responder comandos
    uint32_t sensor_cnt = 0;

    while (1) {
        DRV::Mrf24j40::poll();
        delay_ms(10);

        sensor_cnt++;
        if (g_joined && sensor_cnt >= 800) {   // ~8 s
            sensor_cnt = 0;

            // Datos simulados de sensor
            char msg[40];
            // En un sistema real: leer ADC/I2C sensor
            static uint16_t fake_temp = 2350;  // 23.50 °C simulado
            static uint16_t fake_hum  = 6500;  // 65.00 % simulado
            fake_temp += 5; if (fake_temp > 2800) fake_temp = 2200;
            fake_hum  += 3; if (fake_hum  > 8000) fake_hum  = 4000;

            // Formatear sin sprintf float (evitar libm en PIC32 bare-metal)
            int n = 0;
            msg[n++]='T';msg[n++]='=';
            msg[n++]='0'+fake_temp/1000;
            msg[n++]='0'+(fake_temp/100)%10;
            msg[n++]='.';
            msg[n++]='0'+(fake_temp/10)%10;
            msg[n++]=' ';
            msg[n++]='H';msg[n++]='=';
            msg[n++]='0'+fake_hum/1000;
            msg[n++]='0'+(fake_hum/100)%10;
            msg[n++]='.';
            msg[n++]='0'+(fake_hum/10)%10;
            msg[n] = '\0';

            HAL::Uart::logf(HAL::LogLevel::TX, "Sensor: %s", msg);
            PROTO::Protocol::send_reliable(CFG::PEER_ID, PROTO::MsgType::DATA,
                                           (const uint8_t*)msg, (uint8_t)n,
                                           rf_send);
            DRV::Ssd1306::show_last_msg(msg);
            DRV::Ssd1306::display();
        }
    }
#endif

    return 0;
}
