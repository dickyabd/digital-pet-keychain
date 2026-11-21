#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ESP32-S3 N16R8
#define TFT_SCLK 12
#define TFT_MOSI 11
#define TFT_RST  8
#define TFT_DC   10
#define TFT_CS   -1

// RGB565 colors
#define BG_COLOR       0x0000
#define EYE_COLOR      0x07FF
#define EYE_OUTLINE    0x18E3
#define EYE_HIGHLIGHT  0xFFFF

Adafruit_ST7789 tft = Adafruit_ST7789(
  &SPI, TFT_CS, TFT_DC, TFT_RST
);

void drawBigEye(int x, int y) {
  // Eye outline
  tft.drawRoundRect(
    x - 2, y - 2, 52, 84, 18, EYE_OUTLINE
  );

  // Main eye shape
  tft.fillRoundRect(
    x, y, 48, 80, 16, EYE_COLOR
  );

  // Light reflections
  tft.fillCircle(
    x + 14, y + 16, 7, EYE_HIGHLIGHT
  );

  tft.fillCircle(
    x + 31, y + 38, 3, EYE_HIGHLIGHT
  );
}

void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // Known-good display configuration
  tft.init(240, 240, SPI_MODE3);
  tft.setRotation(2);

  tft.fillScreen(BG_COLOR);

  // Left eye
  drawBigEye(48, 80);

  // Right eye
  drawBigEye(144, 80);

  Serial.println("BIG EYES DISPLAYED!");
}

void loop() {
}
