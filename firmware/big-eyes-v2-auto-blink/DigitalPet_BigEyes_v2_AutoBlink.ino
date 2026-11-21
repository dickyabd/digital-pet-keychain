#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ESP32-S3 N16R8 + YY-PS130-V4.0 ST7789 LCD
#define TFT_SCLK 12
#define TFT_MOSI 11
#define TFT_RST  8
#define TFT_DC   10
#define TFT_CS   -1

// RGB565 colors
#define BG_COLOR        0x0000
#define EYE_COLOR       0x07FF
#define EYE_OUTLINE     0x18E3
#define EYE_HIGHLIGHT   0xFFFF

// Eye geometry
const int EYE_LEFT_X = 48;
const int EYE_RIGHT_X = 144;
const int EYE_CENTER_Y = 120;
const int EYE_WIDTH = 48;
const int EYE_OPEN_HEIGHT = 80;
const int EYE_CLOSED_HEIGHT = 8;

// Blink timing (milliseconds)
const unsigned long BLINK_CLOSE_MS = 90;
const unsigned long BLINK_HOLD_MS = 70;
const unsigned long BLINK_OPEN_MS = 90;
const unsigned long FRAME_INTERVAL_MS = 20;

Adafruit_ST7789 tft = Adafruit_ST7789(
  &SPI, TFT_CS, TFT_DC, TFT_RST
);

enum BlinkState {
  EYES_OPEN,
  EYES_CLOSING,
  EYES_CLOSED,
  EYES_OPENING
};

BlinkState blinkState = EYES_OPEN;

unsigned long stateStartedAt = 0;
unsigned long nextBlinkAt = 0;
unsigned long lastFrameAt = 0;

int currentEyeHeight = EYE_OPEN_HEIGHT;

void drawEye(int x, int height) {
  // Keep the eye vertically centered while it closes and opens.
  int top = EYE_CENTER_Y - (height / 2);

  // Clear only this eye's drawing area to reduce full-screen flicker.
  tft.fillRect(x - 4, 70, 60, 100, BG_COLOR);

  uint8_t radius = (height < 32) ? (height / 2) : 16;
  uint8_t outlineRadius = radius + 2;

  // Subtle outline and main eye shape.
  tft.drawRoundRect(
    x - 2, top - 2, 52, height + 4,
    outlineRadius, EYE_OUTLINE
  );

  tft.fillRoundRect(
    x, top, EYE_WIDTH, height,
    radius, EYE_COLOR
  );

  // Keep highlights visible only while the eye is sufficiently open.
  if (height >= 42) {
    tft.fillCircle(
      x + 14, top + 16, 7, EYE_HIGHLIGHT
    );

    tft.fillCircle(
      x + 31, top + 38, 3, EYE_HIGHLIGHT
    );
  }
}

void updateBlink(unsigned long now) {
  switch (blinkState) {
    case EYES_OPEN:
      currentEyeHeight = EYE_OPEN_HEIGHT;

      if ((long)(now - nextBlinkAt) >= 0) {
        blinkState = EYES_CLOSING;
        stateStartedAt = now;
      }
      break;

    case EYES_CLOSING: {
      unsigned long elapsed = now - stateStartedAt;

      if (elapsed >= BLINK_CLOSE_MS) {
        currentEyeHeight = EYE_CLOSED_HEIGHT;
        blinkState = EYES_CLOSED;
        stateStartedAt = now;
      } else {
        currentEyeHeight = EYE_OPEN_HEIGHT -
          ((EYE_OPEN_HEIGHT - EYE_CLOSED_HEIGHT) *
           (int)elapsed / (int)BLINK_CLOSE_MS);
      }
      break;
    }

    case EYES_CLOSED:
      currentEyeHeight = EYE_CLOSED_HEIGHT;

      if (now - stateStartedAt >= BLINK_HOLD_MS) {
        blinkState = EYES_OPENING;
        stateStartedAt = now;
      }
      break;

    case EYES_OPENING: {
      unsigned long elapsed = now - stateStartedAt;

      if (elapsed >= BLINK_OPEN_MS) {
        currentEyeHeight = EYE_OPEN_HEIGHT;
        blinkState = EYES_OPEN;
        nextBlinkAt = now + random(2500, 5500);
      } else {
        currentEyeHeight = EYE_CLOSED_HEIGHT +
          ((EYE_OPEN_HEIGHT - EYE_CLOSED_HEIGHT) *
           (int)elapsed / (int)BLINK_OPEN_MS);
      }
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // Keep the verified display settings from Big Eyes v1.
  tft.init(240, 240, SPI_MODE3);
  tft.setRotation(2);
  tft.fillScreen(BG_COLOR);

  randomSeed((unsigned long)micros());
  nextBlinkAt = millis() + random(1800, 3500);

  Serial.println("BIG EYES V2 - AUTO BLINK");
  Serial.println("Display initialized.");
}

void loop() {
  unsigned long now = millis();

  // Limit redraw attempts to about 50 frames per second.
  // Actual frame rate depends on SPI transfer and drawing time.
  if (now - lastFrameAt < FRAME_INTERVAL_MS) {
    return;
  }
  lastFrameAt = now;

  updateBlink(now);

  // Draw both eyes using the same height for synchronized blinking.
  drawEye(EYE_LEFT_X, currentEyeHeight);
  drawEye(EYE_RIGHT_X, currentEyeHeight);
}
