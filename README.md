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

The updated sketch draws both eyes into one 16-bit off-screen canvas and transfers the combined image to the LCD. The project owner reported that the latest video no longer showed the earlier visible glitch.

**Milestone 05 — Big Eyes v2.2 Gaze + Double Blink: experimental**

This iteration adds gaze directions with pupils and occasional double blinks. It is retained as an experimental version; the design direction subsequently changed to remove pupils.

**Milestone 06 — Big Eyes v2.3 Solid Cartoon Eyes: superseded design iteration**

This candidate removed pupils and enlarged the solid cyan eye shapes. The owner reported that the eyes were too large and their horizontal movement was not exaggerated enough.

**Milestone 07 — Big Eyes v2.4 Exaggerated Cartoon Gaze: visual review completed**

The owner supplied a video of v2.4. The eyes were smaller than v2.3 and the gaze movement was more visible. Further cartoon expression and personality behavior were requested.

**Milestone 08 — Big Eyes v2.5 Cartoon Personality Engine: test reported successful**

The owner tested the automatic personality behavior and confirmed the Serial Monitor panic, blink, and center-gaze commands responded.

**Milestone 09 — TTP223B standalone test: working**

The TTP223B v1.0 High Level module was tested separately on GPIO 4. Serial Monitor showed touch and release events.

**Milestone 10 — Big Eyes v2.6 Touch Interaction: test reported successful**

The owner tested the integrated touch behavior and reported that it looked good. Short touch triggers a happy reaction; long touch changes the eye color.

**Milestone 11 — Big Eyes v2.7 Five-Tap Fear Reaction: firmware prepared, test pending**

This candidate adds a fear reaction when five short taps occur within three seconds. The eyes shrink, move closer together, and tremble briefly. Physical Verify/Upload and behavior testing are pending.

**Milestone 12 — Big Eyes v2.8 DINARA Marquee: tested on device**

The project owner reports that the v2.8 firmware was tested successfully. A continuous touch hold for three seconds launches a one-pass right-to-left DINARA marquee. The name uses a pink FreeSans Bold Oblique font with a white outline and pulsing pink hearts outlined in white. After the whole word exits the left edge, the pet face returns automatically. A 700 ms to under-three-second hold, when released, changes the eye color.

**Milestone 13 — Big Eyes v2.9 Permanent Pupils: firmware prepared, device test pending**

This experimental iteration adds dark oval pupils that remain visible during normal expressions and move within the colored eye shapes according to gaze direction. Pupils are naturally hidden while the eyes are almost fully closed during a blink. All v2.8 touch gestures and DINARA marquee behavior are retained. Physical testing is pending.

**Milestone 14 — Big Eyes v3.0 Eye Style Roulette: firmware prepared, device test pending**

This version adds five eye presets: Normal (white sclera / navy oval pupil), Cute (purple sclera / small green pupil), Star (lime sclera / yellow star pupil), Heart (white sclera / pink heart pupil), and Fire (amber sclera / flame pupil). A medium touch hold followed by release starts a roughly 2.2-second roulette. Styles change quickly at first, then progressively more slowly before a random style is selected. The selected style remains active until the next roulette or restart. Existing gaze, blink, happy, fear, and DINARA behavior is retained. Physical testing is pending.

**Milestone 15 — Big Eyes v3.1 Visible Eye Carousel: firmware prepared, device test pending**

This revision makes the eye-style roulette begin immediately when a touch hold reaches 700 ms, rather than waiting for release. The five styles visibly cycle and slow down over roughly 2.2 seconds. A bottom-screen ROLLING label and progress bar make the animation state explicit, followed by a short selected-style label. Releasing touch does not cancel the roulette. A continuous three-second hold still takes priority and launches the DINARA marquee. Physical testing is pending.

**Milestone 16 — Big Eyes v3.2 Horizontal Sliding Carousel: firmware prepared, device test pending**

