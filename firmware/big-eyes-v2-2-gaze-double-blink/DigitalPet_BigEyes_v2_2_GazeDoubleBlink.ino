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
#define PUPIL_COLOR     0x0010
#define EYE_HIGHLIGHT   0xFFFF

// Canvas area on the LCD
#define CANVAS_X 40
#define CANVAS_Y 70
#define CANVAS_W 160
#define CANVAS_H 100

// Eye geometry inside the canvas
const int LEFT_EYE_X = 8;
const int RIGHT_EYE_X = 104;
const int EYE_CENTER_Y = 50;
const int EYE_WIDTH = 48;

const int EYE_OPEN_HEIGHT = 80;
const int EYE_CLOSED_HEIGHT = 8;

// Blink timing
const unsigned long CLOSE_MS = 160;
const unsigned long HOLD_MS = 70;
const unsigned long OPEN_MS = 160;

// Gaze and drawing timing
const unsigned long FRAME_INTERVAL_MS = 25;
const unsigned long GAZE_MIN_DWELL_MS = 900;
const unsigned long GAZE_MAX_DWELL_MS = 1450;
const float GAZE_SMOOTHING = 0.28f;

// Adafruit GFX off-screen 16-bit canvas
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

// Sequence: center, left, right, up, down,
// upper-left, and upper-right, returning to center between looks.
const GazePoint gazeSequence[] = {
  {  0,   0 },  // Center
  { -9,   0 },  // Left
  {  0,   0 },  // Center
  {  9,   0 },  // Right
  {  0,   0 },  // Center
  {  0, -14 },  // Up
  {  0,   0 },  // Center
  {  0,  14 },  // Down
  {  0,   0 },  // Center
  { -8, -13 },  // Upper-left
  {  0,   0 },  // Center
  {  8, -13 },  // Upper-right
  {  0,   0 }   // Center
};

const uint8_t GAZE_COUNT =
  sizeof(gazeSequence) / sizeof(gazeSequence[0]);

BlinkState blinkState = EYES_OPEN;

unsigned long stateStartedAt = 0;
unsigned long nextBlinkAt = 0;
unsigned long lastFrameAt = 0;
unsigned long nextGazeAt = 0;

bool secondBlinkQueued = false;

uint8_t gazeIndex = 0;

float currentGazeX = 0.0f;
float currentGazeY = 0.0f;

int targetGazeX = 0;
int targetGazeY = 0;

int currentEyeHeight = EYE_OPEN_HEIGHT;

int lastRenderedHeight = -1;
int lastRenderedGazeX = 999;
int lastRenderedGazeY = 999;

// Round a floating-point gaze position to the nearest pixel.
int roundedPixel(float value) {
  if (value >= 0.0f) {
    return (int)(value + 0.5f);
  }

  return (int)(value - 0.5f);
}

// Draw one eye and its moving pupil into the canvas.
void drawEyeOnCanvas(
  int x,
  int height,
  int gazeX,
  int gazeY
) {
  int top = EYE_CENTER_Y - (height / 2);

  int radius = (height < 32)
                 ? (height / 2)
                 : 16;

  // Main cyan eye.
  eyeCanvas->fillRoundRect(
    x, top,
    EYE_WIDTH, height,
    radius, EYE_COLOR
  );

  // Subtle outline.
  eyeCanvas->drawRoundRect(
    x - 2, top - 2,
    52, height + 4,
    radius + 2, EYE_OUTLINE
  );

  // Hide pupils and highlights while the eye is closing.
  if (height >= 42) {
    int pupilX = x + 15 + gazeX;
    int pupilY = top + 25 + gazeY;

    // Dark pupil / iris.
    eyeCanvas->fillRoundRect(
      pupilX, pupilY,
      18, 30,
      8, PUPIL_COLOR
    );

    // Eye reflections.
    eyeCanvas->fillCircle(
      pupilX + 5, pupilY + 6,
      4, EYE_HIGHLIGHT
    );

    eyeCanvas->fillCircle(
      pupilX + 13, pupilY + 19,
      2, EYE_HIGHLIGHT
    );
  }
}

// Draw both eyes into one canvas and transfer them together.
void renderEyes(
  int height,
  int gazeX,
  int gazeY
) {
  eyeCanvas->fillScreen(BG_COLOR);

  drawEyeOnCanvas(
    LEFT_EYE_X, height, gazeX, gazeY
  );

  drawEyeOnCanvas(
    RIGHT_EYE_X, height, gazeX, gazeY
  );

  tft.drawRGBBitmap(
    CANVAS_X,
    CANVAS_Y,
    eyeCanvas->getBuffer(),
    CANVAS_W,
    CANVAS_H
  );
}

// Advance through the requested gaze directions.
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

  // Smoothly move the pupils toward their target.
  currentGazeX +=
    (targetGazeX - currentGazeX) * GAZE_SMOOTHING;

  currentGazeY +=
    (targetGazeY - currentGazeY) * GAZE_SMOOTHING;
}

// Synchronized blink animation with occasional double blink.
void updateBlink(unsigned long now) {
  switch (blinkState) {
    case EYES_OPEN:
      currentEyeHeight = EYE_OPEN_HEIGHT;

      if ((long)(now - nextBlinkAt) >= 0) {
        // About one in five blinks is followed by a second blink.
        secondBlinkQueued = (random(0, 5) == 0);

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
          // Short pause before the second blink.
          nextBlinkAt = now + 260;
          secondBlinkQueued = false;
        } else {
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

  unsigned long now = millis();

  nextBlinkAt = now + random(1800, 3500);
  nextGazeAt = now + 900;

  Serial.println("BIG EYES V2.2 - GAZE + DOUBLE BLINK");
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

  // Redraw only when the eye shape or pupil position changes.
  if (currentEyeHeight != lastRenderedHeight ||
      gazeX != lastRenderedGazeX ||
      gazeY != lastRenderedGazeY) {

    renderEyes(currentEyeHeight, gazeX, gazeY);

    lastRenderedHeight = currentEyeHeight;
    lastRenderedGazeX = gazeX;
    lastRenderedGazeY = gazeY;
  }
}
