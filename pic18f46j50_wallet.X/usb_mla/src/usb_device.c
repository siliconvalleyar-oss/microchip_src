/**
 * USB Device Core Stub - Complete implementation for all CDC functions
 */

#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "usb_cdc.h"

// USB Device states
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

static USB_DEVICE_STATE device_state = USB_DETACHED;

// CDC data buffers
#define CDC_RX_BUFFER_SIZE 64
#define CDC_TX_BUFFER_SIZE 64

static uint8_t rx_buffer[CDC_RX_BUFFER_SIZE];
static uint8_t tx_buffer[CDC_TX_BUFFER_SIZE];
static uint8_t rx_head = 0;
static uint8_t rx_tail = 0;
static uint8_t tx_head = 0;
static uint8_t tx_tail = 0;

// USB Device core functions
void USBDeviceInit(void) {
    device_state = USB_DETACHED;
    UCONbits.USBEN = 0;
    rx_head = rx_tail = 0;
    tx_head = tx_tail = 0;
}

void USBDeviceTasks(void) {
    static int counter = 0;
    counter++;
    if(counter > 1000) {
        counter = 0;
        if(device_state == USB_ATTACHED) {
            device_state = USB_CONFIGURED;
        }
    }
}

bool USBDeviceAttach(void) {
    device_state = USB_ATTACHED;
    UCONbits.USBEN = 1;
    return true;
}

void USBDeviceDetach(void) {
    device_state = USB_DETACHED;
    UCONbits.USBEN = 0;
}

bool USBIsDeviceSuspended(void) {
    return false;
}

bool USBIsDeviceConfigured(void) {
    return (device_state == USB_CONFIGURED);
}

// CDC specific functions
void USB_CDC_Init(void) {
    rx_head = rx_tail = 0;
    tx_head = tx_tail = 0;
    UCFG = 0x08;
    UIE = 0x00;
    UIR = 0x00;
    USBDeviceAttach();
}

void USB_CDC_Tasks(void) {
    USBDeviceTasks();
}

void USB_CDC_WriteString(const char* str) {
    if(str == NULL) return;
    
    uint8_t len = 0;
    while(str[len] != '\0' && len < CDC_TX_BUFFER_SIZE) {
        tx_buffer[(tx_head + len) % CDC_TX_BUFFER_SIZE] = str[len];
        len++;
    }
    tx_head = (tx_head + len) % CDC_TX_BUFFER_SIZE;
}

uint8_t USB_CDC_ReadData(uint8_t* buffer, uint8_t max_len) {
    if(buffer == NULL || max_len == 0) return 0;
    
    uint8_t count = 0;
    while(count < max_len && rx_head != rx_tail) {
        buffer[count] = rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1) % CDC_RX_BUFFER_SIZE;
        count++;
    }
    return count;
}

void USBCDCSendData(uint8_t* data, uint8_t len) {
    for(uint8_t i = 0; i < len && i < CDC_TX_BUFFER_SIZE; i++) {
        tx_buffer[(tx_head + i) % CDC_TX_BUFFER_SIZE] = data[i];
    }
    tx_head = (tx_head + len) % CDC_TX_BUFFER_SIZE;
}

bool USBCDCIsTransmitted(void) {
    return (tx_head == tx_tail);
}

uint8_t USBCDCGetData(uint8_t* buffer, uint8_t len) {
    return USB_CDC_ReadData(buffer, len);
}

bool USBCDCPutChar(uint8_t c) {
    uint8_t next_head = (tx_head + 1) % CDC_TX_BUFFER_SIZE;
    if(next_head != tx_tail) {
        tx_buffer[tx_head] = c;
        tx_head = next_head;
        return true;
    }
    return false;
}

uint8_t USBCDCGetChar(void) {
    if(rx_head != rx_tail) {
        uint8_t c = rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1) % CDC_RX_BUFFER_SIZE;
        return c;
    }
    return 0;
}
