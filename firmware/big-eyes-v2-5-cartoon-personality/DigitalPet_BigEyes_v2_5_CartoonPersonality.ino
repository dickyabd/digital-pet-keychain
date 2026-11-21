#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ============================================================
// DIGITAL PET - BIG EYES V2.5
// Cartoon Personality Engine
// ESP32-S3 N16R8 + YY-PS130-V4.0 ST7789 LCD
// ============================================================

// Verified LCD wiring
#define TFT_SCLK 12
#define TFT_MOSI 11
#define TFT_RST  8
#define TFT_DC   10
#define TFT_CS   -1

// RGB565 colors
#define BG_COLOR       0x0000
#define EYE_COLOR      0x07FF
#define EYE_OUTLINE    0x18E3

// Off-screen canvas: 240 x 174 x 2 = 83,520 bytes
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

// Rendering and gaze tuning
const unsigned long FRAME_INTERVAL_MS = 25;
const unsigned long GAZE_MIN_DWELL_MS = 850;
const unsigned long GAZE_MAX_DWELL_MS = 1500;

// Damped spring: responsive but with a small natural overshoot.
const float GAZE_SPRING = 0.24f;
const float GAZE_DAMPING = 0.62f;

// Blink timing
const unsigned long BLINK_CLOSE_MS = 150;
const unsigned long BLINK_HOLD_MS = 75;
const unsigned long BLINK_OPEN_MS = 150;

// Panic demo timing
const unsigned long PANIC_DURATION_MS = 1100;
const unsigned long PANIC_STEP_MS = 85;

// LCD and canvas
Adafruit_ST7789 tft = Adafruit_ST7789(
  &SPI, TFT_CS, TFT_DC, TFT_RST
);

GFXcanvas16 *eyeCanvas = nullptr;

// ============================================================
// STATES
// ============================================================

enum BlinkState {
  EYES_OPEN,
  EYES_CLOSING,
  EYES_CLOSED,
  EYES_OPENING
};

enum PetMood {
  MOOD_IDLE,
  MOOD_PANIC
};

struct GazePoint {
  int8_t x;
  int8_t y;
};

PetMood currentMood = MOOD_IDLE;
BlinkState blinkState = EYES_OPEN;

// Available idle looks. Center is inserted between looks.
const GazePoint gazeDirections[] = {
  { -24,   0 },  // Far left
  {  24,   0 },  // Far right
  {   0, -16 },  // Up
  {   0,  16 },  // Down
  { -22, -15 },  // Upper-left
  {  22, -15 }   // Upper-right
};

const uint8_t GAZE_DIRECTION_COUNT =
  sizeof(gazeDirections) / sizeof(gazeDirections[0]);

// ============================================================
// RUNTIME VARIABLES
// ============================================================

// Gaze position and spring velocity
float currentGazeX = 0.0f;
float currentGazeY = 0.0f;

float gazeVelocityX = 0.0f;
float gazeVelocityY = 0.0f;

int targetGazeX = 0;
int targetGazeY = 0;

unsigned long nextGazeAt = 0;

// Blink state
unsigned long blinkStateStartedAt = 0;
unsigned long nextBlinkAt = 0;

bool secondBlinkQueued = false;
bool isSecondBlink = false;

int currentEyeHeight = BASE_EYE_HEIGHT;

// Panic state
unsigned long panicStartedAt = 0;
unsigned long nextPanicStepAt = 0;

int panicShakeX = 0;
int panicShakeY = 0;
int panicEyeHeight = BASE_EYE_HEIGHT;

// Frame control
unsigned long lastFrameAt = 0;

int lastRenderedHeight = -1;
int lastRenderedGazeX = 999;
int lastRenderedGazeY = 999;
bool lastRenderedPanic = false;

// ============================================================
// SMALL HELPERS
// ============================================================

int roundedPixel(float value) {
  if (value >= 0.0f) {
    return (int)(value + 0.5f);
  }

  return (int)(value - 0.5f);
}

void scheduleNextGaze(unsigned long now) {
  nextGazeAt = now + random(
    GAZE_MIN_DWELL_MS,
    GAZE_MAX_DWELL_MS + 1
  );
}

// ============================================================
// IDLE GAZE / CARTOON EYE MOVEMENT
// ============================================================

void chooseNextIdleLook(unsigned long now) {
  // Always return to center between two directional looks.
  if (targetGazeX != 0 || targetGazeY != 0) {
    targetGazeX = 0;
    targetGazeY = 0;
  } else {
    uint8_t pick = random(0, GAZE_DIRECTION_COUNT);

    targetGazeX = gazeDirections[pick].x;
    targetGazeY = gazeDirections[pick].y;
  }

  scheduleNextGaze(now);
}

