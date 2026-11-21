#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ============================================================
// DIGITAL PET - BIG EYES V2.6
// Touch Interaction + Cartoon Personality
// ESP32-S3 N16R8 + YY-PS130-V4.0 ST7789 LCD
// ============================================================

// Verified LCD wiring
#define TFT_SCLK 12
#define TFT_MOSI 11
#define TFT_RST  8
#define TFT_DC   10
#define TFT_CS   -1

// TTP223B touch sensor (module marked HIGH LEVEL)
#define TOUCH_PIN 4
#define TOUCH_DEBOUNCE_MS 35
#define LONG_PRESS_MS 700

// Five short taps within this time trigger a fear reaction.
#define FEAR_TAP_COUNT 5
#define TAP_WINDOW_MS 3000
#define FEAR_DURATION_MS 1400
#define FEAR_STEP_MS 55

// RGB565 colors
#define BG_COLOR       0x0000
#define EYE_OUTLINE    0x18E3

// Eye theme palette: cyan, yellow, white, magenta
const uint16_t eyeThemeColors[] = {
  0x07FF,
  0xFFE0,
  0xFFFF,
  0xF81F
};

const uint8_t THEME_COUNT =
  sizeof(eyeThemeColors) / sizeof(eyeThemeColors[0]);

uint8_t colorIndex = 0;

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

const float GAZE_SPRING = 0.24f;
const float GAZE_DAMPING = 0.62f;

// Blink timing
const unsigned long BLINK_CLOSE_MS = 150;
const unsigned long BLINK_HOLD_MS = 75;
const unsigned long BLINK_OPEN_MS = 150;

// Touch happy reaction
const unsigned long HAPPY_DURATION_MS = 850;

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
  MOOD_HAPPY,
  MOOD_SCARED,
  MOOD_PANIC
};

struct GazePoint {
  int8_t x;
  int8_t y;
};

PetMood currentMood = MOOD_IDLE;
BlinkState blinkState = EYES_OPEN;

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

// Touch state
bool lastRawTouchState = LOW;
bool stableTouchState = LOW;

unsigned long rawTouchChangedAt = 0;
unsigned long touchStartedAt = 0;

bool touchActive = false;
bool longPressHandled = false;

// Short-tap counter
uint8_t shortTapCount = 0;
unsigned long tapWindowStartedAt = 0;

// Mood timers
unsigned long happyStartedAt = 0;

unsigned long fearStartedAt = 0;
unsigned long nextFearStepAt = 0;
int fearShakeX = 0;
int fearShakeY = 0;
int fearEyeHeight = 66;

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

uint16_t lastRenderedColor = 0x0000;
PetMood lastRenderedMood = MOOD_PANIC;

// ============================================================
// SMALL HELPERS
// ============================================================

int roundedPixel(float value) {
  if (value >= 0.0f) {
    return (int)(value + 0.5f);
  }

  return (int)(value - 0.5f);
}

uint16_t getActiveEyeColor() {
  return eyeThemeColors[colorIndex];
}

void scheduleNextGaze(unsigned long now) {
  nextGazeAt = now + random(
    GAZE_MIN_DWELL_MS,
    GAZE_MAX_DWELL_MS + 1
  );
}

// ============================================================
// GAZE / IDLE BEHAVIOR
// ============================================================

