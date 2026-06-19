# 🧩 Microchip PIC Projects — MPLAB X Workspace

Repositorio de proyectos MPLAB X para microcontroladores **Microchip PIC18F** y **PIC32MX**. Cada proyecto incluye su propio README.md con descripción detallada, pines y periféricos utilizados.

---

## 📋 Índice de Proyectos

### 🔵 PIC18F — 8-bit

| Proyecto | MCU | Descripción | Periféricos |
|----------|-----|-------------|-------------|
| [`pic18f452_MAF.X`](pic18f452_MAF.X/) | 18F452 | Frequency Meter + Signal Regenerator | CCP, Timer, Comparator, GPIO |
| [`pic18f4550_i2c.X`](pic18f4550_i2c.X/) | 18F4550 | I2C Master + LED animation | MSSP (I2C), GPIO |
| [`pic18f4550_led_rd0_rd3.X`](pic18f4550_led_rd0_rd3.X/) | 18F4550 | LED sequence demo | GPIO |
| [`pic18f4550_pwm.X`](pic18f4550_pwm.X/) | 18F4550 | Hardware PWM with fade (CCP1) | CCP1, Timer2 |
| [`pic18f4550_terminator.X`](pic18f4550_terminator.X/) | 18F4550 | Software PWM 7-mode controller | GPIO, PWM (software) |
| [`pic18f4550_ws2812_adafruit.X`](pic18f4550_ws2812_adafruit.X/) | 18F4550 | WS2812 RGB LED strip driver | GPIO (timing NOP) |
| [`pic18f45550_out_interrupt.X`](pic18f45550_out_interrupt.X/) | 18F45550 | EXT INT0 + ADC + Button demo | INT0, ADC, GPIO |
| [`pic18f4620_LCR_oled.X`](pic18f4620_LCR_oled.X/) | 18F4620 | LCR Meter with OLED | I2C, OLED, ADC |
| [`pic18f4620_MAF.X`](pic18f4620_MAF.X/) | 18F4620 | Signal regenerator + variable freq | CCP1/CCP2, Comparator, Timer |
| [`pic18f4620_detector`](pic18f4620_detector/) | 18F4620 | **Metal detector** (full featured) | ADC, PWM, Timer, EEPROM, LCD |
| [`pic18f4620_mrf24j40_tx_rx_claude.X`](pic18f4620_mrf24j40_tx_rx_claude.X/) | 18F4620 | MRF24J40 wireless (Claude version) | SPI, USART, I2C, MRF24J40 |
| [`pic18f4620_mrf24j40_tx_rx_deepseek.X`](pic18f4620_mrf24j40_tx_rx_deepseek.X/) | 18F4620 | MRF24J40 wireless (DeepSeek version) | SPI, INT1, USART, MRF24J40 |
| [`pic18f4620_ssd1306_oled.X`](pic18f4620_ssd1306_oled.X/) | 18F4620 | SSD1306 OLED I2C text demo | I2C (bit-bang), OLED |
| [`pic18f4620_usart.X`](pic18f4620_usart.X/) | 18F4620 | USART serial communication | EUSART, GPIO |
| [`pic18f46j50.X`](pic18f46j50.X/) | 18F46J50 | OLED + EEPROM data storage | I2C, OLED, EEPROM |
| [`pic18f46j50_i2c.X`](pic18f46j50_i2c.X/) | 18F46J50 | I2C OLED with button control | I2C (bit-bang), OLED |
| [`pic18f46j50_i2c_wallet_freebuff.X`](pic18f46j50_i2c_wallet_freebuff.X/) | 18F46J50 | Crypto wallet (Trezor-like, OLED) | I2C, OLED, GPIO |
| [`pic18f46j50_oled.X`](pic18f46j50_oled.X/) | 18F46J50 | LED sequencer + OLED display | MSSP2 (I2C), OLED, GPIO |
| [`pic18f46j50_tmp.X`](pic18f46j50_tmp.X/) | 18F46J50 | Temperature sensor (I2C) | I2C, TMP sensor |
| [`pic18f46j50_wallet.X`](pic18f46j50_wallet.X/) | 18F46J50 | USB CDC device (LED commands) | USB (CDC), GPIO, MLA stack |
| [`pic18f46j50_wallet_usb.X`](pic18f46j50_wallet_usb.X/) | 18F46J50 | Crypto wallet + USB CDC + OLED | USB, I2C, OLED |

### 🟢 PIC32MX795 — 32-bit MIPS

