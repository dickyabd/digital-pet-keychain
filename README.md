# Digital Pet Keychain

An ESP32-S3-based interactive digital pet keychain. This repository records the hardware setup, LCD bring-up, firmware experiments, and future sensor and battery integration.

## Current status

**Milestone 01 — LCD bring-up: working**

The prototype successfully displayed the text **DIGITAL PET** and a yellow circle on the TFT LCD.

**Milestone 02 — Big Eyes v1: working**

The prototype successfully displayed a pair of large cyan eyes with white highlights on the ST7789 display. This is a static eye prototype; blinking and interaction are not implemented yet.

## Hardware baseline

| Item | Value |
|---|---|
| MCU board | ESP32-S3 N16R8 development board |
| LCD PCB | YY-PS130-V4.0 |
| LCD controller | ST7789 |
| Resolution | 240 × 240 pixels |
| Interface | SPI |
| Arduino IDE | 2.3.11 |
| ESP32 Arduino core | Espressif Systems 3.3.7 |
| Board selection | ESP32S3 Dev Module |
| Adafruit ST7735 and ST7789 Library | 1.12.0 |
| Host OS used during setup | Windows 11 |

## Verified LCD pin mapping

| LCD pin | ESP32-S3 connection | Wire color in the successful rebuild |
|---|---|---|
| GND | GND rail | Purple |
| VCC | 3V3 rail | Blue |
| SCL / SCK | GPIO 12 | Green / teal |
| SDA / MOSI | GPIO 11 | Yellow |
| RES / RST | GPIO 8 | Orange |
| DC | GPIO 10 | Red |
| BLK | Not connected | Brown wire left disconnected |

The LCD's SDA label is the SPI data input (MOSI) for this module; it is not being used as I²C SDA. The module has no exposed CS pin; the sketch uses `TFT_CS = -1`.

## Breadboard power distribution

| ESP32-S3 | Breadboard rail | Jumper color |
|---|---|---|
| 3V3 | Red (+) rail | Black |
| GND | Blue (-) rail | White |

The LCD VCC connects to the 3.3 V rail and LCD GND to the GND rail. Breadboard power rails may be split into separate segments. Do not assume every section is electrically continuous.

## Firmware

- [LCD test baseline](firmware/lcd-test/lcd-test.ino)
- [Big Eyes v1 — static cyan eyes](firmware/big-eyes-v1/DigitalPet_BigEyes_v1.ino)

The working display initialization is:

```cpp
SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
tft.init(240, 240, SPI_MODE3);
```

**SPI Mode 3 is the configuration that produced visible output in this prototype.** The Big Eyes v1 sketch also uses `tft.setRotation(2)`, which produced the upright orientation in the observed LCD photo.

The Big Eyes v1 sketch uses Adafruit GFX drawing primitives. It does not require TFT_eSPI, sprite buffering, PSRAM allocation, or the MPU6050.

## Documentation

- [Hardware setup and wiring](docs/01-hardware-setup.md)
- [Troubleshooting notes](docs/02-troubleshooting.md)
- [Big Eyes v1 notes](docs/03-big-eyes-v1.md)
- [Changelog](CHANGELOG.md)

## Next milestones

1. Add automatic eye blinking.
2. Add eye movement and expressions.
3. Add touch interaction (TTP223B).
4. Integrate MPU6050 motion sensing.
5. Add buzzer and tactile buttons.
6. Design and verify the LiPo power and charging system.
7. Design the keychain enclosure.

## Safety and reproducibility notes

- The successful prototype was powered from the PC USB port. The LiPo battery has **not** been integrated.
- Do not connect the LiPo directly to ESP32 3V3 or 5V.
- Battery charging, protection, and voltage regulation still need a separately verified design.
- No multimeter continuity or voltage measurements were performed during this setup. The wiring is documented from the observed working prototype and user-confirmed connections; verify board pin labels and breadboard rail continuity when reproducing it.
- Keep the successful firmware as a baseline before making changes.
