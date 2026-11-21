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

// Wide canvas: 240 x 174 pixels, 16-bit color.
#define CANVAS_X 0
#define CANVAS_Y 33
#define CANVAS_W 240
#define CANVAS_H 174

// Large eye geometry
const int LEFT_EYE_CENTER_X = 60;
const int RIGHT_EYE_CENTER_X = 180;
const int EYE_CENTER_Y = 87;
const int EYE_WIDTH = 96;
const int EYE_OPEN_HEIGHT = 150;
const int EYE_CLOSED_HEIGHT = 10;

// Blink timing
const unsigned long CLOSE_MS = 160;
const unsigned long HOLD_MS = 80;
const unsigned long OPEN_MS = 160;

// Cartoon gaze timing
const unsigned long FRAME_INTERVAL_MS = 25;
const unsigned long GAZE_MIN_DWELL_MS = 1000;
const unsigned long GAZE_MAX_DWELL_MS = 1600;
const float GAZE_SMOOTHING = 0.24f;

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

// The whole pair of eyes moves like a cartoon character.
// No pupils, highlights, or inner dots are drawn.
const GazePoint gazeSequence[] = {
  {  0,  0 },  // Center
  { -7,  0 },  // Look left
  {  0,  0 },  // Center
  {  7,  0 },  // Look right
  {  0,  0 },  // Center
  {  0, -5 },  // Look up
  {  0,  0 },  // Center
  {  0,  5 },  // Look down
  {  0,  0 },  // Center
  { -7, -5 },  // Upper-left corner
  {  0,  0 },  // Center
  {  7, -5 },  // Upper-right corner
  {  0,  0 }   // Center, repeat
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

int currentEyeHeight = EYE_OPEN_HEIGHT;

int lastRenderedHeight = -1;
int lastRenderedGazeX = 999;
int lastRenderedGazeY = 999;

int roundedPixel(float value) {
  if (value >= 0.0f) {
    return (int)(value + 0.5f);
  }

  return (int)(value - 0.5f);
}

// Draw one large solid eye. The eye shape itself moves;
// there are no pupils, highlights, or inner dots.
void drawCartoonEye(
  int centerX,
  int height,
  int gazeX,
  int gazeY
) {
  int x = centerX - (EYE_WIDTH / 2) + gazeX;
  int top = EYE_CENTER_Y - (height / 2) + gazeY;

  int radius = min(EYE_WIDTH / 2, height / 2);
  int outlineRadius = min(
    (EYE_WIDTH + 6) / 2,
    (height + 6) / 2
  );

  eyeCanvas->fillRoundRect(
    x,
    top,
    EYE_WIDTH,
    height,
    radius,
    EYE_COLOR
  );

  eyeCanvas->drawRoundRect(
    x - 3,
    top - 3,
    EYE_WIDTH + 6,
    height + 6,
    outlineRadius,
    EYE_OUTLINE
  );
}

// Draw both eyes into one canvas, then transfer them together.
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

// Smoothly move the whole pair of eyes through each direction.
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
      currentEyeHeight = EYE_OPEN_HEIGHT;

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

        if (secondBlinkQueued) {
          // Queue exactly one second blink.
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

  // Preserve the verified ST7789 configuration.
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

  Serial.println("BIG EYES V2.3 - SOLID CARTOON EYES");
  Serial.println("No pupils. Large eye shapes only.");
  Serial.println("Canvas allocated successfully.");

  renderEyes(EYE_OPEN_HEIGHT, 0, 0);

  lastRenderedHeight = EYE_OPEN_HEIGHT;
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
