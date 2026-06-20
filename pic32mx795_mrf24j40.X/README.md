# pic32mx795_mrf24j40

Transceptor MRF24J40 para PIC32MX795 con soporte ZigBee, OLED SSD1306, GPIO, I2C, UART, SPI y cifrado.

## Microcontrolador

- **PIC32MX795**

## Estructura del proyecto

```
pic32mx795_mrf24j40.X/
├── src/           # Codigo fuente (.cpp)
├── inc/           # Archivos de cabecera (.hpp)
├── examples/      # Ejemplo de uso
├── .gitignore
├── README.md
└── CHANGELOG.md
```

## Modulos

| Modulo | Archivos | Descripcion |
|--------|----------|-------------|
| MRF24J40 | mrf24j40.cpp/.hpp | Driver del transceptor |
| Protocolo | protocol.cpp/.hpp | Protocolo de comunicacion |
| SPI | spi4.cpp/.hpp | Comunicacion SPI4 |
| I2C | i2c.cpp/.hpp | Comunicacion I2C |
| UART | uart.cpp/.hpp | Comunicacion serie |
| GPIO | gpio.cpp/.hpp | Control de pines |
| OLED | ssd1306.cpp/.hpp | Driver OLED SSD1306 |
| Cifrado | crypto.cpp/.hpp | Funciones criptograficas |

## Compilacion

Abrir en MPLAB X IDE y compilar con XC32.
