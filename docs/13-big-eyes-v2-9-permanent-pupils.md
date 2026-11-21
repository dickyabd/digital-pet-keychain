# Big Eyes v2.9 — Permanent Pupils

## Goal

Try a version of the pet face with pupils always present during normal open-eye expressions, while preserving the v2.8 animation and touch interaction.

## Visual changes

- Each colored eye now contains one dark oval pupil.
- Pupils move inside the eye in the direction of the current gaze.
- Pupil size follows the eye dimensions, so the pupils scale down with the smaller scared expression.
- Pupils are not drawn while the eye is almost fully closed during a blink.
- No extra highlights, inner dots, or mouth were added.

## Retained v2.8 behavior

- Random idle gaze with spring-like movement.
- Normal blink and occasional double blink.
- Short touch: happy reaction.
- Five short taps within three seconds: fear reaction.
- Hold for 700 ms to under three seconds, then release: cycle eye color.
- Continuous hold for three seconds: DINARA pink marquee with outlined pulsing hearts.
- Serial Monitor commands: `P` = panic, `B` = blink, `C` = center gaze.

## Hardware and pin mapping

The wiring is unchanged from v2.8:

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

The sketch continues to use Adafruit GFX, Adafruit ST7789, and the existing 240 × 174 16-bit canvas.

## Test status

**Firmware prepared; physical test pending.**

Before treating this version as the new baseline:

1. Compile and upload with the same Arduino IDE and board settings used for v2.8.
2. Check that pupils remain inside both eye shapes at left, right, up, down, and diagonal gaze positions.
3. Check that pupils scale appropriately during happy and fear reactions.
4. Check that pupils disappear cleanly during blinking and return when the eyes open.
5. Recheck the short-touch, five-tap fear, medium-hold color change, and DINARA marquee gestures.

Keep v2.8 as the known-good fallback until v2.9 has been tested on the device.
