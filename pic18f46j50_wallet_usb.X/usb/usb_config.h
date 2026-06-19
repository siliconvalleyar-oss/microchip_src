/**
 * usb_config.h - USB CDC Configuration for PIC18F46J50
 * Included by the MLA USB stack (usb.h)
 */

#ifndef USB_CONFIG_H
#define USB_CONFIG_H

// EP0 buffer size (8 bytes minimum)
#define USB_EP0_BUFF_SIZE           8

// Ping-pong buffering mode
#define USB_PING_PONG_MODE          USB_PING_PONG__FULL_PING_PONG

// Polling mode (not interrupt-driven)
#define USB_POLLING
// #define USB_INTERRUPT

// Enable CDC class
#define USB_USE_CDC

// Maximum endpoint number used (EP0 + EP1 + EP2)
#define USB_MAX_EP_NUMBER           2

// Maximum number of interfaces
#define USB_MAX_NUM_INT             2

// Number of string descriptors (excluding language descriptor at index 0)
#define USB_NUM_STRING_DESCRIPTORS  3

// CDC interface IDs (as defined in the descriptors)
#define CDC_COMM_INTF_ID            0
#define CDC_DATA_INTF_ID            1

// CDC Communication Interface endpoint
#define CDC_COMM_EP                 1
#define CDC_COMM_IN_EP_SIZE         10

// CDC Data Interface endpoints
#define CDC_DATA_EP                 2
#define CDC_DATA_IN_EP_SIZE         64
#define CDC_DATA_OUT_EP_SIZE        64

// CDC feature support
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1

// String descriptor support
#define USB_SUPPORT_DEVICE

// Status stage timeouts
#define USB_ENABLE_STATUS_STAGE_TIMEOUTS
#define USB_STATUS_STAGE_TIMEOUT    45  // ms

#endif // USB_CONFIG_H
