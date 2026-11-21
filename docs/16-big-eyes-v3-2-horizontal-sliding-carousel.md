# Big Eyes v3.2 — Horizontal Sliding Carousel

## Goal

Use a horizontal sliding transition for the complete eye display frame instead of replacing styles in place.

The same transition effect is used for:
1. Moving from one eye style to the next during roulette.
2. Moving the eyes out and bringing DINARA into view.
3. Moving the final DINARA marquee frame out and bringing the pet eyes back.

## Horizontal transition behavior

Each transition uses the same direction:
- The outgoing full frame moves to the left.
- The incoming full frame enters from the right.
- The transition covers the 240 × 174 eye canvas area (screen y=33 to y=206).
- The bottom roulette status area is drawn separately and is not part of the sliding eye canvas.

For style roulette, every new selected style is rendered into an off-screen canvas first. The previous full canvas frame is saved, then a line-by-line composite is streamed to the ST7789 with Adafruit_SPITFT `startWrite()`, `setAddrWindow()`, and `writePixels()`. This avoids redrawing one pixel at a time during the slide.

## Eye styles

The five styles from v3.0 / v3.1 are retained:

| Style | Sclera | Pupil |
|---|---|---|
| Normal | White | Navy oval with white highlight |
| Cute | Purple | Small green oval with white highlight |
| Star | Lime | Yellow five-point star |
| Heart | White | Pink heart with a small white highlight |
| Fire | Amber | Red/orange flame with a yellow inner flame |

Pupils continue to follow the gaze and remain inside the sclera.

## Roulette and touch behavior

| Gesture | Action |
|---|---|
| Short touch under 700 ms | Happy reaction and counts toward the fear sequence |
| Five short touches within 3 seconds | Fear reaction |
| Hold reaches 700 ms | Starts the eye-style roulette |
| Continuous hold for 3 seconds | Starts DINARA marquee; takes priority over roulette |

The roulette takes approximately 1.7 seconds before its final style transition. Outgoing and incoming styles slide horizontally; the interval between style changes increases toward the final result. Releasing touch does not cancel the roulette.

The three-second DINARA hold is preserved. The eye pair slides left while the DINARA marquee frame enters from the right. After DINARA has moved completely off the left edge, the pet eyes slide in from the right.

Touch changes are tracked but new gestures are ignored while a slide transition is active, preventing overlapping transitions.

Serial Monitor commands remain: `P` = panic, `B` = blink, `C` = center gaze.

## Hardware and libraries

No wiring change from the tested v2.8 baseline:

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

No MPU6050 or additional hardware is required for v3.2.

## Memory

The original 240 × 174 16-bit eye canvas is retained (83,520 bytes). The transition renderer allocates one additional saved frame (83,520 bytes) and one 240-pixel scanline (480 bytes) at runtime.

## Test status

**Firmware prepared; compile, upload, and physical testing pending.**

1. Compile with the same Arduino IDE, board selection, and libraries used for v2.8/v3.1.
2. Test roulette: each outgoing eye pair should slide left and each next style should enter from the right.
3. Hold continuously for three seconds: verify the eye pair slides away and DINARA enters from the right.
4. Wait for DINARA to exit fully: verify the pet eyes slide back in from the right.
5. Re-test short touch, five-tap fear, blinking, and the roulette result.

Keep v2.8 as the known-good firmware backup until v3.2 has been tested on the device.
