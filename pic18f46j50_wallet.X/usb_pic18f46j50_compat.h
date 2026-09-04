#ifndef USB_PIC18F46J50_COMPAT_H
#define USB_PIC18F46J50_COMPAT_H

#include <xc.h>

// Register compatibility mappings
#define UOTGIRbits UIRbits
#define UOTGIEbits UIEbits
#define TRXIF TRNIF

// Fix for activity interrupt flag
#ifndef ACTVIF
#define ACTVIF 0
#endif

#endif
