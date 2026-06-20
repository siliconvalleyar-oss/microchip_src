#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 20000000

#pragma config FOSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF

#define WS LATBbits.LATB0
#define LED LATDbits.LATD0

#define NUM_LEDS 16
void ws2812_all_blue(uint8_t intensity);

void wheel(uint8_t pos,uint8_t *r,uint8_t *g,uint8_t *b);

void ws2812_bit1()
{
    WS=1;
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    WS=0;
    asm("nop"); asm("nop");
}

void ws2812_bit0()
{
    WS=1;
    asm("nop");
    WS=0;
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
}

void ws2812_send_byte(uint8_t byte)
{
    for(uint8_t i=0;i<8;i++)
    {
        if(byte & 0x80)
            ws2812_bit1();
        else
            ws2812_bit0();

        byte <<= 1;
    }
}

void ws2812_send_color(uint8_t r,uint8_t g,uint8_t b)
{
    ws2812_send_byte(g);
    ws2812_send_byte(r);
    ws2812_send_byte(b);
}

void ws2812_reset()
{
    WS = 0;
    __delay_us(80);
}

void wheel(uint8_t pos,uint8_t *r,uint8_t *g,uint8_t *b)
{
    if(pos < 85)
    {
        *r = pos * 3;
        *g = 255 - pos * 3;
        *b = 0;
    }
    else if(pos < 170)
    {
        pos -= 85;
        *r = 255 - pos * 3;
        *g = 0;
        *b = pos * 3;
    }
    else
    {
        pos -= 170;
        *r = 0;
        *g = pos * 3;
        *b = 255 - pos * 3;
    }
}




void main()
{
    ADCON1 = 0x0F;
    CMCON  = 0x07;

    TRISB = 0x00;
    LATB  = 0;

    TRISD = 0x00;
    LATD  = 0;

    uint8_t r,g,b;
    uint8_t color=0;
    
    
    while(1){
        ws2812_all_blue(90);
    }
/*
    while(1)
    {
        LED = !LED;

        for(uint8_t i=0;i<NUM_LEDS;i++)
        {
            wheel(color+i*8,&r,&g,&b);
            ws2812_send_color(r,g,b);
        }

        ws2812_reset();

        color++;

        __delay_ms(40);
    }*/
}


void ws2812_all_blue(uint8_t intensity)
{
    for(uint8_t i=0;i<NUM_LEDS;i++)
    {
        ws2812_send_color(0,intensity,intensity);
    }

    ws2812_reset();
}
