#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "oled.h"
#include "config.h"

#define OLED_ADDR       0x78
#define OLED_WIDTH      128
#define OLED_HEIGHT     32
#define OLED_PAGES      4

#define OLED_CMD_OFF             0xAE
#define OLED_CMD_ON              0xAF
#define OLED_CMD_SET_DISP_CLK    0xD5
#define OLED_CMD_SET_MULTIPLEX   0xA8
#define OLED_CMD_SET_OFFSET      0xD3
#define OLED_CMD_SET_START_LINE  0x40
#define OLED_CMD_CHARGE_PUMP     0x8D
#define OLED_CMD_MEM_MODE        0x20
#define OLED_CMD_SEG_REMAP       0xA1
#define OLED_CMD_COM_SCAN_DIR    0xC8
#define OLED_CMD_SET_COM_PINS    0xDA
#define OLED_CMD_SET_CONTRAST    0x81
#define OLED_CMD_SET_PRECHARGE   0xD9
#define OLED_CMD_SET_VCOM_DETECT 0xDB
#define OLED_CMD_DISPLAY_RESUME  0xA4
#define OLED_CMD_NORMAL_DISP     0xA6
#define OLED_CMD_DEACT_SCROLL    0x2E

#define SDA_TRIS        TRISBbits.TRISB0
#define SCL_TRIS        TRISBbits.TRISB1
#define SDA_LAT         LATBbits.LATB0
#define SCL_LAT         LATBbits.LATB1
#define SDA_PORT        PORTBbits.RB0

static void i2c_delay(void) {
    __delay_us(5);
}

static void i2c_start(void) {
    SDA_LAT = 1;
    SCL_LAT = 1;
    i2c_delay();
    SDA_LAT = 0;
    i2c_delay();
    SCL_LAT = 0;
    i2c_delay();
}

static void i2c_stop(void) {
    SDA_LAT = 0;
    SCL_LAT = 1;
    i2c_delay();
    SDA_LAT = 1;
    i2c_delay();
}

static uint8_t i2c_write_byte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        SDA_LAT = (data & 0x80) ? 1 : 0;
        data <<= 1;
        i2c_delay();
        SCL_LAT = 1;
        i2c_delay();
        SCL_LAT = 0;
        i2c_delay();
    }
    SDA_TRIS = 1;
    SCL_LAT = 1;
    i2c_delay();
    uint8_t ack = SDA_PORT;
    SCL_LAT = 0;
    SDA_TRIS = 0;
    i2c_delay();
    return ack;
}

static uint8_t oled_ok = 0;
static uint8_t buffer[OLED_WIDTH * OLED_PAGES];

static void oled_write_cmd(uint8_t cmd) {
    if (!oled_ok) return;
    i2c_start();
    if (i2c_write_byte(OLED_ADDR) != 0) { oled_ok = 0; i2c_stop(); return; }
    if (i2c_write_byte(0x00) != 0)      { oled_ok = 0; i2c_stop(); return; }
    if (i2c_write_byte(cmd) != 0)       { oled_ok = 0; i2c_stop(); return; }
    i2c_stop();
}

static void oled_write_data(uint8_t data) {
    if (!oled_ok) return;
    i2c_start();
    if (i2c_write_byte(OLED_ADDR) != 0) { oled_ok = 0; i2c_stop(); return; }
    if (i2c_write_byte(0x40) != 0)      { oled_ok = 0; i2c_stop(); return; }
    if (i2c_write_byte(data) != 0)      { oled_ok = 0; i2c_stop(); return; }
    i2c_stop();
}

