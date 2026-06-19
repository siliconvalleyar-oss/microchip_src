/**
 * USB CDC Stub
 */

#include "usb_cdc.h"

// All USB functions are implemented in usb_mla/src/usb_device.c
// This file exists only to satisfy the build system

void USBInitHardware(void) {
    // Configure USB pins
    UCFG = 0x08;
    UIE = 0x00;
    UIR = 0x00;
}
