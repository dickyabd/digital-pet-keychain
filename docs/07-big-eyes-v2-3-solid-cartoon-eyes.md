# Big Eyes v2.3 — Solid Cartoon Eyes

## Design direction

This iteration removes pupils, highlights, and inner dots. The LCD displays only two large solid cyan eye shapes. The eye shapes themselves move to suggest cartoon-like gaze directions.

The goal is to use much more of the 240 × 240 LCD area than the earlier small-eye prototype.

## Status

**Firmware committed; physical compile/upload and visual test pending.** The sketch is a new candidate version. The previous v2.1 remains the known-good smooth blinking baseline.

## Display and canvas

- LCD: YY-PS130-V4.0, ST7789, 240 × 240
- Canvas: 240 × 174 pixels
- Pixel format: 16-bit RGB565
- Canvas allocation: 83,520 bytes
- Canvas position on LCD: x=0, y=33
- Eye width: 96 pixels
- Eye open height: 150 pixels
- Eyes are cyan solid rounded shapes with a subtle dark-cyan outline.
- No pupils, highlights, or inner dots.

## Cartoon gaze behavior

The entire pair of eye shapes moves together through this sequence, returning to center between directions:

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

The movement uses simple interpolation for a smooth transition. Each gaze direction is held for a randomized 1,000–1,600 ms.

## Blinking

The synchronized blink and occasional double blink are retained:

- Closing: 160 ms
- Closed: 80 ms
- Opening: 160 ms
- Normal pause: randomized 2,500–5,500 ms
- Occasional second blink after a 260 ms pause
- About one in five normal blink cycles is selected for a double blink

## Hardware and display setup

The known-good hardware mapping is unchanged:

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

Libraries:
- Adafruit GFX
- Adafruit ST7735 and ST7789

No TFT_eSPI, GC9A01, MPU6050, touch sensor, or button is used by this sketch.

## Memory note

The off-screen 16-bit canvas uses 83,520 bytes. The sketch checks that the canvas allocation succeeds before attempting to render. If allocation fails, it prints an error to Serial Monitor and stops.

## How to test

1. Open `firmware/big-eyes-v2-3-solid-cartoon-eyes/DigitalPet_BigEyes_v2_3_SolidCartoonEyes.ino` in Arduino IDE.
2. Keep the verified LCD wiring unchanged.
3. Run Verify/Compile.
4. Upload only after compilation succeeds.
5. Check that the two large solid eyes fill most of the display area.
6. Confirm the whole pair moves to left, right, up, down, upper-left, and upper-right, returning to center.
7. Confirm synchronized blinking and occasional double blinking.
8. If memory allocation or visual output fails, return to v2.1 as the known-good smooth-blink baseline.

## Design note

The gaze direction is conveyed by moving the eye shapes themselves, rather than moving pupils. If the movement reads as sliding rather than expressive cartoon gaze, tune offsets, eye spacing, and squash/stretch behavior in a later iteration.
