# Big Eyes v3.1 — Visible Eye Carousel

## Why v3.1 exists

In v3.0, roulette started only after the touch was released. The style changed in place, with no on-screen status indicator. This made the gesture easy to miss and the roulette effect unclear.

v3.1 starts the effect while the user is still holding the touch sensor and adds a visible status label and progress bar.

## Eye styles

The five eye styles from v3.0 are retained:

| Style | Sclera | Pupil |
|---|---|---|
| Normal | White | Navy oval with white highlight |
| Cute | Purple | Small green oval with white highlight |
| Star | Lime | Yellow five-point star |
| Heart | White | Pink heart with a small white highlight |
| Fire | Amber | Red/orange flame with a yellow inner flame |

## Roulette behavior

- Touch hold reaches 700 ms: roulette starts immediately.
- Styles change quickly at first, then progressively more slowly.
- Total roulette duration is approximately 2.2 seconds.
- The bottom of the screen displays `ROLLING...` and a progress bar while styles cycle.
- At the end, a random style is selected and its name is displayed briefly.
- Releasing touch does not cancel the animation.

## Touch gesture map

| Gesture | Action |
|---|---|
| Short touch under 700 ms | Happy reaction and counts toward the fear sequence |
| Five short touches within 3 seconds | Fear reaction |
| Hold reaches 700 ms | Starts eye-style roulette |
| Continuous hold for 3 seconds | DINARA marquee; takes priority over roulette |

The roulette usually completes before a three-second hold reaches the DINARA trigger. The longer hold still launches DINARA at the three-second mark.

Serial Monitor commands remain: `P` = panic, `B` = blink, `C` = center gaze.

## Hardware and libraries

Unchanged from v3.0 and the tested v2.8 baseline:

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

Libraries: Adafruit GFX and Adafruit ST7735 and ST7789. The DINARA marquee uses the bundled FreeSans Bold Oblique font.

No MPU6050 or additional wiring is required.

## Test status

**Firmware prepared; compile, upload, and physical testing pending.**

1. Compile and upload using the same Arduino IDE, board, and verified wiring as v2.8.
2. Touch and hold until the 700 ms threshold. Confirm that style changes begin while the finger is still touching.
3. Confirm the `ROLLING...` label and progress bar appear.
4. Release while it is rolling; confirm the roulette continues and locks onto one style.
5. Keep holding for three seconds; confirm DINARA takes priority.
6. Re-test short touch and five-tap fear.

Keep v2.8 as the known-good backup until v3.1 is physically tested.
