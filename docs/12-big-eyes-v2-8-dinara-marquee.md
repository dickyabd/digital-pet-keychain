# Big Eyes v2.8 — DINARA Marquee

## Purpose

v2.8 adds a special name reveal gesture to the tested v2.6/v2.7 touch interaction. A continuous touch-and-hold for three seconds launches the word **DINARA** as a right-to-left marquee with a playful pink font, white outline, and animated popping hearts. When the complete word has left the display, the normal pet eyes return automatically.

## Status

**Firmware committed; Verify/Compile and physical LCD test pending.** The TTP223B itself and the previous touch integration were tested by the project owner. The new custom-font marquee and gesture timing have not yet been compiled or visually tested on the physical LCD.

## Gesture mapping

The TTP223B is connected to GPIO 4 and provides an active-HIGH output.

| Gesture | Action |
|---|---|
| Short touch, less than 700 ms | Happy reaction; counts toward five-tap fear sequence |
| Five short taps within three seconds | Fear reaction |
| Hold 700 ms to under three seconds, then release | Cycle eye color once |
| Continuous hold for three seconds | Launch DINARA marquee once |

A medium hold changes color on release. It no longer changes color immediately at the 700 ms threshold, because the firmware must allow the same continuous hold to reach the three-second DINARA gesture.

A long hold is consumed once the marquee starts. Releasing afterward does not also trigger a color change or short tap. Touch events while the marquee is running are ignored so they cannot interrupt the name animation.

## DINARA visual treatment

- Text: `DINARA`
- Font: Adafruit GFX bundled `FreeSansBoldOblique18pt7b`
- Letter fill: pink RGB565 `0xFB56`
- Outline: white RGB565 `0xFFFF`, drawn in eight surrounding offsets
- Motion: one continuous right-to-left marquee
- Scroll speed: 2.4 canvas pixels per animation frame (nominal 25 ms frame interval)
- Decoration: six small hearts that pulse in size while moving with the name
- Heart fill: pink, with a white outline
- Completion: once the full word passes beyond the left edge, the normal eye animation resumes

The marquee uses the existing 240 × 174 off-screen `GFXcanvas16` and the same ST7789 transfer path as the eye animation. It does not introduce another framebuffer.

## Existing face behavior retained

- Solid cyan cartoon eyes with no pupils, inner dots, or highlights.
- Random idle gaze in six directions with spring-like motion.
- Direction-dependent squash-and-stretch.
- Synchronized blink and occasional double blink.
- Short-touch happy reaction.
- Five-tap fear reaction.
- Medium-hold color cycling across cyan, yellow, white, and magenta.
- Serial debug commands: `P` panic, `B` force blink, `C` center gaze.

## Hardware and display setup

- ESP32-S3 N16R8
- YY-PS130-V4.0 ST7789 LCD, 240 × 240 SPI
- TTP223B v1.0 High Level touch module
- Adafruit GFX and Adafruit ST7735/ST7789 libraries

LCD mapping remains unchanged:

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

Touch sensor:

| TTP223B pin | ESP32-S3 |
|---|---|
| SIG | GPIO 4 |
| VCC | 3.3 V |
| GND | GND |

Display initialization:

```cpp
SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
tft.init(240, 240, SPI_MODE3);
tft.setRotation(2);
```

## Compile and test procedure

1. Preserve the tested v2.6 and v2.7 sketches as backups.
2. Open `firmware/big-eyes-v2-8-dinara-marquee/DigitalPet_BigEyes_v2_8_DinaraMarquee.ino` in Arduino IDE.
3. Keep the verified LCD and TTP223B wiring unchanged.
4. Run Verify/Compile first.
5. Upload only after compilation succeeds.
6. Test a short tap: happy reaction should still work.
7. Test five short taps within three seconds: fear reaction should still work.
8. Hold for about one second and release: eye color should change.
9. Hold continuously for three seconds: DINARA should scroll from right to left with pink letters, a white outline, and pulsing heart decorations.
10. Wait until the end of the marquee: the normal eyes should return automatically.
11. Verify that releasing the three-second hold does not trigger a second gesture.

## Notes

- The custom font is included with the Adafruit GFX library; no separate font library is required.
- The name animation runs within the existing canvas allocation of 83,520 bytes.
- No MPU6050 is integrated in v2.8.
- Continue prototyping from USB power only. LiPo integration remains pending.
- Disconnect USB before changing wiring.
