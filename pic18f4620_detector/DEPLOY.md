# Deployment

## Production Programming

### Single Unit
1. Build project (`make`)
2. Open hex file in MPLAB IPE
3. Connect Pickit3/4 to ICSP header on detector board
4. Power board via Pickit or external 5V
5. Program device
6. Verify checksum

### Batch Programming
Use MPLAB IPE batch mode or a Pickit3 production programmer:

```bash
# Example: ipecmd.sh (adjust paths for your system)
/opt/microchip/mplabx/v5.45/mplab_platform/bin/ipecmd.sh \
  -TPPK3 \
  -P18F4620 \
  -M \
  -F"dist/default/production/pic18f4620_detector.production.hex"
```

## EEPROM Initialization
On first boot, the firmware initializes EEPROM with defaults:
- Volumen: 3
- Discrimination: 1
- SamplePosition: 4
- Ganancia: 2
- Tone: 60
- OffsetTono: 0

To force re-initialization, hold RESET button >5 seconds.

## Hardware Notes
- Crystal: 4MHz, use HSPLL fuses for 16MHz operation
- MCLR pin has pull-up resistor for Pickit3 programming
- ICSP: PGD (RB7), PGC (RB6), MCLR, VDD, VSS
- Power latch MOSFET: driven by RA0, gated by power button on RE0
- All unused pins left as inputs (no internal pull-ups configured)
