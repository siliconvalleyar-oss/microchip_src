
//#pragma config OSC = INTIO67
//OSCCON = 0x60;   // Configurar el oscilador interno a 4 MHz
 

#include <xc.h>
/*
//#pragma config OSC = INTIO67
#pragma config OSC = XT
#pragma config LVP = OFF
#pragma config WDT=OFF
#pragma config IESO=OFF
#pragma config PWRT=OFF
#pragma config MCLRE=ON
#pragma config XINST=OFF
#pragma config DEBUG=OFF
#pragma config FCMEN = OFF
#pragma config BOREN=OFF
#pragma config PBADEN = OFF
*/    
#define _XTAL_FREQ 4000000 

void USARTInit(void);
void USARTPutChar(char Dato);
 void UART_String(char * _string);
 void Delay1KTCYx(int);
 
void main(void) {
    //OSCCON = 0x60;
    TRISA = 0;
    TRISB = 0;
    TRISC = 0b10000000;
    TRISD = 0;
    TRISE = 0;
    PORTA=PORTB=PORTC=PORTD=PORTE= 0x00;
    USARTInit();
    __delay_ms(1000);
    UART_String("my source code\r\n");
    while (1) {
        PORTA = 0xff;
        __delay_ms(500);
        PORTA = 0x00;
        __delay_ms(500);
        USARTPutChar(0x40);
    }
}




void USARTInit(void){ 
    SPBRG = 25;//64;//9600
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 1;
    TXSTA = 0b00100100;//TXSTAbits.BRGH=1; bit->2 //TXEN bit ->5
    BAUDCONbits.BRG16 = 0;
    TXSTAbits.BRGH=1;
    RCSTAbits.SPEN = 1;
}


void USARTPutChar(char Dato)
{ // esta funcion envia caracteres por el puerto serie
	PIR1bits.TXIF = 0; 	// Borramos flag de transmision

	TXREG = Dato;		// Enviamos dato
	while(TXSTAbits.TRMT == 0); 	// esperamos a que se termine
								// la transmicion
}

 void UART_String(char * _string){
 while(*_string!=0x00)USARTPutChar(*_string++);
}


