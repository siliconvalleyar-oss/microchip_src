/**
 * usb_cdc.c - USB CDC Wrapper Implementation
 * Uses the MLA USB stack for CDC serial communication on PIC18F46J50
 */

#include <xc.h>
#include <string.h>
#include "usb_cdc.h"

// Include the MLA USB stack
#include "USB/usb.h"
#include "USB/usb_function_cdc.h"

// USB state tracking
static uint8_t usb_configured = 0;
static uint8_t hello_sent = 0;

//=============================================================
// MLA USB Stack Callback Functions
// Required by the MLA USB device stack
//=============================================================

// Called when USB enters suspend mode
void USBCBSuspend(void)
{
    // Optional: put MCU to sleep, turn off LEDs, etc.
}

// Called when USB wakes from suspend
void USBCBWakeFromSuspend(void)
{
    // Optional: wake up peripherals
}

// Called on Start Of Frame interrupt
void USBCB_SOF_Handler(void)
{
    // Optional: used for timing
}

// Called on USB error
void USBCBErrorHandler(void)
{
    // Optional: handle USB errors
}

// Called to check non-standard USB requests
void USBCBCheckOtherReq(void)
{
    USBCheckCDCRequest();
}

// Called when EP0 data is received
void USBCBEP0DataReceived(void)
{
    // Optional: handle EP0 data
}

// Called on SET_DESCRIPTOR request
void USBCBStdSetDscHandler(void)
{
    // Optional: handle set descriptor
}

// Called to initialize endpoints when USB is configured
void USBCBInitEP(void)
{
    CDCInitEP();
}

// Called to send remote wakeup
void USBCBSendResume(void)
{
    static uint16_t delay_count;

    if(!USBIsDeviceSuspended()) return;

    // Check if remote wakeup is armed by host
    if(RemoteWakeup)
    {
        // Drive resume signaling (K-state) for 10ms
        UCONbits.RESUME = 1;
        delay_count = 0U;
        while(delay_count < 10U)
        {
            delay_count++;
        }
        UCONbits.RESUME = 0;
    }
}

//=============================================================
// USB Event Handler
// Called by the MLA stack on USB state changes
// Must match the prototype in usb_device.c:
//   BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size);
//=============================================================
BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size)
{
    // event values come from USB_DEVICE_STACK_EVENTS enum (usb_device.h)
    // and USB_EVENT enum (usb_common.h)
    switch(event)
    {
        case EVENT_CONFIGURED:
            USBCBInitEP();
            usb_configured = 1;
            break;

        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;

        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;

        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;

        case EVENT_SUSPEND:
            USBCBSuspend();
            usb_configured = 0;
            break;

        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;

        case EVENT_TRANSFER:
            break;

        default:
            break;
    }
    return TRUE;
}

//=============================================================
// Public API
//=============================================================

void USB_CDC_Init(void)
{
    usb_configured = 0;
    hello_sent = 0;

    // Initialize USB module registers
    UCFG = 0x08;        // USB pull-up on, internal transceiver
    UIE  = 0x00;        // No interrupts
    UIR  = 0x00;        // Clear any pending flags

    // Initialize USB device stack
    USBDeviceInit();

    // Attach to USB bus
    USBDeviceAttach();
}

void USB_CDC_Tasks(void)
{
    // Process USB device state machine (enumeration, etc.)
    USBDeviceTasks();

    // Process CDC transmission state machine
    CDCTxService();

    // Send "Hola mundo" once when USB is configured
    if(usb_configured && !hello_sent)
    {
        hello_sent = 1;
        putrsUSBUSART((const ROM char*)"Hola mundo\r\n");
    }
}

void USB_CDC_WriteString(const char* str)
{
    if(!usb_configured || str == NULL) return;

    // Wait for previous transfer to complete
    while(!USBUSARTIsTxTrfReady())
    {
        CDCTxService();
        USBDeviceTasks();
    }

    // Send the string
    putsUSBUSART((char*)str);
}

uint8_t USB_CDC_ReadData(uint8_t* buffer, uint8_t max_len)
{
    if(!usb_configured || buffer == NULL || max_len == 0) return 0;

    uint8_t len = 0;
    uint8_t c;

    while(len < max_len && getsUSBUSART((char*)&c, 1))
    {
        buffer[len++] = c;
    }

    return len;
}

uint8_t USB_CDC_IsConfigured(void)
{
    return usb_configured;
}