This revision changes the eye-style roulette from in-place style replacement to a full-frame horizontal slide: the outgoing pair of eyes moves left while the incoming style enters from the right. The same transition is used when DINARA enters after the three-second hold and when the eyes return after the marquee exits. It retains the five eye styles, slowing roulette, visible status/progress indicator, touch gesture mapping, and Adafruit ST7789 wiring. Physical testing is pending.

## Hardware baseline

| Item | Value |
|---|---|
| MCU board | ESP32-S3 N16R8 development board |
| LCD PCB | YY-PS130-V4.0 |
| LCD controller | ST7789 |
| Resolution | 240 × 240 pixels |
| Interface | SPI |
| Touch sensor | TTP223B v1.0, High Level output |
| Touch signal | GPIO 4 |
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

## Touch sensor wiring

| TTP223B pin | ESP32-S3 connection |
|---|---|
| SIG | GPIO 4 |
| VCC | 3.3 V |
| GND | GND |

The module is marked High Level. The standalone test produced `TOUCH DETECTED!` and `TOUCH RELEASED!` events in Serial Monitor. The integrated touch behavior in v2.6 was reported as working.

## Breadboard power distribution

| ESP32-S3 | Breadboard rail | Jumper color |
|---|---|---|
| 3V3 | Red (+) rail | Black |
| GND | Blue (-) rail | White |

The LCD VCC and TTP223B VCC connect to 3.3 V. LCD GND and TTP223B GND share common ground. Breadboard power rails may be split into separate segments. Do not assume every section is electrically continuous.

## Firmware

- [LCD test baseline](firmware/lcd-test/lcd-test.ino)
- [Big Eyes v1 — static cyan eyes](firmware/big-eyes-v1/DigitalPet_BigEyes_v1.ino)
- [Big Eyes v2 — first auto-blink iteration (visual artifacts)](firmware/big-eyes-v2-auto-blink/DigitalPet_BigEyes_v2_AutoBlink.ino)
- [Big Eyes v2.1 — smooth synchronized auto-blink](firmware/big-eyes-v2-1-smooth-blink/DigitalPet_BigEyes_v2_1_SmoothBlink.ino)
- [Big Eyes v2.2 — gaze directions + occasional double blink (pupils; experimental)](firmware/big-eyes-v2-2-gaze-double-blink/DigitalPet_BigEyes_v2_2_GazeDoubleBlink.ino)
- [Big Eyes v2.3 — large solid cartoon eyes without pupils (superseded)](firmware/big-eyes-v2-3-solid-cartoon-eyes/DigitalPet_BigEyes_v2_3_SolidCartoonEyes.ino)
- [Big Eyes v2.4 — smaller solid eyes, exaggerated gaze, squash-and-stretch](firmware/big-eyes-v2-4-exaggerated-cartoon-gaze/DigitalPet_BigEyes_v2_4_ExaggeratedCartoonGaze.ino)
- [Big Eyes v2.5 — Cartoon Personality Engine](firmware/big-eyes-v2-5-cartoon-personality/DigitalPet_BigEyes_v2_5_CartoonPersonality.ino)
- [Big Eyes v2.6 — Touch Interaction (tested)](firmware/big-eyes-v2-6-touch-interaction/DigitalPet_BigEyes_v2_6_TouchInteraction.ino)
- [Big Eyes v2.7 — Five-Tap Fear Reaction (test pending)](firmware/big-eyes-v2-7-five-tap-fear/DigitalPet_BigEyes_v2_7_FiveTapFear.ino)
- [Big Eyes v2.8 — DINARA pink marquee with popping hearts (tested)](firmware/big-eyes-v2-8-dinara-marquee/DigitalPet_BigEyes_v2_8_DinaraMarquee.ino)
- [Big Eyes v2.9 — permanent moving pupils (device test pending)](firmware/big-eyes-v2-9-permanent-pupils/DigitalPet_BigEyes_v2_9_PermanentPupils.ino)
- [Big Eyes v3.0 — five eye styles + slowing roulette (device test pending)](firmware/big-eyes-v3-0-eye-style-roulette/DigitalPet_BigEyes_v3_0_EyeStyleRoulette.ino)
- [Big Eyes v3.1 — visible hold-triggered eye carousel (device test pending)](firmware/big-eyes-v3-1-visible-eye-carousel/DigitalPet_BigEyes_v3_1_VisibleEyeCarousel.ino)
- [Big Eyes v3.2 — horizontal sliding carousel and DINARA transitions (device test pending)](firmware/big-eyes-v3-2-horizontal-sliding-carousel/DigitalPet_BigEyes_v3_2_HorizontalSlidingCarousel.ino)
- [TTP223B standalone sensor test](firmware/touch-test/DigitalPet_TouchTest_v1.ino)