| Proyecto | MCU | Descripción | Periféricos |
|----------|-----|-------------|-------------|
| [`pic32mx795_Epaper_Display.X`](pic32mx795_Epaper_Display.X/) | 32MX795 | Base driver for Pervasive Displays E-Paper | SPI4, GPIO |
| [`pic32mx795_Epaper_Display_1v1.X`](pic32mx795_Epaper_Display_1v1.X/) | 32MX795 | E-Paper v1.1 + pin test utilities | SPI4, GPIO |
| [`pic32mx795_Epaper_Display_1v2.X`](pic32mx795_Epaper_Display_1v2.X/) | 32MX795 | E-Paper v1.2 + pin test | SPI4, GPIO |
| [`pic32mx795_Epaper_Display_1v3.X`](pic32mx795_Epaper_Display_1v3.X/) | 32MX795 | E-Paper v1.3 refinado | SPI4, GPIO |
| [`pic32mx795_Epaper_Display_1v4_claude.X`](pic32mx795_Epaper_Display_1v4_claude.X/) | 32MX795 | **E-Paper digital clock** (zone refresh) | SPI4, GPIO, Timer |
| [`pic32mx795_Epaper_Display_1v5.X`](pic32mx795_Epaper_Display_1v5.X/) | 32MX795 | E-Paper v1.5 + Code 39 barcode | SPI4, GPIO |
| [`pic32mx795_mrf24j40.X`](pic32mx795_mrf24j40.X/) | 32MX795 | MRF24J40 wireless transceiver | SPI, MRF24J40 |
| [`pic32mx795_st7789_version1.0.X`](pic32mx795_st7789_version1.0.X/) | 32MX795 | ST7789 display driver — v1.0 | SPI4, TFT |
| [`pic32mx795_st7789_version1.1.X`](pic32mx795_st7789_version1.1.X/) | 32MX795 | ST7789 display driver — v1.1 | SPI4, TFT |
| [`pic32mx795_st7789_version1.2.X`](pic32mx795_st7789_version1.2.X/) | 32MX795 | ST7789 display driver — v1.2 | SPI4, TFT |
| [`pic32mx795_st7789_version1.3.X`](pic32mx795_st7789_version1.3.X/) | 32MX795 | **Space shooter game** (ST7789) | SPI4, TFT, GPIO |
| [`pic32mx795_st7789_version1.4.X`](pic32mx795_st7789_version1.4.X/) | 32MX795 | ST7789 game + sound effects | SPI4, TFT, Sound |
| [`pic32mx795_st7789_version1.5.X`](pic32mx795_st7789_version1.5.X/) | 32MX795 | **Pac-Man game** (ST7789) | SPI4, TFT, Sound |
| [`pic32mx795_test_epaper_pins.X`](pic32mx795_test_epaper_pins.X/) | 32MX795 | SPI4 pin test for E-Paper con PPS | SPI4, PPS, GPIO |

---

## 🗺️ Roadmap / Estado de proyectos

### 🔵 PIC18F — 8-bit

| Proyecto | Estado | Próximos pasos |
|----------|--------|----------------|
| [`pic18f452_MAF.X`](pic18f452_MAF.X/) | ✅ Completo | — |
| [`pic18f4550_i2c.X`](pic18f4550_i2c.X/) | ✅ Completo | — |
| [`pic18f4550_led_rd0_rd3.X`](pic18f4550_led_rd0_rd3.X/) | ✅ Completo | — |
| [`pic18f4550_pwm.X`](pic18f4550_pwm.X/) | ✅ Completo | — |
| [`pic18f4550_terminator.X`](pic18f4550_terminator.X/) | ✅ Completo | — |
| [`pic18f4550_ws2812_adafruit.X`](pic18f4550_ws2812_adafruit.X/) | ✅ Completo | Integrar DMA para mayor velocidad |
| [`pic18f45550_out_interrupt.X`](pic18f45550_out_interrupt.X/) | ✅ Completo | — |
| [`pic18f4620_LCR_oled.X`](pic18f4620_LCR_oled.X/) | 🔬 Experimental | Calibración automática, rango auto |
| [`pic18f4620_MAF.X`](pic18f4620_MAF.X/) | ✅ Completo | — |
| [`pic18f4620_detector`](pic18f4620_detector/) | ✅ Completo | Modos de discriminación, salida audio |
| [`pic18f4620_mrf24j40_tx_rx_claude.X`](pic18f4620_mrf24j40_tx_rx_claude.X/) | ✅ Completo | Encriptación, mesh networking |
| [`pic18f4620_mrf24j40_tx_rx_deepseek.X`](pic18f4620_mrf24j40_tx_rx_deepseek.X/) | ✅ Completo | Encriptación, mesh networking |
| [`pic18f4620_ssd1306_oled.X`](pic18f4620_ssd1306_oled.X/) | ✅ Completo | — |
| [`pic18f4620_usart.X`](pic18f4620_usart.X/) | ✅ Completo | — |
| [`pic18f46j50.X`](pic18f46j50.X/) | ✅ Completo | — |
| [`pic18f46j50_i2c.X`](pic18f46j50_i2c.X/) | ✅ Completo | — |
| [`pic18f46j50_i2c_wallet_freebuff.X`](pic18f46j50_i2c_wallet_freebuff.X/) | 🧪 En desarrollo | Integrar criptografía real (SHA256/ECDSA) |
| [`pic18f46j50_oled.X`](pic18f46j50_oled.X/) | ✅ Completo | — |
| [`pic18f46j50_tmp.X`](pic18f46j50_tmp.X/) | ✅ Completo | — |
| [`pic18f46j50_wallet.X`](pic18f46j50_wallet.X/) | ✅ Completo | Agregar soporte USB HID |
| [`pic18f46j50_wallet_usb.X`](pic18f46j50_wallet_usb.X/) | 🧪 En desarrollo | Unificar con wallet.X, menú OLED completo |

