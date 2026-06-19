/**
 * usb_cdc.h - USB CDC Wrapper API
 * High-level functions for CDC serial communication
 */

#ifndef USB_CDC_WRAPPER_H
#define USB_CDC_WRAPPER_H

#include <xc.h>
#include <stdint.h>

// Initialize the USB CDC module
void USB_CDC_Init(void);

// Periodic task - must be called frequently in main loop
void USB_CDC_Tasks(void);

// Write a string over USB CDC (blocking until transmitted)
void USB_CDC_WriteString(const char* str);

// Read data from USB CDC (non-blocking)
// Returns number of bytes read
uint8_t USB_CDC_ReadData(uint8_t* buffer, uint8_t max_len);

// Check if device is configured (USB enumerated)
uint8_t USB_CDC_IsConfigured(void);

#endif // USB_CDC_WRAPPER_H
