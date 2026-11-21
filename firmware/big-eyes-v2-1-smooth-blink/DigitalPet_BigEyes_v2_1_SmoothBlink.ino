#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ESP32-S3 N16R8 + YY-PS130-V4.0
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

// LCD canvas area
#define CANVAS_X 40
#define CANVAS_Y 70
#define CANVAS_W 160
#define CANVAS_H 100

// Eye geometry (canvas coordinates)
const int LEFT_EYE_X = 8;
const int RIGHT_EYE_X = 104;
const int EYE_CENTER_Y = 50;

const int EYE_WIDTH = 48;
const int EYE_OPEN_HEIGHT = 80;
const int EYE_CLOSED_HEIGHT = 8;

// Animation timing
const unsigned long CLOSE_MS = 160;
const unsigned long HOLD_MS = 70;
const unsigned long OPEN_MS = 160;
const unsigned long FRAME_INTERVAL_MS = 16;

Adafruit_ST7789 tft = Adafruit_ST7789(
  &SPI, TFT_CS, TFT_DC, TFT_RST
);

// Allocate the canvas in setup(), after startup.
GFXcanvas16 *eyeCanvas = nullptr;

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
int lastRenderedHeight = -1;

// Draw one eye into the memory canvas.
void drawEyeOnCanvas(int x, int height) {
  int top = EYE_CENTER_Y - height / 2;

  int radius = (height < 32)
                 ? height / 2
                 : 16;

  // Main eye
  eyeCanvas->fillRoundRect(
    x, top,
    EYE_WIDTH, height,
    radius, EYE_COLOR
  );

  // Eye outline
  eyeCanvas->drawRoundRect(
    x - 2, top - 2,
    52, height + 4,
    radius + 2, EYE_OUTLINE
  );

  // Highlights only while the eye is sufficiently open
  if (height >= 42) {
    eyeCanvas->fillCircle(
      x + 14, top + 16, 7,
      EYE_HIGHLIGHT
    );

    eyeCanvas->fillCircle(
      x + 31, top + 38, 3,
      EYE_HIGHLIGHT
    );
  }
}

// Render both eyes into one canvas, then transfer it to the LCD.
void renderEyes(int height) {
  eyeCanvas->fillScreen(BG_COLOR);

  drawEyeOnCanvas(LEFT_EYE_X, height);
  drawEyeOnCanvas(RIGHT_EYE_X, height);

  tft.drawRGBBitmap(
    CANVAS_X,
    CANVAS_Y,
    eyeCanvas->getBuffer(),
    CANVAS_W,
    CANVAS_H
  );
}

// Synchronized blink animation state machine.
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

      if (elapsed >= CLOSE_MS) {
        currentEyeHeight = EYE_CLOSED_HEIGHT;
        blinkState = EYES_CLOSED;
        stateStartedAt = now;
      } else {
        currentEyeHeight =
          EYE_OPEN_HEIGHT -
          ((EYE_OPEN_HEIGHT - EYE_CLOSED_HEIGHT) *
           (int)elapsed / (int)CLOSE_MS);
      }
      break;
    }

    case EYES_CLOSED:
      currentEyeHeight = EYE_CLOSED_HEIGHT;

      if (now - stateStartedAt >= HOLD_MS) {
        blinkState = EYES_OPENING;
        stateStartedAt = now;
      }
      break;

    case EYES_OPENING: {
      unsigned long elapsed = now - stateStartedAt;

      if (elapsed >= OPEN_MS) {
        currentEyeHeight = EYE_OPEN_HEIGHT;
        blinkState = EYES_OPEN;
        nextBlinkAt = now + random(2500, 5500);
      } else {
        currentEyeHeight =
          EYE_CLOSED_HEIGHT +
          ((EYE_OPEN_HEIGHT - EYE_CLOSED_HEIGHT) *
           (int)elapsed / (int)OPEN_MS);
      }
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // Keep the verified ST7789 settings.
  tft.init(240, 240, SPI_MODE3);
  tft.setRotation(2);
  tft.fillScreen(BG_COLOR);

  // 160 x 100 x 2 bytes = 32,000 bytes.
  eyeCanvas = new GFXcanvas16(CANVAS_W, CANVAS_H);

  if (eyeCanvas == nullptr ||
      eyeCanvas->getBuffer() == nullptr) {
    Serial.println("ERROR: Canvas allocation failed!");

    while (true) {
      delay(1000);
    }
  }

  randomSeed((unsigned long)micros());
  nextBlinkAt = millis() + random(1800, 3500);

  Serial.println("BIG EYES V2.1 - SMOOTH BLINK");
  Serial.println("Canvas allocated successfully.");

  renderEyes(EYE_OPEN_HEIGHT);
  lastRenderedHeight = EYE_OPEN_HEIGHT;
}

void loop() {
  unsigned long now = millis();

  if (now - lastFrameAt < FRAME_INTERVAL_MS) {
    return;
  }

  lastFrameAt = now;

  updateBlink(now);

  // Transfer only when the eye height changes.
  if (currentEyeHeight != lastRenderedHeight) {
    renderEyes(currentEyeHeight);
    lastRenderedHeight = currentEyeHeight;
  }
}
