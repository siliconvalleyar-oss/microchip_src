#ifndef USB_DEVICE_CONFIG_H
#define USB_DEVICE_CONFIG_H

#define USB_USE_CDC 1
#define USB_NUM_EP 4
#define USB_MAX_NUM_DEVICES 1
#define USB_EP0_BUFF_SIZE 8
#define USB_CDC_TX_BUFF_SIZE 64
#define USB_CDC_RX_BUFF_SIZE 64
#define USB_PING_PONG_MODE 0
#define USB_SUPPORT_DEVICE 1
#define USB_ENABLE_INTERRUPT 1
#define USB_POLLING 0
#define USB_CDC_DEVICE 1

// Endpoint addresses
#define USB_CDC_EP_IN 1
#define USB_CDC_EP_OUT 2

#endif
