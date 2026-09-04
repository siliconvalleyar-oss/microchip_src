# Architecture

## Overview

Pulse induction metal detector firmware ported from MC68HC908AP32 to PIC18F4620.
Single-threaded, interrupt-driven sampling loop with blocking menu UI.

## Execution Flow

```
Power ON
  └─ initMCU() → PLL, ports, ADC
  └─ Power latch (KEY_SENSE)
  └─ INITLCD() → LCD 4-bit mode
  └─ Presentacion() → splash screen
  └─ initTIM2() → start Timer2 interrupts
  └─ Global interrupts ON
  └─ VerEEPROM() → load config
  └─ Main loop (for;;):
       ├─ Detector mode (Menu==0):
       │    ├─ FlagCalcular → CalcularSegmentos()
       │    ├─ Update vu-meter segments
       │    └─ Auto-reset logic
       ├─ Button polling (MAS, MENOS, MENU, RESET, MUTE, LIGHT)
       ├─ Menu system (1-6 submenus with timeout)
       ├─ Battery check (every 10s)
       └─ Power-off detection
```

## Interrupts (Timer2 - 1ms base)

```
Timer2 ISR (1ms):
  ├─ GestionBobina():
  │    ├─ Pulse coil (48µs)
  │    ├─ Delay by SamplePosition (85-115µs)
  │    ├─ Sample switch (48µs)
  │    ├─ Read ADC (battery or signal)
  │    └─ Accumulate in Medicion
  ├─ Every 10ms → average ADC samples → ValorCalculado
  ├─ Every 100ms → manage counters, menu timeout
  ├─ Every 500ms → update Segmentos, flag FlagCalcular
  └─ Every 10s → trigger battery check
```

## Audio PWM (CCP1)

- CCP1 module on RC2 generates audio tone
- Frequency adjusted via Tone variable + OffsetTono
- Duty cycle proportional to ValorDefinitivo (signal strength)
- PWM output gated by Mute flag

## Signal Processing Chain

```
Coil pulse → Sample switch → ADC (AN7, 10-bit)
  → Accumulate 10 samples → Average → ValorCalculado
  → Subtract ValorZero (ground reference)
  → Apply Gain (ValorTemp * Ganancia / 2)
  → Apply Discrimination threshold
  → Divide by 10 → Segmentos (0-40)
  → Smoothing (gradual increment/decrement)
  → Display vu-meter bars
  → Adjust PWM duty cycle
```

## Auto-Reset Algorithm

When signal exceeds 8 segments, FlagAutoReset is set.
When signal drops below 6 segments, execute AccionReset():
1. Mute audio temporarily
2. Adjust sensitivity until ValorCalculado is 400-800
3. Recalculate ValorZero (ground reference)
4. Resume normal operation

## Menu System

| Menu | Variable | Range | EEPROM Addr |
|------|----------|-------|-------------|
| 0 | Detector mode | - | - |
| 2 | SamplePosition | 1-7 | 0x02 |
| 3 | Volumen | 0-30 | 0x00 |
| 4 | Ganancia | 1-20 | 0x04 |
| 5 | Tone | 60-100 | 0x05 |
| 6 | OffsetTono | 0-80 | 0x06 |

Menu 1 is skipped (unused). In debug mode, Menu 0 provides live ADC values and manual sensitivity adjustment.

## Power Management

- Power latch on RA0 holds MOSFET on while MCU runs
- KEY_SENSE (RE0) detects button release to shut down
- Battery measured on AN6 every 10s via voltage divider (5V * 11)
- Low battery warning at < threshold, auto-shutdown at critical level
- Configuration saved to EEPROM before power-off
