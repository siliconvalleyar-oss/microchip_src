# Changelog — Microchip PIC Projects

Todas las modificaciones notables en este repositorio.

---

## [2026-06-19] — Refactorización y documentación

### Agregado
- `ST7789_common/` — Driver base reutilizable para display ST7789 (SPI4, primitivas, fuente 5×7)
- README.md general con índice de proyectos, roadmap/estado y herramientas
- README.md individuales para los 6 proyectos ST7789 (v1.0–v1.5)
- Sección 🗺️ Roadmap / Estado de proyectos en README.md

### Modificado
- **Refactor ST7789**: 6 proyectos refactorizados a base común (~2,800 líneas eliminadas)
  - v1.0, v1.1, v1.2: `main.cpp` simplificado (~600 líneas menos c/u)
  - v1.3: `HardwareProfile.h` + `Graphics.h` con wrappers inline
  - v1.4, v1.5: Pac-Man sin primitivas duplicadas, `fonts.h` eliminado
  - Todos los proyectos: `nbproject/configurations.xml` actualizado
- `.gitignore` — Agregados patrones para builds de MPLAB X (`build/`, `dist/`, `*.hex`, `*.o`, etc.)
- Tags actualizados para E-Paper y ST7789 (apuntan a commits con documentación)

### Eliminado
- `fonts.h` duplicado de v1.3, v1.4, v1.5 (ahora en `ST7789_common/`)

---

## [2026-06-18] — Documentación PIC18F y E-Paper

### Agregado
- README.md para 22 proyectos PIC18F
- README.md para 7 proyectos E-Paper PIC32MX795 (v1.0–v1.5 + test_pins)
- README.md general con índice completo de proyectos

### Modificado
- Tags `*-v1.0.0` actualizados a commits con README

---

## [2026-06-16] — Inicial

### Agregado
- Proyectos PIC18F:
  - `pic18f452_MAF.X` — Frequency Meter + Signal Regenerator
  - `pic18f4550_i2c.X` — I2C Master + LED animation
  - `pic18f4550_led_rd0_rd3.X` — LED sequence demo
  - `pic18f4550_pwm.X` — Hardware PWM with fade
  - `pic18f4550_terminator.X` — Software PWM 7-mode
  - `pic18f4550_ws2812_adafruit.X` — WS2812 RGB LED strip
  - `pic18f45550_out_interrupt.X` — EXT INT0 + ADC
  - `pic18f4620_LCR_oled.X` — LCR Meter with OLED
  - `pic18f4620_MAF.X` — Signal regenerator
  - `pic18f4620_detector` — Metal detector (full featured)
  - `pic18f4620_mrf24j40_tx_rx_claude.X` — MRF24J40 wireless (Claude)
  - `pic18f4620_mrf24j40_tx_rx_deepseek.X` — MRF24J40 wireless (DeepSeek)
  - `pic18f4620_ssd1306_oled.X` — SSD1306 OLED I2C
  - `pic18f4620_usart.X` — USART serial
  - `pic18f46j50.X` — OLED + EEPROM
  - `pic18f46j50_i2c.X` — I2C OLED with buttons
  - `pic18f46j50_i2c_wallet_freebuff.X` — Crypto wallet (Trezor-like)
  - `pic18f46j50_oled.X` — LED sequencer + OLED
  - `pic18f46j50_tmp.X` — Temperature sensor (I2C)
  - `pic18f46j50_wallet.X` — USB CDC device
  - `pic18f46j50_wallet_usb.X` — Crypto wallet + USB + OLED

- Proyectos PIC32MX795:
  - `pic32mx795_Epaper_Display.X` — Base E-Paper driver
  - `pic32mx795_Epaper_Display_1v1.X` — E-Paper v1.1
  - `pic32mx795_Epaper_Display_1v2.X` — E-Paper v1.2
  - `pic32mx795_Epaper_Display_1v3.X` — E-Paper v1.3 refinado
  - `pic32mx795_Epaper_Display_1v4_claude.X` — E-Paper digital clock
  - `pic32mx795_Epaper_Display_1v5.X` — E-Paper + Code 39 barcode
  - `pic32mx795_mrf24j40.X` — MRF24J40 wireless transceiver
  - `pic32mx795_st7789_version1.0.X` — ST7789 Space Shooter v1.0
  - `pic32mx795_st7789_version1.1.X` — ST7789 Space Shooter v1.1
  - `pic32mx795_st7789_version1.2.X` — ST7789 enemigos coloreados v1.2
  - `pic32mx795_st7789_version1.3.X` — ST7789 refactor modular v1.3
  - `pic32mx795_st7789_version1.4.X` — ST7789 Pac-Man v1.4
  - `pic32mx795_st7789_version1.5.X` — ST7789 Pac-Man mejorado v1.5
  - `pic32mx795_test_epaper_pins.X` — SPI4 pin test

- `.gitignore` inicial
- Tags `*-v1.0.0` para todos los proyectos

---

## Formato

Este changelog sigue el formato [Keep a Changelog](https://keepachangelog.com/).