### 🟢 PIC32MX795 — 32-bit MIPS

| Proyecto | Estado | Próximos pasos |
|----------|--------|----------------|
| [`pic32mx795_Epaper_Display.X`](pic32mx795_Epaper_Display.X/) | ✅ Completo | — |
| [`pic32mx795_Epaper_Display_1v1.X`](pic32mx795_Epaper_Display_1v1.X/) | ✅ Completo | — |
| [`pic32mx795_Epaper_Display_1v2.X`](pic32mx795_Epaper_Display_1v2.X/) | ✅ Completo | — |
| [`pic32mx795_Epaper_Display_1v3.X`](pic32mx795_Epaper_Display_1v3.X/) | ✅ Completo | — |
| [`pic32mx795_Epaper_Display_1v4_claude.X`](pic32mx795_Epaper_Display_1v4_claude.X/) | ✅ Completo | Agregar alarma programable |
| [`pic32mx795_Epaper_Display_1v5.X`](pic32mx795_Epaper_Display_1v5.X/) | ✅ Completo | Generador de barcodes dinámico |
| [`pic32mx795_mrf24j40.X`](pic32mx795_mrf24j40.X/) | 🧪 En desarrollo | Stack Zigbee completo, encriptación |
| [`pic32mx795_st7789_version1.0.X`](pic32mx795_st7789_version1.0.X/) | ✅ Completo | — |
| [`pic32mx795_st7789_version1.1.X`](pic32mx795_st7789_version1.1.X/) | ✅ Completo | — |
| [`pic32mx795_st7789_version1.2.X`](pic32mx795_st7789_version1.2.X/) | ✅ Completo | — |
| [`pic32mx795_st7789_version1.3.X`](pic32mx795_st7789_version1.3.X/) | ✅ Completo | — |
| [`pic32mx795_st7789_version1.4.X`](pic32mx795_st7789_version1.4.X/) | ✅ Completo | Power pellets, niveles, high score |
| [`pic32mx795_st7789_version1.5.X`](pic32mx795_st7789_version1.5.X/) | ✅ Completo | Pantalla de inicio, mapa más grande |
| [`pic32mx795_test_epaper_pins.X`](pic32mx795_test_epaper_pins.X/) | 🔧 Utilidad | — |

### Leyenda

| Icono | Estado |
|-------|--------|
| ✅ Completo | Proyecto funcional, compilable y documentado |
| 🧪 En desarrollo | Funcional pero con mejoras planificadas |
| 🔬 Experimental | Prueba de concepto, requiere validación |
| 🔧 Utilidad | Herramienta auxiliar/de prueba |

---

## 🔧 Herramientas

- **IDE:** MPLAB X IDE
- **Compiladores:** XC8 (PIC18F), XC32 (PIC32MX)
- **Microcontroladores:** PIC18F (8-bit), PIC32MX795F512H (32-bit MIPS)
- **Display E-Paper:** Pervasive Displays (1.54"–4.37")
- **Display TFT:** ST7789 240×240
- **OLED:** SSD1306 128×32/128×64 (I2C)
- **RF:** MRF24J40 (IEEE 802.15.4)

## 📁 Convenciones

- `*.X/` — Proyecto MPLAB X
- `README.md` — Documentación del proyecto
- `main.c`, `test.c` — Punto de entrada principal
- `HardwareProfile.h` — Configuración de pines (PIC32)
- `config.h`, `hardware_cfg.h` — Configuración de pines (PIC18)
