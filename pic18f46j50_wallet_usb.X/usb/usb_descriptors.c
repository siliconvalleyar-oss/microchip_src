/**
 * usb_descriptors.c - USB CDC Descriptors for PIC18F46J50
 * Adapted from Microchip CDC Basic Demo
 *
 * Defines all USB descriptors using the Microchip MLA types.
 */

#include <xc.h>
#include <stdint.h>
#include "GenericTypeDefs.h"
#include "usb_config.h"
#include "USB/usb_ch9.h"
#include "USB/usb_function_cdc.h"

// ============================================================
// Vendor & Product IDs
// ============================================================
#define USB_VID   0x04D8   // Microchip Technology Inc.
#define USB_PID   0x000A   // CDC RS-232 Emulation Demo
#define USB_BCD   0x0200   // USB 2.00

#define CFG01     1

// String descriptor indices
#define STRID_LANG          0
#define STRID_MANUFACTURER  1
#define STRID_PRODUCT       2
#define STRID_SERIAL        3

// ============================================================
// Device Descriptor
// ============================================================
ROM USB_DEVICE_DESCRIPTOR device_dsc =
{
    .bLength            = sizeof(USB_DEVICE_DESCRIPTOR),
    .bDescriptorType    = USB_DESCRIPTOR_DEVICE,
    .bcdUSB             = USB_BCD,
    .bDeviceClass       = 0x02,              // CDC class
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = USB_EP0_BUFF_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0001,
    .iManufacturer      = STRID_MANUFACTURER,
    .iProduct           = STRID_PRODUCT,
    .iSerialNumber      = STRID_SERIAL,
    .bNumConfigurations = 1
};

// ============================================================
// Configuration Descriptor (CDC)
// Combined: config + interface0 (Comm) + functional descriptors
//           + EP1 (notification) + interface1 (Data) + EP2 (bulk)
// ============================================================
ROM struct {
    USB_CONFIGURATION_DESCRIPTOR config;
    USB_INTERFACE_DESCRIPTOR    interface_comm;
    USB_CDC_HEADER_FN_DSC      header_fn;
    USB_CDC_ACM_FN_DSC         acm_fn;
    USB_CDC_UNION_FN_DSC       union_fn;
    USB_CDC_CALL_MGT_FN_DSC    call_mgmt_fn;
    USB_ENDPOINT_DESCRIPTOR    ep1_notification;
    USB_INTERFACE_DESCRIPTOR   interface_data;
    USB_ENDPOINT_DESCRIPTOR    ep2_out;
    USB_ENDPOINT_DESCRIPTOR    ep2_in;
} cfgDesc1 = {
    // ---- Configuration ----
    .config = {
        .bLength             = sizeof(USB_CONFIGURATION_DESCRIPTOR),
        .bDescriptorType     = USB_DESCRIPTOR_CONFIGURATION,
        .wTotalLength        = sizeof(cfgDesc1),
        .bNumInterfaces      = 0x02,
        .bConfigurationValue = CFG01,
        .iConfiguration      = 0x00,
        .bmAttributes        = 0x80,   // Bus powered, no remote wakeup
        .bMaxPower           = 50      // 100 mA
    },

    // ---- Interface 0: Communication Interface ----
    .interface_comm = {
        .bLength             = sizeof(USB_INTERFACE_DESCRIPTOR),
        .bDescriptorType     = USB_DESCRIPTOR_INTERFACE,
        .bInterfaceNumber    = 0x00,
        .bAlternateSetting   = 0x00,
        .bNumEndpoints       = 0x01,
        .bInterfaceClass     = COMM_INTF,
        .bInterfaceSubClass  = ABSTRACT_CONTROL_MODEL,
        .bInterfaceProtocol  = V25TER,
        .iInterface          = 0x00
    },

    // ---- CDC Header Functional Descriptor ----
    .header_fn = {
        .bFNLength           = sizeof(USB_CDC_HEADER_FN_DSC),
        .bDscType            = CS_INTERFACE,
        .bDscSubType         = DSC_FN_HEADER,
        .bcdCDC              = 0x0110
    },

    // ---- CDC ACM Functional Descriptor ----
    .acm_fn = {
        .bFNLength           = sizeof(USB_CDC_ACM_FN_DSC),
        .bDscType            = CS_INTERFACE,
        .bDscSubType         = DSC_FN_ACM,
        .bmCapabilities      = USB_CDC_ACM_FN_DSC_VAL
    },

    // ---- CDC Union Functional Descriptor ----
    .union_fn = {
        .bFNLength           = sizeof(USB_CDC_UNION_FN_DSC),
        .bDscType            = CS_INTERFACE,
        .bDscSubType         = DSC_FN_UNION,
        .bMasterIntf         = 0x00,
        .bSaveIntf0          = 0x01
    },

    // ---- CDC Call Management Functional Descriptor ----
    .call_mgmt_fn = {
        .bFNLength           = sizeof(USB_CDC_CALL_MGT_FN_DSC),
        .bDscType            = CS_INTERFACE,
        .bDscSubType         = DSC_FN_CALL_MGT,
        .bmCapabilities      = 0x00,
        .bDataInterface      = 0x01
    },

    // ---- Endpoint 1: Notification (IN, Interrupt) ----
    .ep1_notification = {
        .bLength             = sizeof(USB_ENDPOINT_DESCRIPTOR),
        .bDescriptorType     = USB_DESCRIPTOR_ENDPOINT,
        .bEndpointAddress    = _EP01_IN,
        .bmAttributes        = _INTERRUPT,
        .wMaxPacketSize      = CDC_COMM_IN_EP_SIZE,
        .bInterval           = 0x02
    },

    // ---- Interface 1: Data Interface ----
    .interface_data = {
        .bLength             = sizeof(USB_INTERFACE_DESCRIPTOR),
        .bDescriptorType     = USB_DESCRIPTOR_INTERFACE,
        .bInterfaceNumber    = 0x01,
        .bAlternateSetting   = 0x00,
        .bNumEndpoints       = 0x02,
        .bInterfaceClass     = DATA_INTF,
        .bInterfaceSubClass  = 0x00,
        .bInterfaceProtocol  = NO_PROTOCOL,
        .iInterface          = 0x00
    },

    // ---- Endpoint 2 OUT (Host -> Device, Bulk) ----
    .ep2_out = {
        .bLength             = sizeof(USB_ENDPOINT_DESCRIPTOR),
        .bDescriptorType     = USB_DESCRIPTOR_ENDPOINT,
        .bEndpointAddress    = _EP02_OUT,
        .bmAttributes        = _BULK,
        .wMaxPacketSize      = CDC_DATA_OUT_EP_SIZE,
        .bInterval           = 0x00
    },

    // ---- Endpoint 2 IN (Device -> Host, Bulk) ----
    .ep2_in = {
        .bLength             = sizeof(USB_ENDPOINT_DESCRIPTOR),
        .bDescriptorType     = USB_DESCRIPTOR_ENDPOINT,
        .bEndpointAddress    = _EP02_IN,
        .bmAttributes        = _BULK,
        .wMaxPacketSize      = CDC_DATA_IN_EP_SIZE,
        .bInterval           = 0x00
    }
};

