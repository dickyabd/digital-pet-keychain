# Big Eyes v2 — Automatic Blinking

## Purpose

Big Eyes v2 builds on the static Big Eyes v1 prototype. It keeps the same two large cyan eyes and adds synchronized automatic blinking.

## Status

The firmware file has been added to the repository as a new version. It has not yet been compiled or tested on the physical prototype in this iteration. Treat it as a candidate sketch until the Verify, Upload, and visual checks succeed.

## Hardware baseline

- ESP32-S3 N16R8
- YY-PS130-V4.0 LCD module
- ST7789 controller
- 240 × 240 pixels
- SPI interface

## Wiring and display configuration

The sketch preserves the known-good Big Eyes v1 configuration:

| LCD signal | ESP32-S3 |
|---|---|
| SCL / SCK | GPIO 12 |
| SDA / MOSI | GPIO 11 |
| RES | GPIO 8 |
| DC | GPIO 10 |
| CS | Not exposed; configured as -1 |
| VCC | 3.3 V |
| GND | GND |
| BLK | Left disconnected |

Display initialization:

```cpp
SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
tft.init(240, 240, SPI_MODE3);
tft.setRotation(2);
```

## Animation behavior

The two eyes blink together through four states:

1. Open
2. Closing
3. Closed
4. Opening

Current timing values:

| Phase | Duration |
|---|---:|
| Close | 90 ms |
| Hold closed | 70 ms |
| Open | 90 ms |
| Delay before next blink | Random 2,500–5,500 ms |
| Initial delay before first blink | Random 1,800–3,500 ms |

The loop uses `millis()` rather than blocking `delay()` for animation timing. Rendering is scheduled at a nominal 20 ms interval (up to 50 frame attempts per second); actual frame rate depends on display transfer and draw time.

Only the regions around the eyes are cleared and redrawn to reduce unnecessary full-screen updates.

## Libraries and memory

- Adafruit GFX
- Adafruit ST7735 and ST7789
- Arduino SPI

This version does not use TFT_eSPI, sprites, PSRAM allocation, or the MPU6050.

## How to test

1. Open `firmware/big-eyes-v2-auto-blink/DigitalPet_BigEyes_v2_AutoBlink.ino` in Arduino IDE.
2. Keep the known-good wiring unchanged.
3. Verify/compile the sketch.
4. Upload to the ESP32-S3.
5. Confirm both eyes appear open, close together briefly, reopen, and repeat after a variable pause.
6. If the display is blank or the compile fails, return to the known-good Big Eyes v1 sketch.

## Next improvements

- Add gaze movement.
- Add different blink durations and occasional double-blinks.
- Add happy, sleepy, surprised, and dizzy eye shapes.
- Add touch and motion input only after the display animation is stable.
