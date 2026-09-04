# Changelog

## v3.0 (current)
- Port from MC68HC908AP32 to PIC18F4620
- ADC changed from 8-bit to 10-bit, values scaled x4
- EEPROM for config storage instead of Flash
- Timer2 (1ms base) instead of TIM2
- CCP1 PWM instead of TIM1
- Button polling instead of KBI interrupts
- Custom EEPROM read/write for XC8 v2.45 compatibility
- Debug mode (MENOS+RESET at boot)
- Auto-reset algorithm for ground balance
- 20x4 LCD with custom character vu-meter