// ============================================================
// Configuration descriptor pointer table
// ============================================================
ROM void* const USB_CD_Ptr[] =
{
    (ROM void*)&cfgDesc1
};

// ============================================================
// String Descriptors
// ============================================================

// Language descriptor (US English)
ROM struct {
    BYTE     bLength;
    BYTE     bDescriptorType;
    WORD     string[1];
} strd_lang = {
    .bLength         = 4,
    .bDescriptorType = USB_DESCRIPTOR_STRING,
    .string          = {0x0409}   // US English
};

// Manufacturer string
ROM struct {
    BYTE     bLength;
    BYTE     bDescriptorType;
    WORD     string[24];
} strd_manufacturer = {
    .bLength         = sizeof(strd_manufacturer),
    .bDescriptorType = USB_DESCRIPTOR_STRING,
    .string          = {
        'C','r','y','p','t','o',' ','W','a','l','l','e','t',
        ' ','D','e','v','i','c','e','s',' ',' ',' '
    }
};

// Product string
ROM struct {
    BYTE     bLength;
    BYTE     bDescriptorType;
    WORD     string[17];
} strd_product = {
    .bLength         = sizeof(strd_product),
    .bDescriptorType = USB_DESCRIPTOR_STRING,
    .string          = {
        'U','S','B',' ','C','D','C',' ','W','a','l','l','e','t',' ',' ',' '
    }
};

// Serial number string
ROM struct {
    BYTE     bLength;
    BYTE     bDescriptorType;
    WORD     string[9];
} strd_serial = {
    .bLength         = sizeof(strd_serial),
    .bDescriptorType = USB_DESCRIPTOR_STRING,
    .string          = {'0','0','0','0','0','0','0','1',' '}
};

// ============================================================
// String descriptor pointer table
// ============================================================
ROM void* const USB_SD_Ptr[] =
{
    (ROM void*)&strd_lang,
    (ROM void*)&strd_manufacturer,
    (ROM void*)&strd_product,
    (ROM void*)&strd_serial
};
