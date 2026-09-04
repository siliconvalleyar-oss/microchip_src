#ifndef USB_CDC_H
#define USB_CDC_H

// Include necessary headers
#include <xc.h>
#include <stdint.h>

// Define bool if not available (for older compilers)
#ifndef __bool_true_false_are_defined
typedef uint8_t bool;
#define true 1
#define false 0
#endif

// CDC Function prototypes - matching what main.c expects
void USB_CDC_Init(void);
void USB_CDC_Tasks(void);
void USB_CDC_WriteString(const char* str);
uint8_t USB_CDC_ReadData(uint8_t* buffer, uint8_t max_len);

// Legacy function prototypes
void USBDeviceInit(void);
void USBDeviceTasks(void);
bool USBDeviceAttach(void);
void USBDeviceDetach(void);
void USBCDCSendData(uint8_t* data, uint8_t len);
bool USBCDCIsTransmitted(void);
uint8_t USBCDCGetData(uint8_t* buffer, uint8_t len);
bool USBCDCPutChar(uint8_t c);
uint8_t USBCDCGetChar(void);

#endif
