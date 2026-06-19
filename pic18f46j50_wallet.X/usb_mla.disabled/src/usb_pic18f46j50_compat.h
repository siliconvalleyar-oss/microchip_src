#ifndef USB_PIC18F46J50_COMPAT_H
#define USB_PIC18F46J50_COMPAT_H

#include <xc.h>

#define UOTGIRbits UIRbits
#define UOTGIEbits UIEbits
#define TRXIF TRNIF

#ifndef ACTVIF
#define ACTVIF 0
#endif

#endif
