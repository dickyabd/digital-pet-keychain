Digital Pet v3.3.5 - Inline Skate Background Match

Based on v3.3.4 Slower Playback.

Change:
- Recolored the connected outer background pixels in all 15 Inline Skate RGB565 frames
  to RGB565 0x1063, the 16-bit representation of #100E18.
- The recolor uses edge-connected dark background regions, preserving enclosed dark
  details and outlines in the skate artwork.
- Crucial Skate Logo frames are unchanged.

Playback:
- Inline Skate: 100 ms/frame (~10 FPS target)
- Crucial Skate Logo: 180 ms/frame (~5.6 FPS target)

Preserved:
- SPI 40 MHz
- RAM animation buffer + memcpy_P + writePixels
- 240x240 RGB565 assets
- pet_mood.h
- Existing Digital Pet features

Open DigitalPet_v3_3_5_Inline_BG_Match.ino. Keep all .h files in the same sketch folder.
