# Big Eyes v3.0 — Eye Style Roulette

## Goal

Build on v2.9 with multiple permanent eye styles and a roulette animation that cycles through the styles before selecting one at random.

## Eye styles

| Style | Sclera | Pupil |
|---|---|---|
| Normal | White | Navy oval with white highlight |
| Cute | Purple | Small green oval with white highlight |
| Star | Lime | Yellow five-point star |
| Heart | White | Pink heart with a small white highlight |
| Fire | Amber | Red/orange flame with a yellow inner flame |

Pupils move with the gaze and remain within the sclera. They are hidden during the near-closed portion of a blink.

## Roulette behavior

A touch hold from 700 ms to under 3 seconds, followed by release, starts the roulette.

- Duration: approximately 2.2 seconds.
- Early phase: styles change quickly.
- Late phase: the interval between style changes grows, creating a slowing-slot-machine effect.
- Finish: one style is selected randomly and remains active until the next roulette or a device restart.

The chosen eye style is currently held in RAM only; saving it across power cycles is not implemented yet.

## Touch gesture map

| Gesture | Action |
|---|---|
| Short touch under 700 ms | Happy reaction and counts toward the fear sequence |
| Five short touches within 3 seconds | Fear reaction |
| Hold 700 ms to under 3 seconds, then release | Start eye-style roulette |
| Continuous hold for 3 seconds | DINARA marquee |
| Serial Monitor `P` | Panic reaction |
| Serial Monitor `B` | Force a blink |
| Serial Monitor `C` | Center gaze |

Touch input is ignored during the roulette and DINARA marquee so an in-progress animation cannot be interrupted.

## Hardware and libraries

The wiring and display driver remain the same as the tested v2.8 baseline:

| Function | ESP32-S3 pin |
|---|---:|
| LCD SCL / SCK | GPIO 12 |
| LCD SDA / MOSI | GPIO 11 |
| LCD RES / RST | GPIO 8 |
| LCD DC | GPIO 10 |
| LCD CS | Not exposed; sketch uses `TFT_CS = -1` |
| TTP223B SIG | GPIO 4 |
| LCD and touch VCC | 3.3 V |
| LCD and touch GND | GND |

Libraries: Adafruit GFX, Adafruit ST7735 and ST7789. The bundled FreeSans Bold Oblique font is used for the DINARA marquee.

The supplied Gemini sketch was used as a reference for pupil shape variety and roulette timing. Its TFT_eSPI display setup, MPU6050 wiring on GPIO 8/9, alternate character modes, and pixel-water mode were not copied because they are outside this hardware iteration and GPIO 8 is already used by LCD reset.

## Test status

**Firmware prepared; compile, upload, and physical visual test pending.**

Suggested test order:

1. Compile and upload using the same Arduino IDE, board selection, and verified wiring used for v2.8.
2. Confirm Normal, Cute, Star, Heart, and Fire render correctly and fit inside both eyes.
3. Hold the touch sensor for 700 ms to under 3 seconds, then release. Confirm roulette starts, speeds up visually at first, then slows before stopping.
4. Confirm the selected style remains visible after roulette ends.
5. Re-test short touch, five-tap fear, and the three-second DINARA marquee.

Keep v2.8 as the known-good fallback until v3.0 is tested on the device.
