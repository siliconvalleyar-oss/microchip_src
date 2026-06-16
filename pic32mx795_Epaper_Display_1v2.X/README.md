# E-Paper Driver para PIC32MX795 - Display 2.66"

## Estructura del proyecto

#
10 GND
9 ECSM gray     RB3           ->  Panel_CS      // pin3 demoboard_mx795 - an3
8 FCSM violet   RB13        ->  Flash_CS        // pin13 demoboard_mx795  - an13
7 MOSI Blue      RF5       ->  MOSI             // pin 1 otro conector  demoboard_mx795
6 MISO Green     RF4       ->  MISO             // pin 2   otro conector  demoboard_mx795
5 RST Yellow    RB2     ->                      // pin2 demoboard_mx795 an2
4 D/C Orange    RB1     ->                      // pin1 demoboard_mx795  an1
3 BUSY Red      RB0     ->                      // pin0 demoboard_mx795 an0
2 SCK Browm     RB14                            // pin14 demoboard_mx795  - an13
1 vcc Black      


 	 1  	VCC +3.3V 	Power 	Power Supply
	 2  	SPI SCK 	SPI 	SPI clock
.panelBusy 	 3  	Panel !BUSY 	Output 	Panel busy signal
.panelDC 	 4  	Panel D/C 	Input 	Panel Data/Command control
.panelReset 	 5  	Panel /RESET 	Input 	Panel reset
	 6  	SPI MISO 	SPI 	SPI MISO
	 7  	SPI MOSI 	SPI 	SPI MOSI
.flashCS 	 8  	Flash /CS 	Input 	Flash chip select
.panelCS 	 9  	Panel /CS 	Input 	Panel chip select
	 10  	Ground 	Power 	Ground
	 
	 
	 
	 
numero de  de pin para el pic de este codigo
RB0	16	PORTB, bit 0
RB1	17	PORTB, bit 1
RB2	18	PORTB, bit 2
RB3	19	PORTB, bit 3
RB13	29	PORTB, bit 13
RB14	30	PORTB, bit 14
RF4	84	PORTF, bit 4
RF5	85	PORTF, bit 5

 *       - RB0 = panelBusy (pin 16)
 *       - RB1 = panelDC   (pin 15)
 *       - RB2 = panelReset (pin 14)
 *       - RB3 = panelCS   (pin 13)
 
WIRE_GRAY_PIN    <->	Panel_CS    <->		PIN13 - gpio27
WIRE_BLUE_PIN    <->	MOSI		<->    	PIN19 - gpio10
WIRE_BROWN_PIN   <->	SCK    		<->    	PIN23 - gpio11
WIRE_GREEN_PIN   <->	MISO    	<->     PIN21 - gpio9

WIRE_VIOLET_PIN   <->	Flash_CS    <->     PIN15 - gpio22
WIRE_YELLOW_PIN   <->	RESET    	<->     PIN22 - gpio25
WIRE_ORANGE_PIN   <->	D/C    		<->    	PIN24 - gpio8 -ce0
WIRE_RED_PIN 	  <->	BUSY 		<->	    PIN26 - gpio7 -ce1


Señal	Pin	Puerto	Función
SCK	29	RB14	Clock SPI
SDO	42	RF5	Master Out Slave In
SDI	41	RF4	Master In Slave Out
BUSY	48	RD0	Estado del display
DC	49	RD1	Data/Command
RESET	50	RD2	Reset
CS	51	RD3	Chip Select

 * @brief ConfiguraciÃ³n de pines para el display
 * @note  Pines utilizados:
 *        - BUSY:   RD0 (pin 48) - entrada
 *        - DC:     RD1 (pin 49) - salida
 *        - RESET:  RD2 (pin 50) - salida
 *        - CS:     RD3 (pin 51) - salida
 * 
 * @note  Pines SPI (configurados en EPD_Driver.cpp):
 *        - SCK:    RB14 (pin 29)
 *        - SDO:    RF5  (pin 42)
 *        - SDI:    RF4  (pin 41)



Conector AN
cnx 14


pic32mx795  -> conector designacion del 0 al 16

pines PIC32  // puertos:
Pin 16	PGED1/AN0/VREF+/CVREF+/PMA6/CN2/RB0
Pin 15	PGEC1/AN1/VREF-/CVREF-/CN3/RB1
Pin 14	AN2/C2IN-/CN4/RB2
Pin 13	AN3/C2IN+/CN5/RB3

Pin 12	AN4/C1IN-/CN6/RB4
Pin 11	AN5/C1IN+/VBUSON/CN7/RB5
Pin 17	PGEC2/AN6/OCFA/RB6
Pin 18	PGED2/AN7/RB7
Pin 21	AN8/SS4/U5RX/U2CTS/C1OUT/RB8···
Pin 22	AN9/C2OUT/PMA7/RB9
Pin 23	TMS/AN10/CVREFOUT/PMA13/RB10
Pin 24	TDO/AN11/PMA12/RB11
Pin 27	TCK/AN12/PMA11/RB12
Pin 28	TDI/AN13/PMA10/RB13
Pin 29	AN14/SCK4/U5TX/U2RTS/PMALH/PMA1/RB14
Pin 30	AN15/OCFB/PMALL/PMA0/CN12/RB15



