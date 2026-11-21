# Big Eyes v1 — Static Eye Prototype

## Purpose

This is the first large-eye visual prototype for Digital Pet Keychain. It intentionally displays only two eyes, without the previous cat head, ears, whiskers, or mouth. The goal is to evaluate the eye style before adding animation.

## Result

The sketch was uploaded successfully and the LCD visibly displayed:

- Two large cyan rounded-rectangle eyes.
- White highlights in each eye.
- Dark background.

The observed result is documented by the successful LCD photo from the build session.

## Hardware

- ESP32-S3 N16R8
- YY-PS130-V4.0 LCD module
- ST7789 controller
- 240 × 240 pixels
- SPI interface

## Wiring

This sketch uses the established LCD wiring:

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

## Display configuration

```cpp
SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
tft.init(240, 240, SPI_MODE3);
tft.setRotation(2);
```

SPI Mode 3 is retained because it was the configuration that produced visible output on this specific prototype. Rotation 2 produced the upright orientation shown in the successful Big Eyes photo.

## Rendering approach

The sketch uses:

- `Adafruit_ST7789`
- `Adafruit_GFX`
- `fillRoundRect()` for the eye shapes
- `drawRoundRect()` for the subtle outline
- `fillCircle()` for eye highlights

No TFT_eSPI migration, sprite buffer, PSRAM allocation, or MPU6050 integration is included in this version.

## Next iteration

- Add timed blinking.
- Add random blink intervals.
- Add eye gaze movement.
- Add happy, sleepy, surprised, and dizzy expressions.

Keep this sketch as the static Big Eyes v1 baseline before introducing animation.