The working LCD initialization is:

```cpp
SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
tft.init(240, 240, SPI_MODE3);
tft.setRotation(2);
```

Canvas-based versions use a 16-bit `GFXcanvas16` to render both eyes together before transferring the bitmap to the LCD. v2.8 uses a 240 × 174 pixel canvas (83,520 bytes).

## Documentation

- [Hardware setup and wiring](docs/01-hardware-setup.md)
- [Troubleshooting notes](docs/02-troubleshooting.md)
- [Big Eyes v1 notes](docs/03-big-eyes-v1.md)
- [Big Eyes v2 Auto Blink notes](docs/04-big-eyes-v2-auto-blink.md)
- [Big Eyes v2.1 Smooth Blink notes](docs/05-big-eyes-v2-1-smooth-blink.md)
- [Big Eyes v2.2 Gaze + Double Blink notes](docs/06-big-eyes-v2-2-gaze-double-blink.md)
- [Big Eyes v2.3 Solid Cartoon Eyes notes](docs/07-big-eyes-v2-3-solid-cartoon-eyes.md)
- [Big Eyes v2.4 Exaggerated Cartoon Gaze notes](docs/08-big-eyes-v2-4-exaggerated-cartoon-gaze.md)
- [Big Eyes v2.5 Cartoon Personality Engine notes](docs/09-big-eyes-v2-5-cartoon-personality.md)
- [Big Eyes v2.6 Touch Interaction notes](docs/10-big-eyes-v2-6-touch-interaction.md)
- [Big Eyes v2.7 Five-Tap Fear Reaction notes](docs/11-big-eyes-v2-7-five-tap-fear.md)
- [Big Eyes v2.8 DINARA Marquee notes](docs/12-big-eyes-v2-8-dinara-marquee.md)
- [Big Eyes v2.9 Permanent Pupils notes](docs/13-big-eyes-v2-9-permanent-pupils.md)
- [Big Eyes v3.0 Eye Style Roulette notes](docs/14-big-eyes-v3-0-eye-style-roulette.md)
- [Big Eyes v3.1 Visible Eye Carousel notes](docs/15-big-eyes-v3-1-visible-eye-carousel.md)
- [Big Eyes v3.2 Horizontal Sliding Carousel notes](docs/16-big-eyes-v3-2-horizontal-sliding-carousel.md)
- [Changelog](CHANGELOG.md)

## Next milestones

1. Compile, upload, and visually test v3.2 horizontal style-to-style slides, DINARA entrance, and return-to-eyes slide.
2. Confirm five-tap fear, roulette slowdown, and touch gesture priority remain correct in v3.2.
3. Integrate MPU6050 after selecting non-conflicting I2C pins.
4. Add buzzer and tactile buttons.
5. Design and verify the LiPo power and charging system.
6. Design the keychain enclosure.

## Safety and reproducibility notes

- The successful prototype is powered from the PC USB port. The LiPo battery has **not** been integrated.
- Do not connect the LiPo directly to ESP32 3V3 or 5V.
- Battery charging, protection, and voltage regulation still need a separately verified design.
- No multimeter continuity or voltage measurements were performed during the initial wiring setup. Verify board pin labels and breadboard rail continuity when reproducing it.
- Disconnect USB power before changing jumper wiring.
- Keep known-good firmware versions as baselines before making changes.
