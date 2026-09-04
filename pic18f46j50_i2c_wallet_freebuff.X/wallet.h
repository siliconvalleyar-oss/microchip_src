// 
// wallet.h - Crypto Wallet System for PIC18F46J50 + SSD1306
//

#ifndef WALLET_H
#define	WALLET_H

#include <xc.h>
#include <stdint.h>

#define MAX_BANKS       5
#define PIN_LENGTH      4
#define MENU_ITEMS      5

typedef struct {
    const char* bankName;
    const char* username;
    const char* password;
    const char* account;
} BankEntry;

extern const BankEntry demoBanks[MAX_BANKS];

typedef enum {
    STATE_SPLASH = 0,
    STATE_PIN_ENTRY,
    STATE_MAIN_MENU,
    STATE_VIEW_BANKS,
    STATE_BANK_DETAIL,
    STATE_BTC_WALLET,
    STATE_ETH_WALLET,
    STATE_ADD_KEY,
    STATE_ABOUT
} AppState;

void Wallet_Init(void);
void Wallet_Update(uint8_t buttons);

#endif	/* WALLET_H */
