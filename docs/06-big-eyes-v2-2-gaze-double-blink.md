# Big Eyes v2.2 — Gaze Directions + Double Blink

## Purpose

Big Eyes v2.2 extends the visually smoother v2.1 canvas-based eye animation with automatic gaze direction changes and occasional double blinks.

## Status

**Firmware prepared; physical compile/upload test pending.** The previous v2.1 canvas animation was visually reported as smooth by the project owner. This v2.2 extension has not yet been verified on the physical LCD. Treat it as a candidate sketch until Verify, Upload, and visual tests succeed.

## Features

### Gaze directions

The pupils move smoothly through this sequence, returning to center between directions:

1. Center
2. Left
3. Center
4. Right
5. Center
6. Up
7. Center
8. Down
9. Center
10. Upper-left
11. Center
12. Upper-right
13. Center, then repeat

The gaze dwell time is randomized between 900 and 1,450 ms. Pupil offsets are bounded within each eye.

### Blink behavior

- Synchronized closing, closed, and opening phases.
- Closing: 160 ms.
- Closed: 70 ms.
- Opening: 160 ms.
- Normal blink pause: randomized 2,500–5,500 ms.
- Initial delay before first blink: randomized 1,800–3,500 ms.
- About one in five blinks is followed by a second blink after a short 260 ms pause.

### Rendering

- Uses an off-screen `GFXcanvas16` from Adafruit GFX.
- Canvas size: 160 × 100 pixels.
- 16-bit RGB565 canvas memory: 32,000 bytes.
- Both eyes are rendered into the same canvas and transferred together with `drawRGBBitmap()`.
- The loop schedules redraw checks every 25 ms; actual frame rate depends on SPI transfer and drawing time.
- Redraw occurs only when the eye height or rounded pupil position changes.

## Hardware baseline

- ESP32-S3 N16R8
- YY-PS130-V4.0 LCD module
- ST7789 controller
- 240 × 240 pixels
- SPI interface

The known-good wiring is unchanged:

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

No additional hardware, button, MPU6050, TFT_eSPI, or GC9A01 library is required for this sketch.

## How to test

1. Open `firmware/big-eyes-v2-2-gaze-double-blink/DigitalPet_BigEyes_v2_2_GazeDoubleBlink.ino` in Arduino IDE.
2. Keep the v2.1 known-good wiring unchanged.
3. Run Verify/Compile first.
4. Upload only after compilation succeeds.
5. Confirm that the pupils visit left, right, up, down, upper-left, and upper-right with center returns.
6. Confirm both eyes blink together and occasional double blinks occur.
7. Watch for tearing, pupil clipping, or canvas allocation failure.
8. If any problem occurs, return to the known-good v2.1 sketch.

## Next improvements

- Add lower-left and lower-right gaze if desired.
- Add gaze behavior tied to touch and motion sensors.
- Add more expressive eye shapes (happy, sleepy, surprised, dizzy).