void chooseNextIdleLook(unsigned long now) {
  // Return to center between directional looks.
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

  gazeVelocityX +=
    (targetGazeX - currentGazeX) * GAZE_SPRING;

  gazeVelocityY +=
    (targetGazeY - currentGazeY) * GAZE_SPRING;

  gazeVelocityX *= GAZE_DAMPING;
  gazeVelocityY *= GAZE_DAMPING;

  currentGazeX += gazeVelocityX;
  currentGazeY += gazeVelocityY;

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
// TOUCH REACTIONS
// ============================================================

void triggerHappy(unsigned long now) {
  currentMood = MOOD_HAPPY;
  happyStartedAt = now;

  // Center the gaze and pause normal blinking during the reaction.
  currentGazeX = 0.0f;
  currentGazeY = 0.0f;

  targetGazeX = 0;
  targetGazeY = 0;

  gazeVelocityX = 0.0f;
  gazeVelocityY = 0.0f;

  currentEyeHeight = BASE_EYE_HEIGHT;

  blinkState = EYES_OPEN;
  secondBlinkQueued = false;
  isSecondBlink = false;
}

void triggerScared(unsigned long now) {
  currentMood = MOOD_SCARED;
  fearStartedAt = now;
  nextFearStepAt = 0;

  // A frightened pet pulls its eyes closer together,
  // shrinks them, and trembles briefly.
  fearShakeX = 0;
  fearShakeY = 0;
  fearEyeHeight = 66;

  currentGazeX = 0.0f;
  currentGazeY = 0.0f;
  targetGazeX = 0;
  targetGazeY = 0;
  gazeVelocityX = 0.0f;
  gazeVelocityY = 0.0f;

  blinkState = EYES_OPEN;
  currentEyeHeight = BASE_EYE_HEIGHT;
  secondBlinkQueued = false;
  isSecondBlink = false;

  Serial.println("FEAR REACTION! Five quick taps detected.");
}

void updateScared(unsigned long now) {
  if (now - fearStartedAt >= FEAR_DURATION_MS) {
    currentMood = MOOD_IDLE;

    fearShakeX = 0;
    fearShakeY = 0;
    fearEyeHeight = 66;

    currentGazeX = 0.0f;
    currentGazeY = 0.0f;
    targetGazeX = 0;
    targetGazeY = 0;
    gazeVelocityX = 0.0f;
    gazeVelocityY = 0.0f;

    scheduleNextGaze(now);
    nextBlinkAt = now + random(900, 1800);
    Serial.println("FEAR REACTION FINISHED");
    return;
  }

  // Small, rapid trembling with slight eye-height variation.
  if ((long)(now - nextFearStepAt) >= 0) {
    fearShakeX = random(-4, 5);
    fearShakeY = random(-3, 4);
    fearEyeHeight = random(58, 73);
    nextFearStepAt = now + FEAR_STEP_MS;
  }
}

void triggerPanic(unsigned long now) {
  currentMood = MOOD_PANIC;
  panicStartedAt = now;
  nextPanicStepAt = 0;

  blinkState = EYES_OPEN;
  currentEyeHeight = BASE_EYE_HEIGHT;

  secondBlinkQueued = false;
  isSecondBlink = false;

  targetGazeX = 0;
  targetGazeY = 0;

  gazeVelocityX = 0.0f;
  gazeVelocityY = 0.0f;
}

void updateTouch(unsigned long now) {
  int rawTouch = digitalRead(TOUCH_PIN);

  // Track raw changes to debounce the sensor output.
  if (rawTouch != lastRawTouchState) {
    lastRawTouchState = rawTouch;
    rawTouchChangedAt = now;
  }

  // Accept a state only after it remains stable.
  if (rawTouch != stableTouchState &&
      now - rawTouchChangedAt >= TOUCH_DEBOUNCE_MS) {

    stableTouchState = rawTouch;

    if (stableTouchState == HIGH) {
      touchActive = true;
      touchStartedAt = now;
      longPressHandled = false;

      Serial.println("TOUCH STARTED");
    } else {
      touchActive = false;

      unsigned long pressDuration =
        now - touchStartedAt;

      if (!longPressHandled) {
        // Start or restart the multi-tap window.
        if (shortTapCount == 0 ||
            now - tapWindowStartedAt > TAP_WINDOW_MS) {
          shortTapCount = 0;
          tapWindowStartedAt = now;
        }

        shortTapCount++;

        Serial.print("SHORT TAP ");
        Serial.print(shortTapCount);
        Serial.print("/");
        Serial.println(FEAR_TAP_COUNT);

        if (shortTapCount >= FEAR_TAP_COUNT) {
          shortTapCount = 0;
          tapWindowStartedAt = 0;
          triggerScared(now);
        } else {
          triggerHappy(now);
        }
      } else {
        // A long press changes color, but does not count as a tap.
        shortTapCount = 0;
        tapWindowStartedAt = 0;

        Serial.print("LONG TOUCH RELEASED: ");
        Serial.print(pressDuration);
        Serial.println(" ms");
      }
    }
  }

  // A long press changes the eye color once per touch.
  if (touchActive &&
      !longPressHandled &&
      now - touchStartedAt >= LONG_PRESS_MS) {

    longPressHandled = true;

    // Long press is a separate gesture, not part of the tap sequence.
    shortTapCount = 0;
    tapWindowStartedAt = 0;

    colorIndex = (colorIndex + 1) % THEME_COUNT;

    Serial.print("LONG TOUCH - COLOR THEME: ");
    Serial.println(colorIndex);

    // Give visual feedback for the color change.
    triggerHappy(now);
  }
}

void updateHappy(unsigned long now) {
  if (now - happyStartedAt >= HAPPY_DURATION_MS) {
    currentMood = MOOD_IDLE;

    currentEyeHeight = BASE_EYE_HEIGHT;

    scheduleNextGaze(now);
    nextBlinkAt = now + random(900, 1800);
  }
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

  if ((long)(now - nextPanicStepAt) >= 0) {
    panicShakeX = random(-6, 7);
    panicShakeY = random(-3, 4);

    panicEyeHeight = (random(0, 2) == 0)
                       ? 78
                       : 128;

    nextPanicStepAt = now + PANIC_STEP_MS;
  }
}

// ============================================================
// SERIAL DEBUG COMMANDS
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
// CARTOON EYE SHAPES
// ============================================================

int getEyeWidth(int gazeX, int gazeY, PetMood mood) {
  if (mood == MOOD_PANIC) {
    return 70;
  }

  if (mood == MOOD_SCARED) {
    return 52;
  }

  if (mood == MOOD_HAPPY) {
    return 88;
  }

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

float getEyeHeightScale(
  int gazeX,
  int gazeY,
  PetMood mood
) {
  if (mood == MOOD_HAPPY) {
    return 0.82f;
  }

  if (mood == MOOD_SCARED) {
    return 1.0f;
  }

  if (mood == MOOD_PANIC) {
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

void drawCartoonEye(
  int centerX,
  int baseHeight,
  int gazeX,
  int gazeY,
  PetMood mood
) {
  // During fear, the eyes huddle closer to the center.
  if (mood == MOOD_SCARED) {
    centerX = (centerX < 120) ? 91 : 149;
  }

  int eyeWidth = getEyeWidth(gazeX, gazeY, mood);

  float scale =
    getEyeHeightScale(gazeX, gazeY, mood);

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

  // Solid eye only. No pupils, highlights, dots, or mouth.
  eyeCanvas->fillRoundRect(
    x,
    top,
    eyeWidth,
    eyeHeight,
    radius,
    getActiveEyeColor()
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
  PetMood mood
) {
  eyeCanvas->fillScreen(BG_COLOR);

  drawCartoonEye(
    LEFT_EYE_CENTER_X,
    eyeHeight,
    gazeX,
    gazeY,
    mood
  );

  drawCartoonEye(
    RIGHT_EYE_CENTER_X,
    eyeHeight,
    gazeX,
    gazeY,
    mood
  );

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

  // TTP223B is a HIGH-level digital output.
  pinMode(TOUCH_PIN, INPUT);

  lastRawTouchState = digitalRead(TOUCH_PIN);
  stableTouchState = lastRawTouchState;
  rawTouchChangedAt = millis();

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.init(240, 240, SPI_MODE3);
  tft.setRotation(2);
  tft.fillScreen(BG_COLOR);

  // Canvas memory: 83,520 bytes.
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
  Serial.println("DIGITAL PET V2.6 - TOUCH INTERACTION");
  Serial.println("TTP223B active HIGH on GPIO 4");
  Serial.println("Short touch: happy reaction");
  Serial.println("Long touch (>700 ms): change eye color");
  Serial.println("Serial: P=panic, B=blink, C=center");
  Serial.println("Canvas allocated successfully.");

  renderPetEyes(
    BASE_EYE_HEIGHT,
    0,
    0,
    MOOD_IDLE
  );

  lastRenderedHeight = BASE_EYE_HEIGHT;
  lastRenderedGazeX = 0;
  lastRenderedGazeY = 0;
  lastRenderedColor = getActiveEyeColor();
  lastRenderedMood = MOOD_IDLE;
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {
  unsigned long now = millis();

  updateSerialInput(now);
  updateTouch(now);

  if (now - lastFrameAt < FRAME_INTERVAL_MS) {
    return;
  }

  lastFrameAt = now;

  if (currentMood == MOOD_PANIC) {
    updatePanic(now);
  } else if (currentMood == MOOD_HAPPY) {
    updateHappy(now);
  } else if (currentMood == MOOD_SCARED) {
    updateScared(now);
  }

  // Update normal idle animation only outside reactions.
  if (currentMood == MOOD_IDLE) {
    updateGaze(now);
    updateBlink(now);
  }

  int renderX = 0;
  int renderY = 0;
  int renderHeight = BASE_EYE_HEIGHT;

  if (currentMood == MOOD_PANIC) {
    renderX = panicShakeX;
    renderY = panicShakeY;
    renderHeight = panicEyeHeight;
  } else if (currentMood == MOOD_SCARED) {
    renderX = fearShakeX;
    renderY = fearShakeY;
    renderHeight = fearEyeHeight;
  } else if (currentMood == MOOD_HAPPY) {
    // Happy reaction: wider and shorter solid eyes.
    renderHeight = BASE_EYE_HEIGHT;
  } else {
    renderX = roundedPixel(currentGazeX);
    renderY = roundedPixel(currentGazeY);
    renderHeight = currentEyeHeight;
  }

  uint16_t activeColor = getActiveEyeColor();

  // Redraw only if a visible property changed.
  if (renderHeight != lastRenderedHeight ||
      renderX != lastRenderedGazeX ||
      renderY != lastRenderedGazeY ||
      activeColor != lastRenderedColor ||
      currentMood != lastRenderedMood) {

    renderPetEyes(
      renderHeight,
      renderX,
      renderY,
      currentMood
    );

    lastRenderedHeight = renderHeight;
    lastRenderedGazeX = renderX;
    lastRenderedGazeY = renderY;
    lastRenderedColor = activeColor;
    lastRenderedMood = currentMood;
  }
}
