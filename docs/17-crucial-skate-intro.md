# Crucial Skate Intro + Original Logo

## Latest firmware package

The current package is:

`firmware/DigitalPet_CrucialSkate_Intro10F_KeepOriginalLogo/`

Main sketch: `DigitalPet_CrucialSkate_Intro10F_KeepOriginalLogo.ino`

Keep the sketch and its supporting files together when opening or compiling the project in Arduino IDE.

## Animation behavior

| Serial command | Action |
|---|---|
| `I` | Play the new 10-frame Crucial Skate intro |
| `L` | Play the original Crucial Skate logo animation |
| `S` | Play the new intro, then the original logo animation |
| `P` | Panic expression |
| `B` | Blink |
| `C` | Center gaze |

Both full-screen animation sequences use 100 ms per frame. The new intro replaces the previous Inline Skate animation on command `I`; the original logo sequence remains available on command `L`.

## Files

- `DigitalPet_CrucialSkate_Intro10F_KeepOriginalLogo.ino` — main firmware sketch
- `inline_skate_frames.h` — 10-frame Crucial Skate intro data
- `crucial_logo_frames.h` — retained original Crucial Skate logo animation data
- `pet_mood.h` — pet mood definitions
- `partitions.csv` — partition table supplied with this package

## Preserved behavior

The package retains the existing eye animation, touch interactions, eye-style roulette, DINARA marquee and sparkle effects, and background cleanup after full-screen animations.

## Board configuration

The package targets the ESP32-S3 Dev Module with 16 MB flash. Use the partition configuration supplied with the package and ensure the selected application partition is large enough for the firmware and animation data.

## Validation status

The package files are present on the repository's `main` branch. Repository presence does not by itself confirm a successful compile or physical test; verify compilation, upload, and animation behavior on the target device.
