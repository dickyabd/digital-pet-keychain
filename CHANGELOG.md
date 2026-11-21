# Changelog

## 2026-11-22 — Big Eyes v2.7 Five-Tap Fear Reaction prepared

- Added `firmware/big-eyes-v2-7-five-tap-fear/DigitalPet_BigEyes_v2_7_FiveTapFear.ino`.
- Added a rolling three-second tap window; five short taps trigger a fear reaction.
- Added frightened eye styling: smaller eyes, reduced spacing, and rapid micro-tremble for about 1.4 seconds.
- Preserved happy reaction for taps one through four.
- Preserved long-press color cycling; long presses do not count toward the five-tap sequence.
- Preserved v2.6 touch debounce, idle gaze, blink/double blink, canvas rendering, and Serial Monitor commands.
- Added v2.7 documentation. Verify/Compile, Upload, and physical testing are pending.

## 2026-11-22 — Big Eyes v2.6 Touch Interaction tested

- Added `firmware/big-eyes-v2-6-touch-interaction/DigitalPet_BigEyes_v2_6_TouchInteraction.ino`.
- Integrated the TTP223B High Level output on GPIO 4.
- Added 35 ms input debounce.
- Added short-touch happy eye reaction.
- Added long-touch (>700 ms) eye color cycling across cyan, yellow, white, and magenta.
- Project owner reported integrated touch behavior looked good.

## 2026-11-22 — Big Eyes v2.5 Cartoon Personality Engine tested

- Added randomized idle gaze across six directions, damped-spring gaze movement, squash-and-stretch, blinking, double blinking, and simulated panic.
- Project owner tested the Serial Monitor demo commands and reported the behavior looked good.

## 2026-11-21 — Big Eyes v2.4 Exaggerated Cartoon Gaze prepared

- Reduced base eye size from 96 × 150 px to 76 × 112 px.
- Expanded horizontal gaze travel from ±7 px to ±24 px and vertical travel from ±5 px to ±16 px.
- Added direction-dependent squash-and-stretch.
- Kept solid eyes without pupils, highlights, or inner dots.

## 2026-11-21 — Big Eyes v2.3 Solid Cartoon Eyes prepared

- Removed pupils, highlights, and inner dots.
- Enlarged the solid cyan eye shapes and added cartoon-like gaze motion.

## 2026-11-21 — Big Eyes v2.2 Gaze + Double Blink prepared

- Added gaze direction sequence and occasional double blink. This experimental version includes pupils.

## 2026-11-21 — Big Eyes v2.1 Smooth Blink visually verified

- Added 160 × 100 off-screen 16-bit canvas rendering for both eyes.
- Project owner reported that the earlier visible glitch no longer appeared.

## 2026-11-21 — Big Eyes v2 Auto Blink prepared

- Added non-blocking blink state machine. First visual test showed drawing artifacts.

## 2026-11-21 — Big Eyes v1 milestone

- Displayed two large cyan eyes with white highlights using verified ESP32-S3 and ST7789 wiring.

## 2026-11-21 — Initial hardware milestone

- Set up ESP32-S3 N16R8 with Arduino IDE 2.3.11.
- Installed Espressif ESP32 Arduino core 3.3.7.
- Identified YY-PS130-V4.0 LCD as 240 × 240 ST7789 SPI.
- Confirmed SPI Mode 3 produces visible LCD graphics.
- Displayed DIGITAL PET text and a yellow circle.
- LiPo battery integration remains pending.
