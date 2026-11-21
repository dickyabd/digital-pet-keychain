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
#define BG_COLOR       0x0000
#define EYE_COLOR      0x07FF
#define EYE_OUTLINE    0x18E3

// Full-width canvas band: 240 x 174 pixels, RGB565.
#define CANVAS_X 0
#define CANVAS_Y 33
#define CANVAS_W 240
#define CANVAS_H 174

// Base eye geometry
const int LEFT_EYE_CENTER_X = 72;
const int RIGHT_EYE_CENTER_X = 168;
const int EYE_CENTER_Y = 87;

const int BASE_EYE_WIDTH = 76;
const int BASE_EYE_HEIGHT = 112;
const int EYE_CLOSED_HEIGHT = 8;

// Blink timing
const unsigned long CLOSE_MS = 160;
const unsigned long HOLD_MS = 80;
const unsigned long OPEN_MS = 160;

// Gaze timing and movement
const unsigned long FRAME_INTERVAL_MS = 20;
const unsigned long GAZE_MIN_DWELL_MS = 850;
const unsigned long GAZE_MAX_DWELL_MS = 1350;
const float GAZE_SMOOTHING = 0.26f;

Adafruit_ST7789 tft = Adafruit_ST7789(
  &SPI, TFT_CS, TFT_DC, TFT_RST
);

GFXcanvas16 *eyeCanvas = nullptr;

enum BlinkState {
  EYES_OPEN,
  EYES_CLOSING,
  EYES_CLOSED,
  EYES_OPENING
};

struct GazePoint {
  int8_t x;
  int8_t y;
};

// Bigger offsets make the gaze direction clearly visible.
// Each direction returns to center before the next one.
const GazePoint gazeSequence[] = {
  {  0,   0 },  // Center
  { -24,  0 },  // Far left
  {  0,   0 },  // Center
  {  24,  0 },  // Far right
  {  0,   0 },  // Center
  {  0, -16 },  // Up
  {  0,   0 },  // Center
  {  0,  16 },  // Down
  {  0,   0 },  // Center
  { -22, -15 }, // Upper-left
  {  0,   0 },  // Center
  {  22, -15 }, // Upper-right
  {  0,   0 }   // Center, repeat
};

const uint8_t GAZE_COUNT =
  sizeof(gazeSequence) / sizeof(gazeSequence[0]);

BlinkState blinkState = EYES_OPEN;

unsigned long stateStartedAt = 0;
unsigned long nextBlinkAt = 0;
unsigned long nextGazeAt = 0;
unsigned long lastFrameAt = 0;

bool secondBlinkQueued = false;
bool isSecondBlink = false;

uint8_t gazeIndex = 0;

float currentGazeX = 0.0f;
float currentGazeY = 0.0f;

int targetGazeX = 0;
int targetGazeY = 0;

int currentEyeHeight = BASE_EYE_HEIGHT;

int lastRenderedHeight = -1;
int lastRenderedGazeX = 999;
int lastRenderedGazeY = 999;

int roundedPixel(float value) {
  if (value >= 0.0f) {
    return (int)(value + 0.5f);
  }

  return (int)(value - 0.5f);
}

// Cartoon squash-and-stretch:
// - Side glance: narrower and taller.
// - Up glance: wider and shorter.
// - Down glance: narrower and taller.
// - Diagonal glance: slightly narrow and tall.
int getEyeWidth(int gazeX, int gazeY) {
  if (abs(gazeX) > 10 && abs(gazeY) > 5) {
    return 70;
  }

  if (abs(gazeX) > 10) {
    return 68;
  }

  if (gazeY < -5) {
    return 86;
  }

  if (gazeY > 5) {
    return 72;
  }

  return BASE_EYE_WIDTH;
}

float getEyeHeightScale(int gazeX, int gazeY) {
  if (abs(gazeX) > 10 && abs(gazeY) > 5) {
    return 1.07f;
  }

  if (abs(gazeX) > 10) {
    return 1.12f;
  }

  if (gazeY < -5) {
    return 0.90f;
  }

  if (gazeY > 5) {
    return 1.10f;
  }

  return 1.0f;
}

// Draw one solid eye. No pupils, highlights, or inner dots.
void drawCartoonEye(
  int centerX,
  int baseHeight,
  int gazeX,
  int gazeY
) {
  int eyeWidth = getEyeWidth(gazeX, gazeY);

  float heightScale = getEyeHeightScale(gazeX, gazeY);
  int eyeHeight = (int)(baseHeight * heightScale + 0.5f);

  int x = centerX - (eyeWidth / 2) + gazeX;
  int top = EYE_CENTER_Y - (eyeHeight / 2) + gazeY;

  int radius = min(eyeWidth / 2, eyeHeight / 2);
  int outlineRadius = min(
    (eyeWidth + 6) / 2,
    (eyeHeight + 6) / 2
  );

  // Solid cyan eye shape.
  eyeCanvas->fillRoundRect(
    x,
    top,
    eyeWidth,
    eyeHeight,
    radius,
    EYE_COLOR
  );

  // Subtle dark-cyan outline.
  eyeCanvas->drawRoundRect(
    x - 3,
    top - 3,
    eyeWidth + 6,
    eyeHeight + 6,
    outlineRadius,
    EYE_OUTLINE
  );
}

