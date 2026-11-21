# Big Eyes v2.4 — Exaggerated Cartoon Gaze

## Design feedback addressed

The v2.3 test showed that the eyes occupied too much of the display and that the horizontal gaze offset was too small to read clearly. v2.4 reduces the base eye dimensions and expands gaze travel. It also uses simple squash-and-stretch shape changes to make directional looks feel more cartoon-like.

The design remains **solid eyes only**: no pupils, no highlights, and no inner dots.

## Status

**Firmware committed; physical compile/upload and visual testing pending.** v2.1 remains the known-good smooth-blink baseline. v2.4 is a candidate iteration and should be verified in Arduino IDE before upload.

## Base dimensions and movement

| Parameter | v2.3 | v2.4 |
|---|---:|---:|
| Base eye width | 96 px | 76 px |
| Base eye height | 150 px | 112 px |
| Horizontal gaze offset | ±7 px | ±24 px |
| Vertical gaze offset | ±5 px | ±16 px |
| Canvas | 240 × 174 px | 240 × 174 px |
| Canvas memory | 83,520 bytes | 83,520 bytes |

The two eyes are centered at x=72 and x=168 within the canvas. The full pair moves together to show the gaze direction.

## Cartoon squash-and-stretch

The eye shapes change proportions based on gaze direction:

- Left/right: narrower (68 px) and taller (112% height scale).
- Up: wider (86 px) and shorter (90% height scale).
- Down: narrower (72 px) and taller (110% height scale).
- Diagonal upper corners: narrower (70 px) and slightly taller (107% height scale).
- Center: base 76 × 112 px.

The eye radius is recalculated to keep rounded capsule-like ends.

## Gaze sequence

1. Center
2. Far left
3. Center
4. Far right
5. Center
6. Up
7. Center
8. Down
9. Center
10. Upper-left
11. Center
12. Upper-right
13. Center, then repeat

Gaze dwell time is randomized between 850 and 1,350 ms. Movement is smoothed using interpolation.

## Blinking

The synchronized blink and occasional double blink are retained:

- Closing: 160 ms
- Closed: 80 ms
- Opening: 160 ms
- Normal pause: randomized 2,500–5,500 ms
- Occasional additional blink after a 260 ms pause
- About one in five normal blink cycles is selected for a double blink

## Hardware and libraries

- ESP32-S3 N16R8
- YY-PS130-V4.0 LCD
- ST7789, 240 × 240, SPI
- Adafruit GFX
- Adafruit ST7735 and ST7789

Known-good LCD wiring remains unchanged:

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

The canvas is 240 × 174 pixels at 16-bit RGB565, using 83,520 bytes of memory. The sketch checks canvas allocation before rendering.

## How to test

1. Open `firmware/big-eyes-v2-4-exaggerated-cartoon-gaze/DigitalPet_BigEyes_v2_4_ExaggeratedCartoonGaze.ino` in Arduino IDE.
2. Keep the verified LCD wiring unchanged.
3. Run Verify/Compile.
4. Upload only after compilation succeeds.
5. Check that the eyes look noticeably smaller than v2.3.
6. Confirm left/right movement is obvious and does not clip at the screen edges.
7. Confirm up, down, upper-left, and upper-right looks.
8. Confirm synchronized blinking and occasional double blinks.
9. If anything fails, return to v2.1 as the known-good baseline.

## Design constraints

- Do not add pupils or highlights unless the project owner changes the design direction.
- No touch sensor, MPU6050, or button is used in this firmware.
- Battery operation is not part of this test.