void updateGaze(unsigned long now) {
  if ((long)(now - nextGazeAt) >= 0) {
    chooseNextIdleLook(now);
  }

  // Damped spring movement. This gives a quick look with a
  // subtle overshoot rather than a slow linear slide.
  gazeVelocityX +=
    (targetGazeX - currentGazeX) * GAZE_SPRING;

  gazeVelocityY +=
    (targetGazeY - currentGazeY) * GAZE_SPRING;

  gazeVelocityX *= GAZE_DAMPING;
  gazeVelocityY *= GAZE_DAMPING;

  currentGazeX += gazeVelocityX;
  currentGazeY += gazeVelocityY;

  // Stop tiny residual movements to prevent unnecessary redraws.
  if (abs(targetGazeX - currentGazeX) < 0.18f &&
      abs(gazeVelocityX) < 0.18f) {
    currentGazeX = targetGazeX;
    gazeVelocityX = 0.0f;
  }

  if (abs(targetGazeY - currentGazeY) < 0.18f &&
      abs(gazeVelocityY) < 0.18f) {
    currentGazeY = targetGazeY;
    gazeVelocityY = 0.0f;
  }
}

// ============================================================
// BLINK / DOUBLE BLINK
// ============================================================

void updateBlink(unsigned long now) {
  switch (blinkState) {
    case EYES_OPEN:
      currentEyeHeight = BASE_EYE_HEIGHT;

      if ((long)(now - nextBlinkAt) >= 0) {
        // Approximately one out of five normal blinks becomes
        // exactly one double blink.
        if (isSecondBlink) {
          secondBlinkQueued = false;
        } else {
          secondBlinkQueued = (random(0, 5) == 0);
        }

        blinkState = EYES_CLOSING;
        blinkStateStartedAt = now;
      }
      break;

    case EYES_CLOSING: {
      unsigned long elapsed = now - blinkStateStartedAt;

      if (elapsed >= BLINK_CLOSE_MS) {
        currentEyeHeight = EYE_CLOSED_HEIGHT;
        blinkState = EYES_CLOSED;
        blinkStateStartedAt = now;
      } else {
        currentEyeHeight =
          BASE_EYE_HEIGHT -
          ((BASE_EYE_HEIGHT - EYE_CLOSED_HEIGHT) *
           (int)elapsed / (int)BLINK_CLOSE_MS);
      }
      break;
    }

    case EYES_CLOSED:
      currentEyeHeight = EYE_CLOSED_HEIGHT;

      if (now - blinkStateStartedAt >= BLINK_HOLD_MS) {
        blinkState = EYES_OPENING;
        blinkStateStartedAt = now;
      }
      break;

    case EYES_OPENING: {
      unsigned long elapsed = now - blinkStateStartedAt;

      if (elapsed >= BLINK_OPEN_MS) {
        currentEyeHeight = BASE_EYE_HEIGHT;
        blinkState = EYES_OPEN;

        if (secondBlinkQueued) {
          // Schedule exactly one additional blink.
          nextBlinkAt = now + 240;
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
           (int)elapsed / (int)BLINK_OPEN_MS);
      }
      break;
    }
  }
}

// ============================================================
// PANIC / STARTLE REACTION
// ============================================================

void triggerPanic(unsigned long now) {
  currentMood = MOOD_PANIC;
  panicStartedAt = now;
  nextPanicStepAt = 0;

  // Cancel an in-progress blink and temporarily center the gaze.
  blinkState = EYES_OPEN;
  currentEyeHeight = BASE_EYE_HEIGHT;

  secondBlinkQueued = false;
  isSecondBlink = false;

  targetGazeX = 0;
  targetGazeY = 0;

  gazeVelocityX = 0.0f;
  gazeVelocityY = 0.0f;
}

void updatePanic(unsigned long now) {
  if (now - panicStartedAt >= PANIC_DURATION_MS) {
    currentMood = MOOD_IDLE;

    panicShakeX = 0;
    panicShakeY = 0;

    currentEyeHeight = BASE_EYE_HEIGHT;

    currentGazeX = 0.0f;
    currentGazeY = 0.0f;

    targetGazeX = 0;
    targetGazeY = 0;

    gazeVelocityX = 0.0f;
    gazeVelocityY = 0.0f;

    scheduleNextGaze(now);
    nextBlinkAt = now + random(900, 1800);

    return;
  }

  // Brief cartoon shake and alternating squash/stretch.
  if ((long)(now - nextPanicStepAt) >= 0) {
    panicShakeX = random(-6, 7);
    panicShakeY = random(-3, 4);

    if (random(0, 2) == 0) {
      panicEyeHeight = 78;
    } else {
      panicEyeHeight = 128;
    }

    nextPanicStepAt = now + PANIC_STEP_MS;
  }
}

// ============================================================
// SERIAL DEMO INPUT
// ============================================================

void updateSerialInput(unsigned long now) {
  while (Serial.available() > 0) {
    char command = (char)Serial.read();

    if (command == 'p' || command == 'P') {
      triggerPanic(now);
      Serial.println("PANIC REACTION!");
    }

    if (command == 'b' || command == 'B') {
      if (currentMood == MOOD_IDLE) {
        nextBlinkAt = now;
        Serial.println("FORCED BLINK!");
      }
    }

    if (command == 'c' || command == 'C') {
      currentGazeX = 0.0f;
      currentGazeY = 0.0f;
      gazeVelocityX = 0.0f;
      gazeVelocityY = 0.0f;
      targetGazeX = 0;
      targetGazeY = 0;
      scheduleNextGaze(now);
      Serial.println("GAZE CENTER!");
    }
  }
}

