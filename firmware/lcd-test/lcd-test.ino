#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// LCD pins on ESP32-S3 N16R8
#define TFT_SCLK 12
#define TFT_MOSI 11
#define TFT_RST  8
#define TFT_DC   10
#define TFT_CS   -1  // LCD module has no exposed CS pin

Adafruit_ST7789 tft = Adafruit_ST7789(
  &SPI, TFT_CS, TFT_DC, TFT_RST
);

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Digital Pet LCD Test");

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // SPI_MODE3 is the mode confirmed to produce visible output
  // on the YY-PS130-V4.0 module used in this prototype.
  tft.init(240, 240, SPI_MODE3);
  tft.setRotation(0);

  Serial.println("LCD initialized");

  // Full-screen color checks
  tft.fillScreen(ST77XX_RED);
  delay(1500);

  tft.fillScreen(ST77XX_GREEN);
  delay(1500);

  tft.fillScreen(ST77XX_BLUE);
  delay(1500);

  // First Digital Pet display
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);

  tft.setCursor(35, 90);
  tft.println("DIGITAL");

  tft.setCursor(55, 120);
  tft.println("PET!");

  tft.drawCircle(120, 55, 20, ST77XX_YELLOW);

  Serial.println("Test complete!");
}

void loop() {
}
