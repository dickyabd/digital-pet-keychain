# Big Eyes v2.7 — Five-Tap Fear Reaction

## Purpose

v2.7 adds a fear reaction to the tested v2.6 touch interaction. Five short taps on the TTP223B within a rolling 3-second window trigger a brief frightened expression.

## Status

**Firmware committed; Verify/Compile and physical test pending.** v2.6 touch integration was reported as working by the project owner. v2.7 is a new candidate adding multi-tap recognition and the fear animation; it has not yet been compiled or tested on the physical device.

## Touch gestures

| Gesture | Behavior |
|---|---|
| One to four short taps | Brief happy reaction after each tap |
| Five short taps within 3,000 ms | Trigger fear reaction |
| Hold for at least 700 ms | Cycle eye color once |
| Release after long hold | Does not count as a short tap |

The short-tap counter resets when the 3-second window expires. A long press clears the tap sequence and is not counted as a tap.

The TTP223B is treated as an active-HIGH module. GPIO 4 input is debounced for 35 ms.

## Fear reaction

When the fifth short tap is registered:

- The two eyes become smaller.
- The eyes move closer together, giving a huddled appearance.
- The eyes tremble rapidly with small random horizontal and vertical offsets.
- Eye height varies slightly during the reaction.
- The reaction lasts about 1.4 seconds, then returns to idle gaze and blinking.

The design remains solid cartoon eyes only: no pupils, highlights, inner dots, or mouth.

## Existing behavior retained

- Random idle gaze: left, right, up, down, upper-left, and upper-right, returning to center between looks.
- Damped-spring gaze motion.
- Direction-dependent squash-and-stretch.
- Synchronized blink and occasional double blink.
- Short-touch happy feedback.
- Long-touch color cycling: cyan, yellow, white, magenta.
- Serial debug commands: `P` panic, `B` force blink, `C` center gaze.

## Hardware

- ESP32-S3 N16R8
- YY-PS130-V4.0 ST7789 LCD, 240 × 240 SPI
- TTP223B v1.0 High Level output

LCD wiring remains unchanged:

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

## Rendering

- Adafruit GFX and Adafruit ST7735/ST7789.
- Off-screen 16-bit `GFXcanvas16`, 240 × 174 pixels.
- Canvas memory: 83,520 bytes.
- Both eyes are drawn to one canvas and transferred using `drawRGBBitmap()`.

## How to test

1. Keep the tested v2.6 sketch as a separate backup.
2. Open `firmware/big-eyes-v2-7-five-tap-fear/DigitalPet_BigEyes_v2_7_FiveTapFear.ino`.
3. Keep the verified LCD and TTP223B wiring unchanged.
4. Run Verify/Compile first.
5. Upload only after compilation succeeds.
6. Tap once to four times; each short tap should produce the happy reaction.
7. Tap five times quickly, completing all five taps within 3 seconds; the eyes should shrink, huddle, tremble, then return to idle.
8. Hold the sensor for at least 700 ms; the eye color should change once, and the long press should not contribute to the five-tap count.
9. Verify idle gaze, blinking, and Serial Monitor commands remain functional.

## Safety and limitations

- No MPU6050 is integrated in v2.7.
- Continue powering from USB during prototyping; LiPo integration remains pending.
- Disconnect USB before changing jumper wiring.
- The new five-tap feature remains unverified on hardware until the project owner completes the test.
