# LCD Troubleshooting Notes

## Symptom

The ESP32-S3 uploaded and ran the sketch, but the ST7789 LCD initially remained a solid blue-purple tint with no visible graphics.

## Checks performed

1. Confirmed the ESP32-S3 could be selected in Arduino IDE and uploaded to over the COM port.
2. Confirmed the sketch compiled and uploaded.
3. Inspected the LCD rear PCB marking: YY-PS130-V4.0, 240 × 240, ST7789, SPI.
4. Rechecked the LCD header order and wire mapping.
5. Reseated the ESP32-S3 board in the breadboard.
6. Replaced the LCD jumper wires and rebuilt the wiring.
7. Ran Serial Monitor diagnostics at 115200 baud. The program printed its progress messages through completion.
8. Tested a lower SPI clock; this did not resolve the blank/tinted display.
9. Changed the initialization to SPI Mode 3.

## Working change

The visible display was achieved after using:

```cpp
tft.init(240, 240, SPI_MODE3);
```

Earlier attempts with SPI Mode 2 did not display the test graphics in this prototype.

## Interpretation

Serial messages such as "LCD initialized" and "Test complete!" only show that the ESP32 code continued executing. SPI has no automatic acknowledgement from this LCD in the wiring used, so those messages do not prove that the display received the data.

The final visible text and circle are the evidence that the display communication worked.

## If the issue returns

- Confirm the exact LCD PCB marking and header order.
- Confirm GPIO labels on the actual ESP32 board, rather than breadboard row numbers.
- Confirm SCL is connected to GPIO 12 and SDA/MOSI to GPIO 11.
- Confirm RES is GPIO 8 and DC is GPIO 10.
- Confirm the LCD has common GND and 3.3 V.
- Keep BLK in the known-good state (disconnected) until the exact module documentation indicates otherwise.
- Check breadboard rail segmentation and jumper seating.
- Use a multimeter to verify 3.3 V, common ground, and continuity when one is available.
- Return to the known-good sketch before making further changes.

## Known-good baseline

See [firmware/lcd-test/lcd-test.ino](../firmware/lcd-test/lcd-test.ino).
