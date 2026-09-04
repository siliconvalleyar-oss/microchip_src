#ifndef USB_CDC_H
#define USB_CDC_H

#include "../hal/hal_config.h"

void USB_CDC_Init(void);
void USB_CDC_Tasks(void);
uint8_t USB_CDC_IsConnected(void);
void USB_CDC_WriteByte(uint8_t data);
void USB_CDC_WriteString(const char* str);
void USB_CDC_WriteStringPGM(const char* str);
uint8_t USB_CDC_ReadData(char* buffer, uint8_t max_len);
void USB_CDC_Printf(const char* fmt, ...);

#endif
