# Installation

## Prerequisites
- MPLAB X IDE (v5.45+ recommended)
- XC8 Compiler (v2.45+)
- Pickit3 or Pickit4 programmer
- 5V power supply for the detector board

## Build

### Via MPLAB X
1. Open MPLAB X IDE
2. File → Open Project → select the project folder
3. Select configuration (default)
4. Clean and Build (hammer icon)

### Via Command Line
```bash
make
make clean  # if rebuild needed
```

## Program

### Via MPLAB IPE
1. Open MPLAB IPE
2. Select device: PIC18F4620
3. Select tool: Pickit3/4
4. Browse for hex file: `dist/default/production/pic18f4620_detector.production.hex`
5. Program

### Via Command Line (if configured)
```bash
make flash
```

## First Boot
1. Apply 5V power
2. Press and hold the power button until splash screen appears
3. Release power button
4. Detector enters main menu after 1s splash
5. Configuration is initialized from EEPROM defaults