LEDs:

pin 1-> RE5 -> led 0 
pin 2-> RE6 -> led 1 
pin 3-> RE7 -> led 2 
pin 64 -> RE4 -> led 3 
pin 63 -> RE3 -> led 4  
pin 62 -> RE2 -> led 5
pin 61 -> RE1 -> led 6
pin 60 -> RE0 -> led 7

pines del 49 al 55  =>> led 8 al led 15

RD7 -> led 8
RD6 -> led 9
RD5 -> led 10
RD4 -> led 11
RD3 -> led 12
RD2 -> led 13
RD1 -> led 14
RD0 -> led 15


CN1
pin 46 -> RD11		ECRS/AEREFCLK/IC4/PMCS1/PMA14/INT4/RD11
pin 49 -> RD10		ECOL/AECRSDV/SCL1/IC3/PMCS2/PMA15/INT3/RD10
pin 50 -> RD9		AERXD0/ETXD2/SS3/U4RX/U1CTS/SDA1/IC2/INT2/RD9
pin 42 -> RF5		AC1RX/SCL5/SDO4/U2TX/PMA8/CN18/RF5
pin 41 -> RF4		AC1TX/SDA5/SDI4/U2RX/PMA9/CN17/RF4
pin 37 -> RF3		USBID/RF3

pin 59 -> RF1
pin 58 -> RF0


pin 16 ->  AN0 
pin 15 ->  AN1
gnd
pin 14 ->  AN2
pin 13 ->  AN3
gnd
pin 12 ->  AN4
pin 11 ->  AN5
gnd
pin 17 ->  AN6
pin 18 ->  AN7
gnd
pin 21 ->  AN8
pin 22 ->  AN9
gnd
pin 23 ->  AN10
pin 24 ->  AN11
gnd
pin 27 ->  AN12
pin 28 ->  AN13
gnd
pin 29 ->  AN14
pin 30 ->  AN15
gnd






# Proyecto E-Paper para PIC32MX795

## Información del Proyecto

| Campo | Valor |
|-------|-------|
| **Autor** | Leo |
| **Fecha** | 2026-04-03 |
| **Pantalla** | 213" (212x104) |
| **Frame Buffer** | 2756 bytes |
| **Compilador** | XC32 v5.00 |
| **Target** | PIC32MX795F512L |

## Estructura del Proyecto

```
MicrochipEpaperPIC32.X/
├── header/
│   ├── EPD_Configuration.h    # Configuración de pantallas y pines
│   └── EPD_Driver.h           # Driver principal
├── source/
│   ├── EPD_Driver.cpp         # Implementación del driver
│   └── main.cpp               # Ejemplo de uso
├── Makefile                   # Makefile para compilación
└── README.md                  # Este archivo
```

## Pines Utilizados

### SPI (configurados en EPD_Driver.cpp)
| Señal | Pin | Puerto |
|-------|-----|--------|
| **SCK** | RB14 (pin 29) | Puerto B |
| **SDO** | RF5 (pin 42) | Puerto F |
| **SDI** | RF4 (pin 41) | Puerto F |

### Control del Display (configurables en main.cpp)
| Señal | Pin | Puerto | Dirección |
|-------|-----|--------|-----------|
| **BUSY** | RD0 (pin 48) | Puerto D | Entrada |
| **DC** | RD1 (pin 49) | Puerto D | Salida |
| **RESET** | RD2 (pin 50) | Puerto D | Salida |
| **CS** | RD3 (pin 51) | Puerto D | Salida |

### LEDs de Depuración
| LED | Pin | Puerto |
|-----|-----|--------|
| LED0 | RE0 (pin 64) | Puerto E |
| LED1 | RE1 (pin 65) | Puerto E |
| LED2 | RE2 (pin 66) | Puerto E |

## Compilación

### Compilar el proyecto:
```bash
cd MicrochipEpaperPIC32.X
make clean
make
```

### Archivos generados:
- `dist/MicrochipEpaperPIC32.elf` - Archivo ejecutable
- `dist/MicrochipEpaperPIC32.hex` - Archivo para flashear

## Programación

Usa MPLAB X IPE o PicKit3 para flashear el archivo `.hex` en el PIC32MX795.

## Secuencia de Inicialización del Display

1. Configuración de pines GPIO
2. Inicialización de SPI (8 MHz, Mode 0)
3. Hard reset del COG
4. Soft reset del COG
5. Configuración de temperatura (25°C)
6. Carga de imagen en frame buffer
7. Encendido de DC/DC
8. Refresco de pantalla

## Notas Importantes

- **No utiliza plib.h** - Todo el código usa registros nativos
- **Delays precisos** - Usa Core Timer del PIC32
- **SPI Mode 0** - CKP=0, CKE=0 (estándar para E-Paper)
- **Configuración de fuses** - Cristal externo de 8 MHz

## Solución de Problemas

### El display no muestra nada:
1. Verificar conexiones de pines
2. Verificar alimentación (3.3V)
3. Verificar que el cristal de 8 MHz esté funcionando

### Error de compilación:
1. Verificar que XC32 v5.00 esté instalado
2. Verificar las rutas en el Makefile

## Licencia

Código de ejemplo - Libre para uso educativo y comercial.
