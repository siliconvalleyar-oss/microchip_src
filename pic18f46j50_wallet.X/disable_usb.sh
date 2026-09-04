#!/bin/bash

echo "=== Temporarily Disabling USB for Build ==="

# 1. Comment out USB includes in main.c
if [ -f main.c ]; then
    cp main.c main.c.bkp
    sed -i 's|#include "usb_cdc.h"|// #include "usb_cdc.h"|g' main.c
    sed -i 's|USBDeviceInit()|// USBDeviceInit()|g' main.c
    sed -i 's|USBDeviceTasks()|// USBDeviceTasks()|g' main.c
    sed -i 's|USBDeviceAttach()|// USBDeviceAttach()|g' main.c
    echo "✓ Disabled USB in main.c"
fi

# 2. Create stub usb_cdc.c if needed (to satisfy linker)
cat > usb_cdc.c.stub << 'EOF'
// Stub USB CDC file - USB functionality disabled
#include <xc.h>

void USBDeviceInit(void) {
    // USB disabled
}

void USBDeviceTasks(void) {
    // USB disabled
}

bool USBDeviceAttach(void) {
    return false;
}

void USBDeviceDetach(void) {
    // USB disabled
}

void USBCDCSendData(uint8_t* data, uint8_t len) {
    // USB disabled
}

bool USBCDCIsTransmitted(void) {
    return true;
}

uint8_t USBCDCGetData(uint8_t* buffer, uint8_t len) {
    return 0;
}