// ============================================================
// CARTOON SQUASH-AND-STRETCH
// ============================================================

int getEyeWidth(int gazeX, int gazeY, bool panic) {
  if (panic) {
    return 70;
  }

  // Diagonal look: slightly narrower.
  if (abs(gazeX) > 10 && abs(gazeY) > 5) {
    return 70;
  }

  // Left / right: narrow and tall.
  if (abs(gazeX) > 10) {
    return 68;
  }

  // Up: wider.
  if (gazeY < -5) {
    return 86;
  }

  // Down: slightly narrower.
  if (gazeY > 5) {
    return 72;
  }

  return BASE_EYE_WIDTH;
}

float getEyeHeightScale(int gazeX, int gazeY, bool panic) {
  if (panic) {
    return 1.0f;
  }

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

// ============================================================
// DRAWING
// ============================================================

void drawCartoonEye(
  int centerX,
  int baseHeight,
  int gazeX,
  int gazeY,
  bool panic
) {
  int eyeWidth = getEyeWidth(gazeX, gazeY, panic);

  float scale =
    getEyeHeightScale(gazeX, gazeY, panic);

  int eyeHeight =
    (int)(baseHeight * scale + 0.5f);

  int x =
    centerX - (eyeWidth / 2) + gazeX;

  int top =
    EYE_CENTER_Y - (eyeHeight / 2) + gazeY;

  int radius =
    min(eyeWidth / 2, eyeHeight / 2);

  int outlineRadius = min(
    (eyeWidth + 6) / 2,
    (eyeHeight + 6) / 2
  );

  // Solid cyan eye only. No pupils, highlights, or mouth.
  eyeCanvas->fillRoundRect(
    x,
    top,
    eyeWidth,
    eyeHeight,
    radius,
    EYE_COLOR
  );

  eyeCanvas->drawRoundRect(
    x - 3,
    top - 3,
    eyeWidth + 6,
    eyeHeight + 6,
    outlineRadius,
    EYE_OUTLINE
  );
}

void renderPetEyes(
  int eyeHeight,
  int gazeX,
  int gazeY,
  bool panic
) {
  eyeCanvas->fillScreen(BG_COLOR);

  drawCartoonEye(
    LEFT_EYE_CENTER_X,
    eyeHeight,
    gazeX,
    gazeY,
    panic
  );

  drawCartoonEye(
    RIGHT_EYE_CENTER_X,
    eyeHeight,
    gazeX,
    gazeY,
    panic
  );

  // Transfer both eyes together to reduce visible tearing.
  tft.drawRGBBitmap(
    CANVAS_X,
    CANVAS_Y,
    eyeCanvas->getBuffer(),
    CANVAS_W,
    CANVAS_H
  );
}

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // Keep the known-good LCD settings.
  tft.init(240, 240, SPI_MODE3);
  tft.setRotation(2);
  tft.fillScreen(BG_COLOR);

  // 240 x 174 x 2 bytes = 83,520 bytes.
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

  nextGazeAt = now + 700;
  nextBlinkAt = now + random(1800, 3500);

  Serial.println();
  Serial.println("DIGITAL PET V2.5 - CARTOON PERSONALITY ENGINE");
  Serial.println("Solid eyes only. No pupils. No mouth.");
  Serial.println("Canvas allocated successfully.");
  Serial.println();
  Serial.println("Serial demo commands:");
  Serial.println("P = trigger panic reaction");
  Serial.println("B = force one blink");
  Serial.println("C = return gaze to center");

  renderPetEyes(BASE_EYE_HEIGHT, 0, 0, false);

  lastRenderedHeight = BASE_EYE_HEIGHT;
  lastRenderedGazeX = 0;
  lastRenderedGazeY = 0;
  lastRenderedPanic = false;
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {
  unsigned long now = millis();

  updateSerialInput(now);

  if (now - lastFrameAt < FRAME_INTERVAL_MS) {
    return;
  }

  lastFrameAt = now;

  bool panic = (currentMood == MOOD_PANIC);

  if (panic) {
    updatePanic(now);
    panic = (currentMood == MOOD_PANIC);
  }

  if (!panic) {
    updateGaze(now);
    updateBlink(now);
  }

  int renderX;
  int renderY;
  int renderHeight;

  if (panic) {
    renderX = panicShakeX;
    renderY = panicShakeY;
    renderHeight = panicEyeHeight;
  } else {
    renderX = roundedPixel(currentGazeX);
    renderY = roundedPixel(currentGazeY);
    renderHeight = currentEyeHeight;
  }

  // Only redraw when a visible property has changed.
  if (renderHeight != lastRenderedHeight ||
      renderX != lastRenderedGazeX ||
      renderY != lastRenderedGazeY ||
      panic != lastRenderedPanic) {

    renderPetEyes(
      renderHeight,
      renderX,
      renderY,
      panic
    );

    lastRenderedHeight = renderHeight;
    lastRenderedGazeX = renderX;
    lastRenderedGazeY = renderY;
    lastRenderedPanic = panic;
  }
}
