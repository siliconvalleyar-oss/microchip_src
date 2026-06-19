# TODO

## High Priority
- [ ] Verify ADC calibration on real hardware (10-bit vs original 8-bit thresholds)
- [ ] Test auto-reset algorithm with actual coil
- [ ] Validate battery voltage scaling formula
- [ ] Confirm EEPROM write endurance and timing

## Medium Priority
- [ ] Add ferrous/non-ferrous discrimination audio (different tones)
- [ ] Implement noise floor tracking for dynamic threshold adjustment
- [ ] Add serial console for calibration (menu option to enable UART debug)

## Low Priority
- [ ] Add frequency domain analysis for target identification
- [ ] Add ground balance presets (salt/mineralized/neutral)
- [ ] Expand EEPROM to store user profiles (beach, field, all-metal)
- [ ] Add pinout diagram to hardware documentation

## Known Issues
- Some `__delay_us()` values assume exact 4MHz Fcy; verify with oscilloscope
- PWM frequency calculation needs verification against original HC08 timing
- LCD initialization timing may need adjustment for different LCD modules
- Debug mode `MostrarAjuste()` may display corruption if called too rapidly
