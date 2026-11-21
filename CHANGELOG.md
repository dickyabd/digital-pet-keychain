# Changelog

## 2026-11-21 — Big Eyes v2.2 Gaze + Double Blink prepared

- Added `firmware/big-eyes-v2-2-gaze-double-blink/DigitalPet_BigEyes_v2_2_GazeDoubleBlink.ino`.
- Added automatic gaze sequence: center, left, right, up, down, upper-left, and upper-right, returning to center between directions.
- Added smooth pupil movement with bounded offsets inside each eye.
- Added occasional double blink behavior.
- Preserved the v2.1 off-screen canvas renderer and verified ST7789 wiring / SPI Mode 3 settings.
- Physical compile, upload, and visual tests are pending.

## 2026-11-21 — Big Eyes v2.1 Smooth Blink visually verified

- Added `firmware/big-eyes-v2-1-smooth-blink/DigitalPet_BigEyes_v2_1_SmoothBlink.ino`.
- Added off-screen rendering with one 160 × 100, 16-bit GFX canvas for both eyes.
- Added synchronized close, hold, and open phases.
- Transferred the combined eye image to the LCD with `drawRGBBitmap()`.
- Project owner reported that the latest video no longer showed the visible glitch seen in v2.
- Added v2.1 implementation notes.

## 2026-11-21 — Big Eyes v2 Auto Blink firmware prepared

- Added `firmware/big-eyes-v2-auto-blink/DigitalPet_BigEyes_v2_AutoBlink.ino`.
- Added non-blocking blink state machine using `millis()`.
- Added closing, closed, and opening phases with a randomized pause between blinks.
- Preserved the verified ST7789, SPI Mode 3, GPIO, and rotation 2 configuration.
- First visual test showed visible drawing artifacts; retained as a historical iteration.

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