// Render both eyes together through the off-screen canvas.
void renderEyes(
  int height,
  int gazeX,
  int gazeY
) {
  eyeCanvas->fillScreen(BG_COLOR);

  drawCartoonEye(
    LEFT_EYE_CENTER_X,
    height,
    gazeX,
    gazeY
  );

  drawCartoonEye(
    RIGHT_EYE_CENTER_X,
    height,
    gazeX,
    gazeY
  );

  tft.drawRGBBitmap(
    CANVAS_X,
    CANVAS_Y,
    eyeCanvas->getBuffer(),
    CANVAS_W,
    CANVAS_H
  );
}

// Smoothly move the whole pair of eyes to each gaze direction.
void updateGaze(unsigned long now) {
  if ((long)(now - nextGazeAt) >= 0) {
    gazeIndex = (gazeIndex + 1) % GAZE_COUNT;

    targetGazeX = gazeSequence[gazeIndex].x;
    targetGazeY = gazeSequence[gazeIndex].y;

    nextGazeAt = now + random(
      GAZE_MIN_DWELL_MS,
      GAZE_MAX_DWELL_MS + 1
    );
  }

  currentGazeX +=
    (targetGazeX - currentGazeX) * GAZE_SMOOTHING;

  currentGazeY +=
    (targetGazeY - currentGazeY) * GAZE_SMOOTHING;
}

// Synchronized blink with an occasional double blink.
void updateBlink(unsigned long now) {
  switch (blinkState) {
    case EYES_OPEN:
      currentEyeHeight = BASE_EYE_HEIGHT;

      if ((long)(now - nextBlinkAt) >= 0) {
        // About one in five normal blinks gets one extra blink.
        // The second blink cannot trigger a third one.
        if (isSecondBlink) {
          secondBlinkQueued = false;
        } else {
          secondBlinkQueued = (random(0, 5) == 0);
        }

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
          BASE_EYE_HEIGHT -
          ((BASE_EYE_HEIGHT - EYE_CLOSED_HEIGHT) *
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
        currentEyeHeight = BASE_EYE_HEIGHT;
        blinkState = EYES_OPEN;

        if (secondBlinkQueued) {
          nextBlinkAt = now + 260;
          secondBlinkQueued = false;
          isSecondBlink = true;
        } else {
          isSecondBlink = false;
          nextBlinkAt = now + random(2500, 5500);
        }
      } else {
        currentEyeHeight =
          EYE_CLOSED_HEIGHT +
          ((BASE_EYE_HEIGHT - EYE_CLOSED_HEIGHT) *
           (int)elapsed / (int)OPEN_MS);
      }
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // Preserve the known-good ST7789 configuration.
  tft.init(240, 240, SPI_MODE3);
  tft.setRotation(2);
  tft.fillScreen(BG_COLOR);

  // Canvas memory: 240 x 174 x 2 = 83,520 bytes.
  eyeCanvas = new GFXcanvas16(CANVAS_W, CANVAS_H);

  if (eyeCanvas == nullptr ||
      eyeCanvas->getBuffer() == nullptr) {
    Serial.println("ERROR: Canvas allocation failed!");

    while (true) {
      delay(1000);
    }
  }

  randomSeed((unsigned long)micros());

  unsigned long now = millis();

  nextBlinkAt = now + random(1800, 3500);
  nextGazeAt = now + 800;

  Serial.println("BIG EYES V2.4 - EXAGGERATED CARTOON GAZE");
  Serial.println("Solid eyes only. No pupils.");
  Serial.println("Canvas allocated successfully.");

  renderEyes(BASE_EYE_HEIGHT, 0, 0);

  lastRenderedHeight = BASE_EYE_HEIGHT;
  lastRenderedGazeX = 0;
  lastRenderedGazeY = 0;
}

void loop() {
  unsigned long now = millis();

  if (now - lastFrameAt < FRAME_INTERVAL_MS) {
    return;
  }

  lastFrameAt = now;

  updateBlink(now);
  updateGaze(now);

  int gazeX = roundedPixel(currentGazeX);
  int gazeY = roundedPixel(currentGazeY);

  // Redraw only when eye height or gaze position changes.
  if (currentEyeHeight != lastRenderedHeight ||
      gazeX != lastRenderedGazeX ||
      gazeY != lastRenderedGazeY) {

    renderEyes(
      currentEyeHeight,
      gazeX,
      gazeY
    );

    lastRenderedHeight = currentEyeHeight;
    lastRenderedGazeX = gazeX;
    lastRenderedGazeY = gazeY;
  }
}
