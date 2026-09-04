#ifndef USB_DEVICE_H
#define USB_DEVICE_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

// USB device states
typedef enum {
    USB_DETACHED = 0,
    USB_ATTACHED,
    USB_POWERED,
    USB_DEFAULT,
    USB_ADDRESS_PENDING,
    USB_ADDRESSED,
    USB_CONFIGURED,
    USB_SUSPENDED
} USB_DEVICE_STATE;

// USB device initialization
void USBDeviceInit(void);
void USBDeviceTasks(void);
bool USBDeviceAttach(void);
void USBDeviceDetach(void);
bool USBIsDeviceSuspended(void);
bool USBIsDeviceConfigured(void);

// CDC specific functions
void USBCDCSendData(uint8_t* data, uint8_t len);
bool USBCDCIsTransmitted(void);
uint8_t USBCDCGetData(uint8_t* buffer, uint8_t len);
bool USBCDCPutChar(uint8_t c);
uint8_t USBCDCGetChar(void);

#endif
