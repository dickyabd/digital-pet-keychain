# Changelog

## 2026-11-21 — Big Eyes v2 Auto Blink firmware prepared

- Added `firmware/big-eyes-v2-auto-blink/DigitalPet_BigEyes_v2_AutoBlink.ino`.
- Added non-blocking blink state machine using `millis()`.
- Added closing, closed, and opening phases with a randomized pause between blinks.
- Preserved the verified ST7789, SPI Mode 3, GPIO, and rotation 2 configuration.
- Physical compile/upload/display test is pending.

## 2026-11-21 — Big Eyes v1 milestone

- Added `firmware/big-eyes-v1/DigitalPet_BigEyes_v1.ino`.
- Reused the verified ESP32-S3 and ST7789 wiring.
- Retained SPI Mode 3 and set display rotation to 2 for the upright screen orientation.
- Displayed two large cyan eyes with white highlights.
- This version is a static prototype; automatic blinking, eye movement, and sensor interactions are future work.

## 2026-11-21 — Initial hardware milestone

- Set up ESP32-S3 N16R8 with Arduino IDE 2.3.11.
- Installed Espressif ESP32 Arduino core 3.3.7.
- Installed Adafruit ST7735 and ST7789 Library 1.12.0 and Adafruit GFX Library.
- Identified LCD module as YY-PS130-V4.0, 240 × 240, ST7789, SPI.
- Rebuilt the jumper wiring.
- Confirmed SPI Mode 3 produces visible LCD graphics.
- Displayed DIGITAL PET text and a yellow circle.
- Documented pin mapping, power rails, and troubleshooting.
- LiPo battery integration remains pending.
