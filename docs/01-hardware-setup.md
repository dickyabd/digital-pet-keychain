# Hardware Setup — ESP32-S3 + ST7789

## Goal

Bring up the 1.54-inch 240 × 240 TFT display on the ESP32-S3 N16R8 and verify that graphics can be drawn.

## Parts used

- ESP32-S3 N16R8 development board
- YY-PS130-V4.0 LCD module
- ST7789 display controller
- 400-point solderless breadboard
- Jumper wires
- USB cable to the PC

The LCD module's rear marking identifies the PCB as YY-PS130-V4.0, with 240 × 240 pixels, ST7789 driver, and SPI interface.

## LCD header order

Viewed from the front of the display with the pin labels along the top edge, the pins are:

1. GND
2. VCC
3. SCL
4. SDA
5. RES
6. DC
7. BLK

## Wiring

### ESP32-S3 to breadboard power rails

| ESP32-S3 pin | Breadboard rail | Wire |
|---|---|---|
| 3V3 | Red (+) | Black |
| GND | Blue (-) | White |

### LCD to ESP32-S3 / power rails

| LCD pin | Connection | Wire in successful rebuild |
|---|---|---|
| GND | Blue (-) GND rail | Purple |
| VCC | Red (+) 3.3 V rail | Blue |
| SCL | GPIO 12 | Green / teal |
| SDA / MOSI | GPIO 11 | Yellow |
| RES | GPIO 8 | Orange |
| DC | GPIO 10 | Red |
| BLK | Left disconnected | Brown (not connected) |

There is no separate CS pin on the LCD header. The firmware sets CS to -1.

**Important:** SCL means SPI clock and SDA is used as SPI MOSI on this module. Do not wire this module as an I²C display.

## Firmware / SPI settings

- SPI SCLK: GPIO 12
- SPI MOSI: GPIO 11
- SPI MISO: unused
- CS: -1 (no exposed CS)
- Reset: GPIO 8
- D/C: GPIO 10
- SPI mode: SPI_MODE3
- Display initialization: `tft.init(240, 240, SPI_MODE3)`
- Rotation used during the successful test: `tft.setRotation(0)`

The successful test used the Adafruit ST7789 library and its default SPI clock configuration.

## Power

During the successful bring-up, the ESP32-S3 was powered over USB from the PC. No LiPo battery was connected.

The LCD VCC was connected to the 3.3 V rail. The BLK pin was left disconnected in the successful wiring. Do not substitute 5 V for the LCD VCC.

## Breadboard notes

A typical solderless breadboard connects each five-hole group on one side of the center trench. The two sides are separated. Power rails may also be segmented. Check the actual board layout rather than assuming the whole red or blue rail is continuous.

## Verification achieved

The LCD visibly displayed:

- A dark background
- White text: DIGITAL PET
- A yellow outlined circle

The test sketch also sent red, green, and blue full-screen fill commands. The visible text and circle are the confirmed final image result.
