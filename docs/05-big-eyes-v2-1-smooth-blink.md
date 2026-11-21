# Big Eyes v2.1 — Smooth Synchronized Blink

## Purpose

Big Eyes v2.1 improves the v2 blinking prototype by drawing both eyes into one off-screen 16-bit graphics canvas, then transferring the combined region to the ST7789 LCD as one bitmap. This avoids drawing and clearing each eye separately on the physical screen.

## Status

**Visual test: reported successful by the project owner.** The owner reported that the latest video no longer showed the visible glitch seen in v2. Treat this as a successful visual prototype test, not a measured frame-rate or long-duration reliability test.

## Hardware baseline

- ESP32-S3 N16R8
- YY-PS130-V4.0 LCD module
- ST7789 controller
- 240 × 240 pixels
- SPI interface

## Wiring

The known-good LCD wiring is preserved:

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

## Display setup

```cpp
SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
tft.init(240, 240, SPI_MODE3);
tft.setRotation(2);
```

SPI Mode 3 and rotation 2 are retained from the known-good Big Eyes v1 setup.

## Rendering method

- Uses `GFXcanvas16` from Adafruit GFX.
- Canvas size: 160 × 100 pixels.
- 16-bit RGB565 canvas memory: 32,000 bytes.
- Both eyes are rendered into the same canvas.
- The canvas is transferred to the LCD with one `drawRGBBitmap()` call per changed animation frame.
- The loop uses `millis()` and redraws only when the calculated eye height changes.
- The canvas allocation is checked before use.

This version does not use TFT_eSPI, GC9A01, MPU6050, or sensor inputs.

## Blink behavior

| Phase | Duration |
|---|---:|
| Closing | 160 ms |
| Closed | 70 ms |
| Opening | 160 ms |
| Delay before next blink | Random 2,500–5,500 ms |
| Initial delay before first blink | Random 1,800–3,500 ms |

Both eyes use the same calculated height and are rendered together to keep their animation synchronized.

The loop checks for a frame update every 16 ms (nominal scheduling interval about 62.5 Hz). This is not a measured or guaranteed display frame rate.

## How to test

1. Open `firmware/big-eyes-v2-1-smooth-blink/DigitalPet_BigEyes_v2_1_SmoothBlink.ino` in Arduino IDE.
2. Keep the verified wiring unchanged.
3. Verify/compile, then upload to the ESP32-S3.
4. Confirm both eyes close and open together, with no obvious tearing or flicker.
5. Return to Big Eyes v1 if the display does not initialize.

## Next improvements

- Tune blink speed and pause distribution.
- Add occasional double-blinks.
- Add gaze movement and expressive eye shapes.
- Add touch and motion interactions after the display animation is stable.
