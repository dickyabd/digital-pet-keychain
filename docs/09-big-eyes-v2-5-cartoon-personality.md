# Big Eyes v2.5 — Cartoon Personality Engine

## Purpose

Big Eyes v2.5 combines the smooth canvas rendering established in v2.1/v2.4 with the snappier idle-look behavior and a simulated startle reaction inspired by the Gemini sample. The design constraint remains: **two large solid cyan eyes only** — no pupils, inner dots, highlights, or mouth.

## Status

**Firmware committed; physical compile/upload test pending.** This is a candidate version. The known-good smooth blink baseline is v2.1, and v2.4 is the previous exaggerated-gaze candidate.

## Features

### Automatic idle gaze

The pet randomly looks in these directions, returning to center between looks:

- Left and right
- Up and down
- Upper-left and upper-right

Each look is held for a randomized 850–1,500 ms. Direction changes use a damped-spring update with a small overshoot, rather than a simple linear interpolation.

### Cartoon squash-and-stretch

The eye shapes themselves change according to the gaze direction:

- Left/right: narrower and taller.
- Up: wider and shorter.
- Down: narrower and taller.
- Diagonal: slightly narrower and taller.
- Center: base dimensions.

No pupils or highlights are rendered.

### Blink and double blink

- Closing: 150 ms.
- Closed: 75 ms.
- Opening: 150 ms.
- Normal pause: randomized 2,500–5,500 ms.
- Occasional additional blink after 240 ms.
- The second blink cannot trigger a third blink.

### Simulated panic reaction

The panic reaction can be tested through Serial Monitor at 115200 baud:

| Command | Action |
|---|---|
| `P` | Trigger a short panic / startle reaction |
| `B` | Force the next blink |
| `C` | Return the gaze target to center |

The panic reaction temporarily shakes and squashes/stretches the solid eyes. It is a software demo only; no touch sensor or MPU6050 is connected in this version.

## Hardware and libraries

- ESP32-S3 N16R8
- YY-PS130-V4.0 LCD
- ST7789, 240 × 240, SPI
- Adafruit GFX
- Adafruit ST7735 and ST7789

Known-good LCD pin mapping is unchanged:

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

## Rendering and memory

- Uses a 240 × 174 pixel, 16-bit RGB565 `GFXcanvas16`.
- Canvas memory: 83,520 bytes.
- Both eyes are rendered into one off-screen canvas and transferred together with `drawRGBBitmap()`.
- The canvas allocation is checked before use.
- Frame update scheduling is 25 ms; actual frame rate depends on SPI transfer and drawing time.

## How to test

1. Open `firmware/big-eyes-v2-5-cartoon-personality/DigitalPet_BigEyes_v2_5_CartoonPersonality.ino` in Arduino IDE.
2. Keep the verified LCD wiring unchanged.
3. Run Verify/Compile first.
4. Upload only after compilation succeeds.
5. Observe automatic idle gaze, squash-and-stretch, and blinking.
6. Open Serial Monitor at 115200 baud and send `P`, `B`, or `C` to test the software demo commands.
7. If any issue occurs, return to v2.1 as the known-good smooth-blink baseline.

## Next steps

- Complete physical compile/upload and visual validation.
- Map TTP223B touch output to a friendly reaction after the display behavior is stable.
- Integrate MPU6050 only after selecting I2C pins that do not conflict with the LCD wiring.
- Keep real sensor integration out of this candidate sketch until individually tested.
