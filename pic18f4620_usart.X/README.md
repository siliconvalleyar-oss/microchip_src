# PIC18F4620 - USART Serial Communication

**Descripción:** Comunicación serie USART asíncrona con PIC18F4620. Envía un mensaje de texto y luego transmite el carácter '@' repetidamente mientras parpadea LEDs en PORTA.

## Características
- USART a 9600 baud (SPBRG=25, BRGH=1)
- TX en RC6, RX en RC7
- Envía "my source code\r\n" al inicio
- Parpadeo de LEDs en PORTA
- Fosc = 4MHz

## Periféricos Utilizados
- USART (EUSART)
- GPIO (LEDs)

## Pines
| Pin | Función |
|-----|---------|
| RC6 | TX (salida) |
| RC7 | RX (entrada) |
