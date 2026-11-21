# Changelog

## 2026-11-21 — Big Eyes v2.6 Touch Interaction prepared

- Added `firmware/big-eyes-v2-6-touch-interaction/DigitalPet_BigEyes_v2_6_TouchInteraction.ino`.
- Integrated the TTP223B High Level output on GPIO 4.
- Added 35 ms input debounce.
- Added short-touch happy eye reaction.
- Added long-touch (>700 ms) eye color cycling across cyan, yellow, white, and magenta.
- Preserved idle gaze, damped-spring movement, squash-and-stretch, synchronized blink, occasional double blink, canvas renderer, and Serial Monitor debug commands.
- The TTP223B standalone test produced touch and release events. Integrated firmware compile, upload, and visual tests are pending.

## 2026-11-21 — Big Eyes v2.5 Cartoon Personality Engine prepared

- Added `firmware/big-eyes-v2-5-cartoon-personality/DigitalPet_BigEyes_v2_5_CartoonPersonality.ino`.
- Added randomized idle gaze across left, right, up, down, upper-left, and upper-right directions, returning to center between looks.
- Added damped-spring gaze motion with a small natural overshoot.
- Preserved solid cyan eyes without pupils, highlights, inner dots, or a mouth.
- Added direction-dependent squash-and-stretch.
- Retained synchronized blinking and occasional double blinking.
- Added a simulated panic/startle reaction controlled by Serial Monitor command `P`, plus `B` for a forced blink and `C` to center the gaze.
- Project owner tested the sketch and confirmed the Serial Monitor demo commands responded.

## 2026-11-21 — Big Eyes v2.4 Exaggerated Cartoon Gaze prepared

- Added `firmware/big-eyes-v2-4-exaggerated-cartoon-gaze/DigitalPet_BigEyes_v2_4_ExaggeratedCartoonGaze.ino`.
- Reduced base eye size from 96 × 150 px to 76 × 112 px.
- Expanded horizontal gaze travel from ±7 px to ±24 px and vertical travel from ±5 px to ±16 px.
- Added direction-dependent squash-and-stretch for horizontal, vertical, and diagonal looks.
- Kept solid eyes only, with no pupils, highlights, or inner dots.
- Preserved canvas rendering, synchronized blink, occasional double blink, and known-good ST7789 / SPI Mode 3 configuration.
- Physical visual review completed by project owner.

## 2026-11-21 — Big Eyes v2.3 Solid Cartoon Eyes prepared

- Added `firmware/big-eyes-v2-3-solid-cartoon-eyes/DigitalPet_BigEyes_v2_3_SolidCartoonEyes.ino`.
- Removed pupils, highlights, and inner dots to match the requested design.
- Enlarged the solid cyan eye shapes to 96 × 150 pixels each.
- Expanded the off-screen canvas to 240 × 174 pixels (83,520 bytes).
- Added cartoon-like gaze motion and retained synchronized blinking.

## 2026-11-21 — Big Eyes v2.2 Gaze + Double Blink prepared

- Added `firmware/big-eyes-v2-2-gaze-double-blink/DigitalPet_BigEyes_v2_2_GazeDoubleBlink.ino`.
- Added automatic gaze sequence and occasional double blink.
- This experimental iteration uses pupils; the design direction subsequently changed to remove pupils.

## 2026-11-21 — Big Eyes v2.1 Smooth Blink visually verified

- Added `firmware/big-eyes-v2-1-smooth-blink/DigitalPet_BigEyes_v2_1_SmoothBlink.ino`.
- Added off-screen rendering with a 160 × 100, 16-bit GFX canvas for both eyes.
- Added synchronized close, hold, and open phases.
- Project owner reported that the latest video no longer showed the visible glitch seen in v2.

## 2026-11-21 — Big Eyes v2 Auto Blink firmware prepared

- Added `firmware/big-eyes-v2-auto-blink/DigitalPet_BigEyes_v2_AutoBlink.ino`.
- Added non-blocking blink state machine using `millis()`.
- First visual test showed visible drawing artifacts; retained as a historical iteration.

## 2026-11-21 — Big Eyes v1 milestone

- Added `firmware/big-eyes-v1/DigitalPet_BigEyes_v1.ino`.
- Displayed two large cyan eyes with white highlights using the verified ESP32-S3 and ST7789 wiring.

## 2026-11-21 — Initial hardware milestone

- Set up ESP32-S3 N16R8 with Arduino IDE 2.3.11.
- Installed Espressif ESP32 Arduino core 3.3.7.
- Identified LCD module as YY-PS130-V4.0, 240 × 240, ST7789, SPI.
- Confirmed SPI Mode 3 produces visible LCD graphics.
- Displayed DIGITAL PET text and a yellow circle.
- Documented pin mapping, power rails, and troubleshooting.
- LiPo battery integration remains pending.