bool OLED_Init(void) {
    SDA_TRIS = 0;
    SCL_TRIS = 0;
    SDA_LAT = 1;
    SCL_LAT = 1;
    __delay_ms(100);

    i2c_start();
    if (i2c_write_byte(OLED_ADDR) == 0) {
        oled_ok = 1;
    } else {
        oled_ok = 0;
        i2c_stop();
        return false;
    }
    i2c_stop();

    oled_write_cmd(OLED_CMD_OFF);
    oled_write_cmd(OLED_CMD_SET_DISP_CLK);   oled_write_cmd(0x80);
    oled_write_cmd(OLED_CMD_SET_MULTIPLEX);  oled_write_cmd(0x1F);
    oled_write_cmd(OLED_CMD_SET_OFFSET);     oled_write_cmd(0x00);
    oled_write_cmd(OLED_CMD_SET_START_LINE);
    oled_write_cmd(OLED_CMD_CHARGE_PUMP);    oled_write_cmd(0x14);
    oled_write_cmd(OLED_CMD_MEM_MODE);       oled_write_cmd(0x00);
    oled_write_cmd(OLED_CMD_SEG_REMAP);
    oled_write_cmd(OLED_CMD_COM_SCAN_DIR);
    oled_write_cmd(OLED_CMD_SET_COM_PINS);   oled_write_cmd(0x02);
    oled_write_cmd(OLED_CMD_SET_CONTRAST);   oled_write_cmd(0xCF);
    oled_write_cmd(OLED_CMD_SET_PRECHARGE);  oled_write_cmd(0xF1);
    oled_write_cmd(OLED_CMD_SET_VCOM_DETECT);oled_write_cmd(0x40);
    oled_write_cmd(OLED_CMD_DISPLAY_RESUME);
    oled_write_cmd(OLED_CMD_NORMAL_DISP);
    oled_write_cmd(OLED_CMD_DEACT_SCROLL);

    OLED_Clear();
    OLED_Update();
    oled_write_cmd(OLED_CMD_ON);
    return true;
}

void OLED_Clear(void) {
    memset(buffer, 0, sizeof(buffer));
}

void OLED_Update(void) {
    if (!oled_ok) return;
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        oled_write_cmd(0xB0 + page);
        oled_write_cmd(0x00);
        oled_write_cmd(0x10);
        i2c_start();
        if (i2c_write_byte(OLED_ADDR) != 0) { oled_ok = 0; i2c_stop(); return; }
        if (i2c_write_byte(0x40) != 0)      { oled_ok = 0; i2c_stop(); return; }
        for (uint8_t col = 0; col < OLED_WIDTH; col++) {
            if (i2c_write_byte(buffer[page * OLED_WIDTH + col]) != 0) {
                oled_ok = 0;
                i2c_stop();
                return;
            }
        }
        i2c_stop();
    }
}

static const uint8_t font[][5] = {
    {0x00,0x00,0x00,0x00,0x00},
    {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00}, {0x42,0x61,0x51,0x49,0x46},
    {0x21,0x41,0x45,0x4B,0x31}, {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03}, {0x36,0x49,0x49,0x49,0x36},
    {0x06,0x49,0x49,0x29,0x1E}, {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36},
    {0x3E,0x41,0x41,0x41,0x22}, {0x7F,0x41,0x41,0x22,0x1C}, {0x7F,0x49,0x49,0x49,0x41},
    {0x7F,0x09,0x09,0x09,0x01}, {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F},
    {0x00,0x41,0x7F,0x41,0x00}, {0x20,0x40,0x41,0x3F,0x01}, {0x7F,0x08,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40}, {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F},
    {0x3E,0x41,0x41,0x41,0x3E}, {0x7F,0x09,0x09,0x09,0x06}, {0x3E,0x41,0x51,0x21,0x5E},
    {0x7F,0x09,0x19,0x29,0x46}, {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01},
    {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F}, {0x7F,0x20,0x18,0x20,0x7F},
    {0x63,0x14,0x08,0x14,0x63}, {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
};

void OLED_DrawChar(uint8_t x, uint8_t y, char c, uint8_t size, bool color) {
    if (!oled_ok) return;
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    uint8_t idx = 0;
    if (c >= '0' && c <= '9')      idx = c - '0' + 1;
    else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 11;
    else if (c == ' ')             idx = 0;
    else return;
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t pattern = font[idx][col];
        for (uint8_t row = 0; row < 7; row++) {
            if (pattern & (1 << row)) {
                uint8_t px = x + col;
                uint8_t py = y + row;
                if (px < OLED_WIDTH && py < OLED_HEIGHT) {
                    uint8_t page = py / 8;
                    uint8_t bit = py % 8;
                    if (color)
                        buffer[page * OLED_WIDTH + px] |= (1 << bit);
                    else
                        buffer[page * OLED_WIDTH + px] &= ~(1 << bit);
                }
            }
        }
    }
}

void OLED_DrawString(uint8_t x, uint8_t y, const char* str, uint8_t size, bool color) {
    uint8_t current_x = x;
    while (*str && current_x < OLED_WIDTH) {
        OLED_DrawChar(current_x, y, *str, size, color);
        current_x += 6;
        str++;
    }
}

void OLED_DrawDec32(uint32_t val, uint8_t x, uint8_t y, uint8_t size) {
    char buf[12];
    uint8_t i = 10;
    buf[11] = 0;
    do {
        buf[i--] = '0' + (val % 10);
        val /= 10;
    } while (val && i);
    OLED_DrawString(x, y, &buf[i+1], size, true);
}
