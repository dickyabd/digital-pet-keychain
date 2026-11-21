# Digital Pet Keychain

An ESP32-S3-based interactive digital pet keychain. This repository records the hardware setup, LCD bring-up, firmware experiments, and future sensor and battery integration.

## Current status

**Milestone 01 — LCD bring-up: working**

The prototype successfully displayed the text **DIGITAL PET** and a yellow circle on the TFT LCD.

**Milestone 02 — Big Eyes v1: working**

The prototype successfully displayed a pair of large cyan eyes with white highlights on the ST7789 display. This is the static eye baseline.

**Milestone 03 — Big Eyes v2 Auto Blink: superseded prototype**

The first non-blocking blinking version was tested and showed visible drawing artifacts. It is retained as a historical iteration.

**Milestone 04 — Big Eyes v2.1 Smooth Blink: visual test passed**

The updated sketch draws both eyes into one 16-bit off-screen canvas and transfers the combined image to the LCD. The project owner reported that the latest video no longer showed the earlier visible glitch. This is a visual prototype test, not a measured FPS or long-duration reliability test.

**Milestone 05 — Big Eyes v2.2 Gaze + Double Blink: experimental**

This iteration adds gaze directions with pupils and occasional double blinks. It is retained as an experimental version; the design direction subsequently changed to remove pupils.

**Milestone 06 — Big Eyes v2.3 Solid Cartoon Eyes: superseded design iteration**

This candidate removed pupils and enlarged the solid cyan eye shapes. The project owner reported that the eyes were too large and their horizontal movement was not exaggerated enough.

**Milestone 07 — Big Eyes v2.4 Exaggerated Cartoon Gaze: firmware prepared, hardware test pending**

This candidate reduces base eye dimensions, increases horizontal and vertical gaze travel, and adds direction-dependent squash-and-stretch. It retains solid eyes only, without pupils or highlights. Physical compile/upload and visual tests are still required.

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

The LCD's SDA label is the SPI data input (MOSI) for this module; it is not being used as I²C SDA. The module has no exposed CS pin; the sketches use `TFT_CS = -1`.

## Breadboard power distribution

| ESP32-S3 | Breadboard rail | Jumper color |
|---|---|---|
| 3V3 | Red (+) rail | Black |
| GND | Blue (-) rail | White |

The LCD VCC connects to the 3.3 V rail and LCD GND to the GND rail. Breadboard power rails may be split into separate segments. Do not assume every section is electrically continuous.

## Firmware

- [LCD test baseline](firmware/lcd-test/lcd-test.ino)
- [Big Eyes v1 — static cyan eyes](firmware/big-eyes-v1/DigitalPet_BigEyes_v1.ino)
- [Big Eyes v2 — first auto-blink iteration (visual artifacts)](firmware/big-eyes-v2-auto-blink/DigitalPet_BigEyes_v2_AutoBlink.ino)
- [Big Eyes v2.1 — smooth synchronized auto-blink](firmware/big-eyes-v2-1-smooth-blink/DigitalPet_BigEyes_v2_1_SmoothBlink.ino)
- [Big Eyes v2.2 — gaze directions + occasional double blink (pupils; experimental)](firmware/big-eyes-v2-2-gaze-double-blink/DigitalPet_BigEyes_v2_2_GazeDoubleBlink.ino)
- [Big Eyes v2.3 — large solid cartoon eyes without pupils (superseded)](firmware/big-eyes-v2-3-solid-cartoon-eyes/DigitalPet_BigEyes_v2_3_SolidCartoonEyes.ino)
- [Big Eyes v2.4 — smaller solid eyes, exaggerated gaze, squash-and-stretch (test pending)](firmware/big-eyes-v2-4-exaggerated-cartoon-gaze/DigitalPet_BigEyes_v2_4_ExaggeratedCartoonGaze.ino)

The working display initialization is:

```cpp
SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
tft.init(240, 240, SPI_MODE3);
tft.setRotation(2);
```

**SPI Mode 3** and rotation 2 are retained from the visible Big Eyes v1 prototype.

The firmware uses Adafruit GFX / Adafruit ST7789. The canvas-based versions use a 16-bit `GFXcanvas16` to render both eyes together before transferring the bitmap to the LCD. v2.4 uses a 240 × 174 pixel canvas (83,520 bytes). No TFT_eSPI or GC9A01 is used by these sketches.

## Documentation

- [Hardware setup and wiring](docs/01-hardware-setup.md)
- [Troubleshooting notes](docs/02-troubleshooting.md)
- [Big Eyes v1 notes](docs/03-big-eyes-v1.md)
- [Big Eyes v2 Auto Blink notes](docs/04-big-eyes-v2-auto-blink.md)
- [Big Eyes v2.1 Smooth Blink notes](docs/05-big-eyes-v2-1-smooth-blink.md)
- [Big Eyes v2.2 Gaze + Double Blink notes](docs/06-big-eyes-v2-2-gaze-double-blink.md)
- [Big Eyes v2.3 Solid Cartoon Eyes notes](docs/07-big-eyes-v2-3-solid-cartoon-eyes.md)
- [Big Eyes v2.4 Exaggerated Cartoon Gaze notes](docs/08-big-eyes-v2-4-exaggerated-cartoon-gaze.md)
- [Changelog](CHANGELOG.md)

## Next milestones

1. Verify and upload Big Eyes v2.4; tune cartoon gaze and eye dimensions.
2. Add touch interaction (TTP223B).
3. Integrate MPU6050 motion sensing.
4. Add buzzer and tactile buttons.
5. Design and verify the LiPo power and charging system.
6. Design the keychain enclosure.

## Safety and reproducibility notes

- The successful prototype was powered from the PC USB port. The LiPo battery has **not** been integrated.
- Do not connect the LiPo directly to ESP32 3V3 or 5V.
- Battery charging, protection, and voltage regulation still need a separately verified design.
- No multimeter continuity or voltage measurements were performed during this setup. The wiring is documented from the observed working prototype and user-confirmed connections; verify board pin labels and breadboard rail continuity when reproducing it.
- Keep the successful firmware as a baseline before making changes.
