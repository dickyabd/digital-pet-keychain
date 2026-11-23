DIGITAL PET – CRUCIAL SKATE INTRO + ORIGINAL LOGO

What's changed in this package:
- The animation previously called Inline Skate (serial command I) is replaced by the new 10-frame Crucial Skate logo supplied by the user.
- The existing Crucial Skate Logo animation (serial command L) is retained unchanged.
- Serial command S plays the new Crucial Skate intro first, then the retained original Crucial Skate logo animation.
- Both animations use 100 ms per frame.
- The background cleanup after full-screen animations is retained.
- Eye animation, touch gestures, roulette, marquee/sparkle and other existing firmware behavior are retained.

Files (keep together):
- DigitalPet_CrucialSkate_10Frame.ino
- inline_skate_frames.h (new 10-frame Crucial Skate intro)
- crucial_logo_frames.h (previous Crucial Skate logo animation, retained)
- pet_mood.h
- partitions.csv

Serial commands:
I = play new Crucial Skate intro (replaces old Inline Skate animation)
L = play original Crucial Skate logo animation
S = play intro, then original logo
P = panic, B = blink, C = center

Board settings used by the previous firmware package: ESP32S3 Dev Module, Flash Size 16MB.
The custom partition issue discussed previously is separate; verify the selected APP partition is large enough before compiling/uploading.

Not compile-tested or hardware-tested in this environment.
