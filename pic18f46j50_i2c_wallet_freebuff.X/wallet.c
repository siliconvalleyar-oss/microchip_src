// 
// wallet.c - Crypto Wallet System for PIC18F46J50 + SSD1306
// RB4 = Move/Navigate, RB5 = Enter/Select
//

#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "hardware_cfg.h"
#include "oled.h"
#include "wallet.h"

//=============================================================
// DEMO DATA (stored in Flash via const)
//=============================================================
const BankEntry demoBanks[MAX_BANKS] = {
    {"Santander",   "jperez",     "P@ss1234",  "Cta: 1234-5678"},
    {"BBVA",        "mgarcia",    "Segura456",  "Cta: 8765-4321"},
    {"Banamex",     "rlopez",     "MyP@ss789",  "Nomina"},
    {"HSBC",        "agomez",     "Hsbc!321",   "Ahorros"},
    {"Scotiabank",  "mhernandez", "Sc0t1@",     "Platinum"}
};

static const char* const menuOptions[MENU_ITEMS] = {
    "Ver Bancos",
    "BTC Wallet",
    "ETH Wallet",
    "Agregar Clave",
    "Acerca de"
};

//=============================================================
// CORRECT PIN
//=============================================================
#define CORRECT_PIN "1234"

//=============================================================
// STATE VARIABLES
//=============================================================
static AppState currentState;
static uint8_t pinValid;                    // 0=wrong, 1=correct
static uint8_t enteredPin[PIN_LENGTH];      // entered digits
static uint8_t pinDigitIndex;               // current PIN position (0-3)
static uint8_t pinCursorPos;                // cursor in digit selector

static uint8_t shuffledDigits[10];          // shuffled 0-9 for PIN
static uint8_t menuOrder[MENU_ITEMS];       // shuffled menu indices
static uint8_t menuCursor;                  // current menu selection
static uint8_t currentBank;                 // current bank index

static uint16_t splashCounter;

//=============================================================
// SIMPLE PRNG (Linear Congruential Generator)
//=============================================================
static uint32_t randState = 0xABCD;

static uint8_t SimpleRand(void)
{
    randState = randState * 1103515245UL + 12345UL;
    return (uint8_t)(randState >> 8);
}

