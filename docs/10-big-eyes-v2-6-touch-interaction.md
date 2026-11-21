# Big Eyes v2.6 — Touch Interaction

## Purpose

Big Eyes v2.6 integrates the TTP223B touch sensor with the existing Adafruit ST7789 / GFXcanvas16 cartoon eye firmware. It preserves the idle gaze, smooth canvas rendering, synchronized blink, occasional double blink, and Serial Monitor debug commands.

## Status

**Firmware committed; compile and physical integration test pending.** The TTP223B was separately tested with the ESP32-S3 and produced one `TOUCH DETECTED!` event on touch and one `TOUCH RELEASED!` event on release. The integrated v2.6 firmware has not yet been compiled or tested on the physical LCD with the sensor connected.

## Hardware

- ESP32-S3 N16R8
- YY-PS130-V4.0, ST7789, 240 × 240 SPI LCD
- TTP223B v1.0 touch module, marked High Level
- LCD and sensor powered from 3.3 V
- PC USB power during prototyping; no LiPo connected

## Wiring

LCD wiring is unchanged:

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

TTP223B:

| TTP223B pin | ESP32-S3 |
|---|---|
| SIG | GPIO 4 |
| VCC | 3.3 V |
| GND | GND |

The module is marked High Level, so the sketch treats a HIGH output as an active touch. The sensor output is debounced in software for 35 ms.

## Touch behavior

- Short touch (press then release before 700 ms): trigger a happy reaction. The eyes become wider and shorter for 850 ms, then return to idle.
- Long touch (held for at least 700 ms): cycle through the eye colors (cyan, yellow, white, magenta) once per press and trigger the same brief visual feedback.
- Releasing a long touch does not also trigger the short-touch reaction.

## Existing behavior retained

- Random idle gaze: left, right, up, down, upper-left, upper-right, returning to center.
- Damped-spring gaze movement.
- Direction-dependent squash-and-stretch.
- Synchronized blink and occasional double blink.
- Simulated panic reaction from Serial Monitor.

## Serial Monitor debug commands

Set Serial Monitor to 115200 baud:

| Command | Action |
|---|---|
| `P` | Trigger panic reaction |
| `B` | Force the next blink |
| `C` | Return gaze target to center |

## Display and rendering

```cpp
SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
tft.init(240, 240, SPI_MODE3);
tft.setRotation(2);
```

- Adafruit GFX and Adafruit ST7735/ST7789 libraries.
- 240 × 174 pixel, 16-bit `GFXcanvas16` buffer.
- Canvas memory: 83,520 bytes.
- Both eyes are rendered together and transferred using `drawRGBBitmap()`.

## How to test

1. Preserve the known-good v2.5 sketch as a separate file.
2. Open `firmware/big-eyes-v2-6-touch-interaction/DigitalPet_BigEyes_v2_6_TouchInteraction.ino`.
3. Keep the LCD wiring unchanged and connect TTP223B SIG to GPIO 4, VCC to 3.3 V, and GND to common ground.
4. Disconnect USB before changing any wiring. Do not connect the LiPo battery.
5. Verify/compile first.
6. Upload after compilation succeeds.
7. Test a short touch: the eyes should briefly become wider and shorter.
8. Test a long touch (at least 700 ms): the eye color should change once.
9. Check that releasing a long touch does not also trigger a short-touch event.
10. Verify idle gaze, blink, and Serial Monitor commands still work.

## Notes and limitations

- The integrated v2.6 sketch is a candidate until compile/upload and physical LCD tests pass.
- No MPU6050 or physical panic trigger is integrated yet.
- Do not use GPIO 8 for MPU6050 SDA; GPIO 8 is the LCD reset line.
- No LiPo/battery operation is part of this test.
