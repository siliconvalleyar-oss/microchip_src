/**
 * HardwareProfile.h - Hardware configuration for PIC18F46J50 USB CDC
 * Included by the MLA USB stack
 */

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include <xc.h>

// Clock frequency (used by delay macros in MLA stack)
#define _XTAL_FREQ  48000000     // 48MHz (PLL enabled for USB)

// USB bus sense - not using dedicated VBUS sense pin
// The stack will assume VBUS is present
//#define USE_USB_BUS_SENSE_IO
//#define USB_BUS_SENSE PORTBbits.RB2  // Example: if using RB2 for VBUS sense

// Self-powered device (not bus-powered)
#define self_power  1

// USB configuration options for SetConfigurationOptions() in usb_hal_pic18.h
#define USB_PULLUP_OPTION       USB_PULLUP_ENABLE
#define USB_TRANSCEIVER_OPTION  USB_INTERNAL_TRANSCEIVER
#define USB_SPEED_OPTION        USB_FULL_SPEED

#endif // HARDWARE_PROFILE_H