static void ShuffleArray(uint8_t* arr, uint8_t len)
{
    uint8_t i, j, temp;
    for(i = len - 1; i > 0; i--)
    {
        j = SimpleRand() % (i + 1);
        temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

//=============================================================
// CORRUPT STRING - ROT13 + shift digits (when PIN is wrong)
//=============================================================
static void CorruptString(const char* src, char* dst, uint8_t len)
{
    uint8_t i;
    for(i = 0; i < len - 1 && src[i] != '\0'; i++)
    {
        char c = src[i];
        if(c >= 'a' && c <= 'z')
        {
            dst[i] = ((c - 'a' + 13) % 26) + 'a';
        }
        else if(c >= 'A' && c <= 'Z')
        {
            dst[i] = ((c - 'A' + 13) % 26) + 'A';
        }
        else if(c >= '0' && c <= '9')
        {
            dst[i] = ((c - '0' + 5) % 10) + '0';
        }
        else
        {
            dst[i] = c;
        }
    }
    dst[i] = '\0';
}

//=============================================================
// DRAW SCREENS
//=============================================================
static void DrawSplash(void)
{
    OLED_SetCursor(18, 0);
    OLED_PrintString("CryptoWallet");
    OLED_SetCursor(15, 1);
    OLED_PrintString("Trezor Style");
    OLED_SetCursor(5, 2);
    OLED_PrintString("PIC18F46J50");
    OLED_SetCursor(15, 3);
    OLED_PrintString("Cargando...");
}

static void DrawPinEntry(void)
{
    uint8_t i;
    uint8_t idx = 0;
    
    // Line 0: PIN progress
    OLED_SetCursor(0, 0);
    OLED_PrintString("PIN: ");
    for(i = 0; i < pinDigitIndex; i++)
    {
        OLED_PrintChar('*');
    }
    // Clear rest of line
    for(i = (5 + pinDigitIndex); i < 21; i++)
    {
        OLED_PrintChar(' ');
    }
    
    // Line 1: first 4 digits (columns 10..34)
    OLED_SetCursor(15, 1);
    for(i = 0; i < 4; i++, idx++)
    {
        if(idx == pinCursorPos)
        {
            OLED_PrintCharInverted('0' + shuffledDigits[idx]);
            OLED_PrintChar(' ');
        }
        else
        {
            OLED_PrintChar('0' + shuffledDigits[idx]);
            OLED_PrintChar(' ');
        }
    }
    
    // Line 2: next 3 digits
    OLED_SetCursor(25, 2);
    for(i = 0; i < 3; i++, idx++)
    {
        if(idx == pinCursorPos)
        {
            OLED_PrintCharInverted('0' + shuffledDigits[idx]);
            OLED_PrintChar(' ');
        }
        else
        {
            OLED_PrintChar('0' + shuffledDigits[idx]);
            OLED_PrintChar(' ');
        }
    }
    
    // Line 3: last 3 digits
    OLED_SetCursor(25, 3);
    for(i = 0; i < 3; i++, idx++)
    {
        if(idx == pinCursorPos)
        {
            OLED_PrintCharInverted('0' + shuffledDigits[idx]);
            OLED_PrintChar(' ');
        }
        else
        {
            OLED_PrintChar('0' + shuffledDigits[idx]);
            OLED_PrintChar(' ');
        }
    }
}

static void DrawMainMenu(void)
{
    uint8_t i, j, len;
    const char* str;
    
    for(i = 0; i < MENU_ITEMS; i++)
    {
        OLED_SetCursor(0, i);
        str = menuOptions[menuOrder[i]];
        
        if(i == menuCursor)
        {
            // Invert entire line
            OLED_PrintCharInverted(' ');
            OLED_PrintStringInverted(str);
            // Pad rest of line with inverted spaces
            len = 0;
            for(j = 0; str[j] != '\0'; j++) len++;
            for(j = len + 1; j < 20; j++)
            {
                OLED_PrintCharInverted(' ');
            }
        }
        else
        {
            OLED_PrintChar(' ');
            OLED_PrintString(str);
            // Clear rest of line
            len = 0;
            for(j = 0; str[j] != '\0'; j++) len++;
            for(j = len + 1; j < 20; j++)
            {
                OLED_PrintChar(' ');
            }
        }
    }
}

static void DrawViewBanks(void)
{
    char buffer[22];
    const BankEntry* bank = &demoBanks[currentBank];
    
    // Header: "Banco X/Y"
    OLED_SetCursor(0, 0);
    OLED_PrintString("Banco ");
    OLED_PrintNumber(currentBank + 1);
    OLED_PrintChar('/');
    OLED_PrintNumber(MAX_BANKS);
    // Pad rest
    for(uint8_t i = 10; i < 21; i++) OLED_PrintChar(' ');
    
    // Bank name
    OLED_SetCursor(0, 1);
    OLED_ClearLine(1);
    OLED_SetCursor(0, 1);
    if(!pinValid)
    {
        CorruptString(bank->bankName, buffer, 16);
        OLED_PrintString(buffer);
    }
    else
    {
        OLED_PrintString(bank->bankName);
    }
    
    // Username
    OLED_SetCursor(0, 2);
    OLED_ClearLine(2);
    OLED_SetCursor(0, 2);
    OLED_PrintString("U: ");
    if(!pinValid)
    {
        CorruptString(bank->username, buffer, 16);
        OLED_PrintString(buffer);
    }
    else
    {
        OLED_PrintString(bank->username);
    }
    
    // Password
    OLED_SetCursor(0, 3);
    OLED_ClearLine(3);
    OLED_SetCursor(0, 3);
    OLED_PrintString("P: ");
    if(!pinValid)
    {
        CorruptString(bank->password, buffer, 16);
        OLED_PrintString(buffer);
    }
    else
    {
        OLED_PrintString(bank->password);
    }
}

static void DrawBankDetail(uint8_t bankIdx)
{
    char buffer[22];
    const BankEntry* bank = &demoBanks[bankIdx];
    
    OLED_SetCursor(0, 0);
    OLED_ClearLine(0);
    OLED_SetCursor(0, 0);
    if(!pinValid)
    {
        CorruptString(bank->bankName, buffer, 16);
        OLED_PrintString(buffer);
    }
    else
    {
        OLED_PrintString(bank->bankName);
    }
    
    OLED_SetCursor(0, 1);
    OLED_ClearLine(1);
    OLED_SetCursor(0, 1);
    OLED_PrintString("User: ");
    if(!pinValid)
    {
        CorruptString(bank->username, buffer, 16);
        OLED_PrintString(buffer);
    }
    else
    {
        OLED_PrintString(bank->username);
    }
    
    OLED_SetCursor(0, 2);
    OLED_ClearLine(2);
    OLED_SetCursor(0, 2);
    OLED_PrintString("Pass: ");
    if(!pinValid)
    {
        CorruptString(bank->password, buffer, 16);
        OLED_PrintString(buffer);
    }
    else
    {
        OLED_PrintString(bank->password);
    }
    
    OLED_SetCursor(0, 3);
    OLED_ClearLine(3);
    OLED_SetCursor(0, 3);
    if(!pinValid)
    {
        CorruptString(bank->account, buffer, 16);
        OLED_PrintString(buffer);
    }
    else
    {
        OLED_PrintString(bank->account);
    }
}

static void DrawBtcWallet(void)
{
    OLED_SetCursor(18, 0);
    OLED_PrintString("BTC Wallet");
    
    OLED_SetCursor(0, 1);
    OLED_ClearLine(1);
    OLED_SetCursor(0, 1);
    if(pinValid)
    {
        OLED_PrintString("Addr: 1A1zP1e...");
    }
    else
    {
        OLED_PrintString("Addr: 1BvBMSE...");
    }
    
    OLED_SetCursor(0, 2);
    OLED_ClearLine(2);
    OLED_SetCursor(0, 2);
    if(pinValid)
    {
        OLED_PrintString("Balance: 0.05 BTC");
    }
    else
    {
        OLED_PrintString("Balance: 0.00 BTC");
    }
    
    OLED_SetCursor(0, 3);
    OLED_ClearLine(3);
    OLED_SetCursor(0, 3);
    OLED_PrintString("RB5: Volver");
}

static void DrawEthWallet(void)
{
    OLED_SetCursor(18, 0);
    OLED_PrintString("ETH Wallet");
    
    OLED_SetCursor(0, 1);
    OLED_ClearLine(1);
    OLED_SetCursor(0, 1);
    if(pinValid)
    {
        OLED_PrintString("Addr: 0xAb5801...");
    }
    else
    {
        OLED_PrintString("Addr: 0x000000...");
    }
    
    OLED_SetCursor(0, 2);
    OLED_ClearLine(2);
    OLED_SetCursor(0, 2);
    if(pinValid)
    {
        OLED_PrintString("Balance: 1.25 ETH");
    }
    else
    {
        OLED_PrintString("Balance: 0.00 ETH");
    }
    
    OLED_SetCursor(0, 3);
    OLED_ClearLine(3);
    OLED_SetCursor(0, 3);
    OLED_PrintString("RB5: Volver");
}

static void DrawAbout(void)
{
    OLED_SetCursor(18, 0);
    OLED_PrintString("Acerca de");
    
    OLED_SetCursor(0, 1);
    OLED_ClearLine(1);
    OLED_SetCursor(0, 1);
    OLED_PrintString("Crypto Wallet v1");
    
    OLED_SetCursor(0, 2);
    OLED_ClearLine(2);
    OLED_SetCursor(0, 2);
    OLED_PrintString("PIC18F46J50");
    
    OLED_SetCursor(0, 3);
    OLED_ClearLine(3);
    OLED_SetCursor(0, 3);
    OLED_PrintString("SSD1306 I2C OLED");
}

static void DrawAddKey(void)
{
    OLED_SetCursor(12, 0);
    OLED_PrintString("Agregar Clave");
    
    OLED_SetCursor(0, 1);
    OLED_ClearLine(1);
    OLED_SetCursor(0, 1);
    OLED_PrintString("Funcion en");
    
    OLED_SetCursor(0, 2);
    OLED_ClearLine(2);
    OLED_SetCursor(0, 2);
    OLED_PrintString("desarrollo...");
    
    OLED_SetCursor(0, 3);
    OLED_ClearLine(3);
    OLED_SetCursor(0, 3);
    OLED_PrintString("RB5: Volver");
}

//=============================================================
// STATE MANAGEMENT
//=============================================================
static void EnterState(AppState newState)
{
    OLED_Clear();
    currentState = newState;
    
    switch(newState)
    {
        case STATE_SPLASH:
            DrawSplash();
            splashCounter = 0;
            break;
            
        case STATE_PIN_ENTRY:
            pinDigitIndex = 0;
            pinCursorPos = 0;
            {
                uint8_t i;
                for(i = 0; i < 10; i++) shuffledDigits[i] = i;
                ShuffleArray(shuffledDigits, 10);
            }
            DrawPinEntry();
            break;
            
        case STATE_MAIN_MENU:
            menuCursor = 0;
            {
                uint8_t i;
                for(i = 0; i < MENU_ITEMS; i++) menuOrder[i] = i;
                ShuffleArray(menuOrder, MENU_ITEMS);
            }
            DrawMainMenu();
            break;
            
        case STATE_VIEW_BANKS:
            currentBank = 0;
            DrawViewBanks();
            break;
            
        case STATE_BANK_DETAIL:
            DrawBankDetail(currentBank);
            break;
            
        case STATE_BTC_WALLET:
            DrawBtcWallet();
            break;
            
        case STATE_ETH_WALLET:
            DrawEthWallet();
            break;
            
        case STATE_ADD_KEY:
            DrawAddKey();
            break;
            
        case STATE_ABOUT:
            DrawAbout();
            break;
    }
}

//=============================================================
// PUBLIC FUNCTIONS
//=============================================================
void Wallet_Init(void)
{
    pinValid = 0;
    EnterState(STATE_SPLASH);
}

void Wallet_Update(uint8_t buttons)
{
    // Edge detection - only process new presses
    static uint8_t lastButtons = 0;
    uint8_t press = buttons & ~lastButtons;
    lastButtons = buttons;
    
    uint8_t rb4 = press & 0x01;  // Move/Navigate
    uint8_t rb5 = press & 0x02;  // Enter/Select
    
    switch(currentState)
    {
        case STATE_SPLASH:
            splashCounter++;
            // Wait ~2s or button press
            if(splashCounter > 100 || rb4 || rb5)
            {
                EnterState(STATE_PIN_ENTRY);
            }
            break;
            
        case STATE_PIN_ENTRY:
            if(rb4)
            {
                pinCursorPos++;
                if(pinCursorPos >= 10) pinCursorPos = 0;
                DrawPinEntry();
            }
            if(rb5)
            {
                if(pinDigitIndex < PIN_LENGTH)
                {
                    enteredPin[pinDigitIndex] = shuffledDigits[pinCursorPos];
                    pinDigitIndex++;
                    
                    if(pinDigitIndex >= PIN_LENGTH)
                    {
                        // Validate PIN
                        const char* correct = CORRECT_PIN;
                        pinValid = 1;
                        for(uint8_t i = 0; i < PIN_LENGTH; i++)
                        {
                            if(enteredPin[i] != (uint8_t)(correct[i] - '0'))
                            {
                                pinValid = 0;
                                break;
                            }
                        }
                        EnterState(STATE_MAIN_MENU);
                    }
                    else
                    {
                        DrawPinEntry();
                    }
                }
            }
            break;
            
        case STATE_MAIN_MENU:
            if(rb4)
            {
                menuCursor++;
                if(menuCursor >= MENU_ITEMS) menuCursor = 0;
                DrawMainMenu();
            }
            if(rb5)
            {
                uint8_t option = menuOrder[menuCursor];
                switch(option)
                {
                    case 0: EnterState(STATE_VIEW_BANKS); break;
                    case 1: EnterState(STATE_BTC_WALLET); break;
                    case 2: EnterState(STATE_ETH_WALLET); break;
                    case 3: EnterState(STATE_ADD_KEY); break;
                    case 4: EnterState(STATE_ABOUT); break;
                }
            }
            break;
            
        case STATE_VIEW_BANKS:
            if(rb4)
            {
                currentBank++;
                if(currentBank >= MAX_BANKS) currentBank = 0;
                DrawViewBanks();
            }
            if(rb5)
            {
                EnterState(STATE_BANK_DETAIL);
            }
            break;
            
        case STATE_BANK_DETAIL:
            if(rb4)
            {
                currentBank++;
                if(currentBank >= MAX_BANKS) currentBank = 0;
                DrawBankDetail(currentBank);
            }
            if(rb5)
            {
                EnterState(STATE_MAIN_MENU);
            }
            break;
            
        case STATE_BTC_WALLET:
        case STATE_ETH_WALLET:
        case STATE_ADD_KEY:
            if(rb5)
            {
                EnterState(STATE_MAIN_MENU);
            }
            break;
            
        case STATE_ABOUT:
            if(rb5)
            {
                EnterState(STATE_MAIN_MENU);
            }
            break;
    }
}
