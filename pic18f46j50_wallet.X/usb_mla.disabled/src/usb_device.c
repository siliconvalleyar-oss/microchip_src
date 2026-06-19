#include "usb_pic18f46j50_compat.h"
#include "../inc/usb_device.h"
#include <xc.h>

static USB_DEVICE_STATE device_state = USB_DETACHED_STATE;

void USBDeviceInit(void)
{
    // Configurar módulo USB
    UCFGbits.UPUEN = 1;     // Pull-up interno en D+
    UCFGbits.FSEN = 1;      // Full Speed
    UCFGbits.UTRDIS = 0;    // Transceiver habilitado
    
    device_state = USB_POWERED_STATE;
}

void USBDeviceTasks(void)
{
    // Procesar interrupciones USB
    if(UIEbits.URSTIE && UIRbits.URSTIF) {
        UIRbits.URSTIF = 0;
        device_state = USB_DEFAULT_STATE;
    }
    
    if(UIEbits.ACTVIE && UOTGIRbits.ACTVIF) {
        UOTGIRbits.ACTVIF = 0;
        device_state = USB_ATTACHED_STATE;
    }
}

void USBDeviceAttach(void)
{
    UCFGbits.UPUEN = 1;  // Habilitar pull-up
    device_state = USB_ATTACHED_STATE;
}

void USBDeviceDetach(void)
{
    UCFGbits.UPUEN = 0;  // Deshabilitar pull-up
    device_state = USB_DETACHED_STATE;
}

USB_DEVICE_STATE USBGetDeviceState(void)
{
    return device_state;
}

void USBEnableEndpoint(uint8_t ep, uint8_t flags)
{
    // Configurar endpoint
    if(ep == 1) {
        UEP1 = flags;
    } else if(ep == 2) {
        UEP2 = flags;
    }
}

bool USBIsTxReady(uint8_t ep)
{
    if(ep == 1) return UIRbits.TRXIF;
    if(ep == 2) return UIRbits.TRXIF;
    return false;
}

bool USBIsRxReady(uint8_t ep)
{
    if(ep == 1) return UIRbits.TRXIF;
    if(ep == 2) return UIRbits.TRXIF;
    return false;
}

bool USBPutTxByte(uint8_t ep, uint8_t data)
{
    // Implementación básica
    if(ep == 1) {
        UEP1 = data;
        return true;
    }
    if(ep == 2) {
        UEP2 = data;
        return true;
    }
    return false;
}

bool USBGetRxByte(uint8_t ep, uint8_t *data)
{
    // Implementación básica
    if(ep == 1) {
        *data = UEP1;
        return true;
    }
    if(ep == 2) {
        *data = UEP2;
        return true;
    }
    return false;
}

void USBStallEndpoint(uint8_t ep, bool stall)
{
    if(ep == 1) UEP1bits.EPSTALL = stall;
    if(ep == 2) UEP2bits.EPSTALL = stall;
}

uint8_t USBGetEPStatus(uint8_t ep)
{
    if(ep == 1) return UEP1;
    if(ep == 2) return UEP2;
    return 0;
}

// Funciones CDC simplificadas
void USBCDCInit(void) {}
void USBCheckOtherReq(void) {}
void USBCCheckOtherReq(void) {}
bool CDCIsTxReady(void) { return true; }
bool CDCIsRxReady(void) { return true; }
bool CDCGetRXByte(uint8_t *data) { return false; }
bool CDCPutTxByte(uint8_t data) { return true; }
