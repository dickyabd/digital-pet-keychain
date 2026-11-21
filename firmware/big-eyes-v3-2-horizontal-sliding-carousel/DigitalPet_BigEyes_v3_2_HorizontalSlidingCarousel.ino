#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSansBoldOblique18pt7b.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// DIGITAL PET - BIG EYES V3.2
// Horizontal Sliding Eye Carousel + DINARA Entrance / Exit Transitions
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
#define NAME_HOLD_MS 3000

// DINARA marquee
#define NAME_SCROLL_SPEED 2.4f
#define NAME_EXIT_MARGIN 10

// Five short taps within this time trigger a fear reaction.
#define FEAR_TAP_COUNT 5
#define TAP_WINDOW_MS 3000
#define FEAR_DURATION_MS 1400
#define FEAR_STEP_MS 55

// RGB565 colors
#define BG_COLOR       0x0000
#define EYE_OUTLINE    0x18E3
#define NAME_PINK      0xFB56
#define NAME_WHITE     0xFFFF

// Five complete eye styles inspired by the visual reference and Gemini's idea.
enum EyeStyle : uint8_t {
  EYE_NORMAL = 0,
  EYE_CUTE,
  EYE_STAR,
  EYE_HEART,
  EYE_FIRE,
  EYE_STYLE_COUNT
};

struct EyeStyleConfig {
  uint16_t scleraColor;
  uint16_t pupilColor;
  uint16_t highlightColor;
};

const EyeStyleConfig eyeStyleConfig[EYE_STYLE_COUNT] = {
  { 0xFFFF, 0x001F, 0xFFFF }, // Normal: white sclera, navy oval pupil
  { 0x801F, 0x03E0, 0xFFFF }, // Cute: purple sclera, small green pupil
  { 0xD7E0, 0xFFE0, 0xFFFF }, // Star: lime sclera, yellow star
  { 0xFFFF, 0xF9B6, 0xFFFF }, // Heart: white sclera, pink heart
  { 0xFD20, 0xF800, 0xFFE0 }  // Fire: amber sclera, flame pupil
};

EyeStyle currentEyeStyle = EYE_NORMAL;

const unsigned long GACHA_DURATION_MS = 1700;
const unsigned long GACHA_START_INTERVAL_MS = 140;
const unsigned long GACHA_SLOWDOWN_MS = 260;
const unsigned long SLIDE_ROULETTE_MIN_MS = 110;
const unsigned long SLIDE_ROULETTE_MAX_EXTRA_MS = 90;
const unsigned long SLIDE_NAME_MS = 360;

bool isRollingGacha = false;
unsigned long gachaStartTime = 0;
unsigned long lastGachaTick = 0;
unsigned long rouletteResultUntil = 0;
bool rouletteOverlayDrawn = false;
bool rouletteFinalStarted = false;

enum SlideAction : uint8_t {
  SLIDE_NONE,
  SLIDE_ROULETTE_STEP,
  SLIDE_ROULETTE_FINAL,
  SLIDE_NAME_INTRO,
  SLIDE_NAME_EXIT
};

bool slideActive = false;
unsigned long slideStartedAt = 0;
unsigned long slideDurationMs = 0;
SlideAction currentSlideAction = SLIDE_NONE;

uint16_t *slideFromBuffer = nullptr;
uint16_t *slideLineBuffer = nullptr;

// Off-screen canvas: 240 x 174 x 2 = 83,520 bytes
#define CANVAS_X 0
#define CANVAS_Y 33
#define CANVAS_W 240
#define CANVAS_H 174

const size_t FRAME_PIXEL_COUNT = CANVAS_W * CANVAS_H;
const size_t FRAME_BUFFER_BYTES = FRAME_PIXEL_COUNT * sizeof(uint16_t);

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
  MOOD_PANIC,
  MOOD_NAME
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
bool nameHoldHandled = false;

// Name marquee metrics and position
const char NAME_TEXT[] = "DINARA";
float nameScrollX = 0.0f;
int16_t nameBoundsX = 0;
int16_t nameBoundsY = 0;
uint16_t nameTextWidth = 0;
uint16_t nameTextHeight = 0;

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
int lastRenderedEyeStyle = -1;

// Forward declarations for the horizontal transition renderer.
void renderPetEyes(int eyeHeight, int gazeX, int gazeY, PetMood mood, bool pushToDisplay = true);
void renderNameMarquee(unsigned long now, bool pushToDisplay = true);
void beginSlideTransition(unsigned long now, unsigned long durationMs, SlideAction action);
void beginEyeStyleSlide(EyeStyle nextStyle, unsigned long now, unsigned long durationMs, SlideAction action);
void beginNameMarqueeSlide(unsigned long now);
void beginReturnToEyesSlide(unsigned long now);
void drawSlideFrame(unsigned long now);
void finishSlideTransition(unsigned long now);

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
  return eyeStyleConfig[(uint8_t)currentEyeStyle].scleraColor;
}

const char* getEyeStyleName(EyeStyle style) {
  switch (style) {
    case EYE_NORMAL: return "NORMAL";
    case EYE_CUTE:   return "CUTE";
    case EYE_STAR:   return "STAR";
    case EYE_HEART:  return "HEART";
    case EYE_FIRE:   return "FIRE";
    default:         return "UNKNOWN";
  }
}

void triggerRoulette(unsigned long now) {
  isRollingGacha = true;
  gachaStartTime = now;
  lastGachaTick = now;
  rouletteResultUntil = 0;
  rouletteFinalStarted = false;

  // Establish a centered idle face before the first style slide.
  currentMood = MOOD_IDLE;
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

  renderPetEyes(BASE_EYE_HEIGHT, 0, 0, MOOD_IDLE, true);
  lastRenderedHeight = BASE_EYE_HEIGHT;
  lastRenderedGazeX = 0;
  lastRenderedGazeY = 0;
  lastRenderedMood = MOOD_IDLE;
  lastRenderedColor = getActiveEyeColor();
  lastRenderedEyeStyle = (int)currentEyeStyle;

  // Freeze the face at center while the eye styles roll.
  currentMood = MOOD_IDLE;
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

  Serial.println("EYE STYLE ROULETTE STARTED!");
}

void updateRoulette(unsigned long now) {
  if (!isRollingGacha || slideActive || rouletteFinalStarted) {
    return;
  }

  unsigned long elapsed = now - gachaStartTime;

  // End the roulette by sliding the final random style into position.
  if (elapsed >= GACHA_DURATION_MS) {
    rouletteFinalStarted = true;

    EyeStyle finalStyle = (EyeStyle)random(0, EYE_STYLE_COUNT);
    if (finalStyle == currentEyeStyle) {
      finalStyle = (EyeStyle)(((uint8_t)finalStyle + 1) % EYE_STYLE_COUNT);
    }

    beginEyeStyleSlide(
      finalStyle,
      now,
      SLIDE_ROULETTE_MIN_MS + SLIDE_ROULETTE_MAX_EXTRA_MS,
      SLIDE_ROULETTE_FINAL
    );
    return;
  }

  float progress = (float)elapsed / (float)GACHA_DURATION_MS;

  // The pause between slides grows as the roulette approaches its result.
  unsigned long interval = GACHA_START_INTERVAL_MS +
    (unsigned long)(progress * progress * GACHA_SLOWDOWN_MS);

  if (now - lastGachaTick >= interval) {
    lastGachaTick = now;

    uint8_t nextStyle = random(0, EYE_STYLE_COUNT);
    if (nextStyle == (uint8_t)currentEyeStyle) {
      nextStyle = (nextStyle + 1) % EYE_STYLE_COUNT;
    }

    unsigned long slideDuration =
      SLIDE_ROULETTE_MIN_MS +
      (unsigned long)(progress * SLIDE_ROULETTE_MAX_EXTRA_MS);

    beginEyeStyleSlide(
      (EyeStyle)nextStyle,
      now,
      slideDuration,
      SLIDE_ROULETTE_STEP
    );
  }
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

void triggerNameMarquee(unsigned long now) {
  isRollingGacha = false;
  rouletteFinalStarted = false;
  rouletteResultUntil = 0;

  if (rouletteOverlayDrawn) {
    tft.fillRect(0, 210, 240, 30, BG_COLOR);
    rouletteOverlayDrawn = false;
  }

  shortTapCount = 0;
  tapWindowStartedAt = 0;

  beginNameMarqueeSlide(now);
  Serial.println("DINARA marquee entrance slide started!");
}

void updateNameMarquee(unsigned long now) {
  nameScrollX -= NAME_SCROLL_SPEED;

  // Once the complete name has left the left edge, slide the pet eyes in.
  if (nameScrollX + nameTextWidth < -NAME_EXIT_MARGIN) {
    beginReturnToEyesSlide(now);
    return;
  }
}

void drawHeartShape(int centerX, int centerY, int radius, uint16_t color) {
  int lobeRadius = max(2, radius / 2);
  int lobeOffset = max(2, radius / 2);

  eyeCanvas->fillCircle(
    centerX - lobeOffset,
    centerY - lobeOffset / 2,
    lobeRadius,
    color
  );

  eyeCanvas->fillCircle(
    centerX + lobeOffset,
    centerY - lobeOffset / 2,
    lobeRadius,
    color
  );

  eyeCanvas->fillTriangle(
    centerX - radius - 1,
    centerY,
    centerX + radius + 1,
    centerY,
    centerX,
    centerY + radius + 2,
    color
  );
}

void drawPoppingHeart(int centerX, int centerY, int radius) {
  // Larger white heart underneath creates a clean outline.
  drawHeartShape(centerX, centerY, radius + 2, NAME_WHITE);
  drawHeartShape(centerX, centerY, radius, NAME_PINK);
}

void renderNameMarquee(unsigned long now, bool pushToDisplay) {
  eyeCanvas->fillScreen(BG_COLOR);

  // Hearts travel with the word and pulse at different phases.
  const float heartPositions[] = {
    0.08f, 0.25f, 0.43f, 0.62f, 0.82f, 0.96f
  };

  const int heartY[] = {
    43, 132, 35, 140, 48, 128
  };

  for (uint8_t i = 0; i < 6; i++) {
    float pulse = 0.5f + 0.5f * sinf(
      (float)now * 0.012f + (float)i * 1.25f
    );

    int radius = 3 + (int)(pulse * 4.0f);
    int heartX = (int)(nameScrollX +
                       nameTextWidth * heartPositions[i]);

    drawPoppingHeart(heartX, heartY[i], radius);
  }

  // Vertically center the custom font inside the LCD canvas.
  int16_t textTop = (CANVAS_H - nameTextHeight) / 2;
  int16_t baselineY = textTop - nameBoundsY;
  int16_t cursorX = (int16_t)nameScrollX - nameBoundsX;

  eyeCanvas->setFont(&FreeSansBoldOblique18pt7b);
  eyeCanvas->setTextSize(1);
  eyeCanvas->setTextWrap(false);

  // Draw a white 8-direction outline, then the pink face of the letters.
  const int8_t outlineX[] = { -2, 0, 2, -2, 2, -2, 0, 2 };
  const int8_t outlineY[] = { -2, -2, -2, 0, 0, 2, 2, 2 };

  eyeCanvas->setTextColor(NAME_WHITE);
  for (uint8_t i = 0; i < 8; i++) {
    eyeCanvas->setCursor(
      cursorX + outlineX[i],
      baselineY + outlineY[i]
    );
    eyeCanvas->print(NAME_TEXT);
  }

  eyeCanvas->setTextColor(NAME_PINK);
  eyeCanvas->setCursor(cursorX, baselineY);
  eyeCanvas->print(NAME_TEXT);

  eyeCanvas->setFont(NULL);

  if (pushToDisplay) {
    tft.drawRGBBitmap(
      CANVAS_X,
      CANVAS_Y,
      eyeCanvas->getBuffer(),
      CANVAS_W,
      CANVAS_H
    );
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

  // Debounce the TTP223B digital output.
  if (rawTouch != lastRawTouchState) {
    lastRawTouchState = rawTouch;
    rawTouchChangedAt = now;
  }

  bool stateChanged = false;

  if (rawTouch != stableTouchState &&
      now - rawTouchChangedAt >= TOUCH_DEBOUNCE_MS) {
    stableTouchState = rawTouch;
    stateChanged = true;
  }

  if (stateChanged) {
    if (stableTouchState == HIGH) {
      touchActive = true;
      touchStartedAt = now;
      longPressHandled = false;
      nameHoldHandled = false;

      Serial.println("TOUCH STARTED");
    } else {
      touchActive = false;

      unsigned long pressDuration = now - touchStartedAt;

      // Releasing during roulette or DINARA only releases the sensor.
      // Do not accidentally count that release as another tap/gesture.
      if (isRollingGacha || currentMood == MOOD_NAME) {
        Serial.print("GESTURE RELEASED: ");
        Serial.print(pressDuration);
        Serial.println(" ms");
      } else if (!longPressHandled) {
        // Only a short press counts as a tap.
        if (pressDuration < LONG_PRESS_MS) {
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
        }
      }
    }
  }

  // A continuous 3-second hold always takes priority and launches DINARA.
  if (touchActive &&
      !nameHoldHandled &&
      now - touchStartedAt >= NAME_HOLD_MS) {
    nameHoldHandled = true;
    longPressHandled = true;
    triggerNameMarquee(now);
    return;
  }

  // Start the eye roulette as soon as the hold reaches 700 ms.
  // The animation continues even if the user releases before it finishes.
  if (touchActive &&
      !longPressHandled &&
      now - touchStartedAt >= LONG_PRESS_MS) {
    longPressHandled = true;
    shortTapCount = 0;
    tapWindowStartedAt = 0;
    triggerRoulette(now);
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

void drawStarPupil(int centerX, int centerY, int outerRadius, uint16_t color) {
  int px[10];
  int py[10];

  for (int i = 0; i < 10; i++) {
    float angle = -1.5707963f + (float)i * 0.6283185f;
    int radius = (i % 2 == 0)
      ? outerRadius
      : (int)(outerRadius * 0.46f);

    px[i] = centerX + (int)(cosf(angle) * radius);
    py[i] = centerY + (int)(sinf(angle) * radius);
  }

  for (int i = 0; i < 10; i++) {
    int next = (i + 1) % 10;
    eyeCanvas->fillTriangle(
      centerX, centerY,
      px[i], py[i],
      px[next], py[next],
      color
    );
  }
}

void drawHeartPupil(int centerX, int centerY, int radius, uint16_t color) {
  int lobeRadius = max(4, radius / 2);
  int lobeOffset = max(4, radius / 2);

  eyeCanvas->fillCircle(
    centerX - lobeOffset,
    centerY - radius / 3,
    lobeRadius,
    color
  );

  eyeCanvas->fillCircle(
    centerX + lobeOffset,
    centerY - radius / 3,
    lobeRadius,
    color
  );

  eyeCanvas->fillTriangle(
    centerX - radius,
    centerY - 1,
    centerX + radius,
    centerY - 1,
    centerX,
    centerY + radius + 5,
    color
  );
}

void drawFirePupil(int centerX, int centerY, int halfWidth, int halfHeight) {
  // Outer flame.
  eyeCanvas->fillCircle(
    centerX, centerY + halfHeight / 4, halfWidth, 0xF800
  );
  eyeCanvas->fillTriangle(
    centerX - halfWidth,
    centerY + halfHeight / 4,
    centerX + halfWidth,
    centerY + halfHeight / 4,
    centerX,
    centerY - halfHeight,
    0xF800
  );

  // Yellow inner flame.
  int innerWidth = max(5, (int)(halfWidth * 0.55f));
  int innerHeight = max(7, (int)(halfHeight * 0.58f));

  eyeCanvas->fillCircle(
    centerX, centerY + innerHeight / 4, innerWidth, 0xFFE0
  );
  eyeCanvas->fillTriangle(
    centerX - innerWidth,
    centerY + innerHeight / 4,
    centerX + innerWidth,
    centerY + innerHeight / 4,
    centerX,
    centerY - innerHeight,
    0xFFE0
  );
}

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

  // Colored eye surface and its subtle outer outline.
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

  // Draw the selected pupil style. Pupil position follows gaze
  // while remaining inside the sclera. Pupils hide during a blink.
  if (eyeHeight > 18) {
    int pupilHalfWidth = 0;
    int pupilHalfHeight = 0;
    int shapeRadius = 0;
    int fireHalfWidth = 0;
    int fireHalfHeight = 0;

    switch (currentEyeStyle) {
      case EYE_NORMAL:
        pupilHalfWidth = max(9, (int)(eyeWidth * 0.18f));
        pupilHalfHeight = max(13, (int)(eyeHeight * 0.22f));
        break;

      case EYE_CUTE:
        pupilHalfWidth = max(6, (int)(eyeWidth * 0.12f));
        pupilHalfHeight = max(9, (int)(eyeHeight * 0.15f));
        break;

      case EYE_STAR:
        shapeRadius = min(
          (int)(eyeWidth * 0.25f),
          (int)(eyeHeight * 0.22f)
        );
        pupilHalfWidth = shapeRadius;
        pupilHalfHeight = shapeRadius;
        break;

      case EYE_HEART:
        shapeRadius = min(
          (int)(eyeWidth * 0.23f),
          (int)(eyeHeight * 0.18f)
        );
        pupilHalfWidth = shapeRadius;
        pupilHalfHeight = (int)(shapeRadius * 1.25f);
        break;

      case EYE_FIRE:
        fireHalfWidth = max(8, (int)(eyeWidth * 0.21f));
        fireHalfHeight = max(12, (int)(eyeHeight * 0.23f));
        pupilHalfWidth = fireHalfWidth;
        pupilHalfHeight = fireHalfHeight;
        break;

      default:
        pupilHalfWidth = 10;
        pupilHalfHeight = 15;
        break;
    }

    float maxPupilOffsetX = max(
      0.0f,
      (eyeWidth / 2.0f) - pupilHalfWidth - 3.0f
    );
    float maxPupilOffsetY = max(
      0.0f,
      (eyeHeight / 2.0f) - pupilHalfHeight - 3.0f
    );

    float pupilOffsetX = constrain(
      gazeX * 0.42f,
      -maxPupilOffsetX,
      maxPupilOffsetX
    );
    float pupilOffsetY = constrain(
      gazeY * 0.42f,
      -maxPupilOffsetY,
      maxPupilOffsetY
    );

    int pupilCenterX = (int)(
      x + eyeWidth / 2.0f + pupilOffsetX + 0.5f
    );
    int pupilCenterY = (int)(
      top + eyeHeight / 2.0f + pupilOffsetY + 0.5f
    );

    const EyeStyleConfig &style =
      eyeStyleConfig[(uint8_t)currentEyeStyle];

    switch (currentEyeStyle) {
      case EYE_NORMAL: {
        int pupilWidth = pupilHalfWidth * 2;
        int pupilHeight = pupilHalfHeight * 2;

        eyeCanvas->fillRoundRect(
          pupilCenterX - pupilHalfWidth,
          pupilCenterY - pupilHalfHeight,
          pupilWidth,
          pupilHeight,
          min(pupilHalfWidth, pupilHalfHeight),
          style.pupilColor
        );

        eyeCanvas->fillCircle(
          pupilCenterX - pupilHalfWidth / 3,
          pupilCenterY - pupilHalfHeight / 2,
          max(3, pupilHalfWidth / 3),
          style.highlightColor
        );
        break;
      }

      case EYE_CUTE: {
        int pupilWidth = pupilHalfWidth * 2;
        int pupilHeight = pupilHalfHeight * 2;

        eyeCanvas->fillRoundRect(
          pupilCenterX - pupilHalfWidth,
          pupilCenterY - pupilHalfHeight,
          pupilWidth,
          pupilHeight,
          min(pupilHalfWidth, pupilHalfHeight),
          style.pupilColor
        );

        eyeCanvas->fillCircle(
          pupilCenterX - pupilHalfWidth / 3,
          pupilCenterY - pupilHalfHeight / 2,
          max(2, pupilHalfWidth / 3),
          style.highlightColor
        );
        break;
      }

      case EYE_STAR:
        drawStarPupil(
          pupilCenterX, pupilCenterY,
          shapeRadius, style.pupilColor
        );
        break;

      case EYE_HEART:
        drawHeartPupil(
          pupilCenterX, pupilCenterY,
          shapeRadius, style.pupilColor
        );
        eyeCanvas->fillCircle(
          pupilCenterX - shapeRadius / 3,
          pupilCenterY - shapeRadius / 4,
          max(2, shapeRadius / 5),
          style.highlightColor
        );
        break;

      case EYE_FIRE:
        drawFirePupil(
          pupilCenterX, pupilCenterY,
          fireHalfWidth, fireHalfHeight
        );
        break;

      default:
        break;
    }
  }
}

void renderPetEyes(
  int eyeHeight,
  int gazeX,
  int gazeY,
  PetMood mood,
  bool pushToDisplay
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

  if (pushToDisplay) {
    tft.drawRGBBitmap(
      CANVAS_X,
      CANVAS_Y,
      eyeCanvas->getBuffer(),
      CANVAS_W,
      CANVAS_H
    );
  }
}

void beginSlideTransition(
  unsigned long now,
  unsigned long durationMs,
  SlideAction action
) {
  if (slideFromBuffer == nullptr || slideLineBuffer == nullptr) {
    // If allocation failed, fall back to displaying the target frame directly.
    tft.drawRGBBitmap(
      CANVAS_X, CANVAS_Y,
      eyeCanvas->getBuffer(),
      CANVAS_W, CANVAS_H
    );
    slideActive = false;
    currentSlideAction = action;
    finishSlideTransition(now);
    return;
  }

  slideStartedAt = now;
  slideDurationMs = max(1UL, durationMs);
  currentSlideAction = action;
  slideActive = true;
}

void beginEyeStyleSlide(
  EyeStyle nextStyle,
  unsigned long now,
  unsigned long durationMs,
  SlideAction action
) {
  if (nextStyle == currentEyeStyle) {
    return;
  }

  // Save the outgoing pair of eyes, then prepare the incoming style.
  memcpy(
    slideFromBuffer,
    eyeCanvas->getBuffer(),
    FRAME_BUFFER_BYTES
  );

  currentEyeStyle = nextStyle;

  renderPetEyes(
    lastRenderedHeight,
    lastRenderedGazeX,
    lastRenderedGazeY,
    currentMood,
    false
  );

  beginSlideTransition(now, durationMs, action);
}

void beginNameMarqueeSlide(unsigned long now) {
  // Complete a pending slide first, if the long-hold gesture arrives at its edge.
  if (slideActive) {
    tft.drawRGBBitmap(
      CANVAS_X, CANVAS_Y,
      eyeCanvas->getBuffer(),
      CANVAS_W, CANVAS_H
    );
    slideActive = false;
    currentSlideAction = SLIDE_NONE;
  }

  memcpy(
    slideFromBuffer,
    eyeCanvas->getBuffer(),
    FRAME_BUFFER_BYTES
  );

  currentMood = MOOD_NAME;

  // Show DINARA partly entering from the right side of the screen.
  int entryOffset = max(28, (int)(nameTextWidth * 0.40f));
  nameScrollX = CANVAS_W - entryOffset;

  renderNameMarquee(now, false);
  beginSlideTransition(now, SLIDE_NAME_MS, SLIDE_NAME_INTRO);
}

void beginReturnToEyesSlide(unsigned long now) {
  // Save the final marquee frame as the outgoing image.
  memcpy(
    slideFromBuffer,
    eyeCanvas->getBuffer(),
    FRAME_BUFFER_BYTES
  );

  currentMood = MOOD_IDLE;

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

  scheduleNextGaze(now);
  nextBlinkAt = now + random(900, 1800);

  // Prepare incoming eyes on the canvas, without showing them yet.
  renderPetEyes(
    BASE_EYE_HEIGHT,
    0,
    0,
    MOOD_IDLE,
    false
  );

  lastRenderedHeight = BASE_EYE_HEIGHT;
  lastRenderedGazeX = 0;
  lastRenderedGazeY = 0;
  lastRenderedMood = MOOD_IDLE;
  lastRenderedColor = getActiveEyeColor();
  lastRenderedEyeStyle = (int)currentEyeStyle;

  beginSlideTransition(now, SLIDE_NAME_MS, SLIDE_NAME_EXIT);
}

void drawSlideFrame(unsigned long now) {
  if (!slideActive || slideFromBuffer == nullptr ||
      slideLineBuffer == nullptr) {
    return;
  }

  float progress = (float)(now - slideStartedAt) /
                   (float)slideDurationMs;
  progress = constrain(progress, 0.0f, 1.0f);

  // Outgoing frame shifts left while the new frame enters from the right.
  int offset = (int)(progress * CANVAS_W + 0.5f);
  offset = constrain(offset, 0, CANVAS_W);

  int oldWidth = CANVAS_W - offset;
  uint16_t *target = eyeCanvas->getBuffer();

  tft.startWrite();
  tft.setAddrWindow(
    CANVAS_X,
    CANVAS_Y,
    CANVAS_W,
    CANVAS_H
  );

  for (int y = 0; y < CANVAS_H; y++) {
    const uint16_t *oldRow =
      slideFromBuffer + (size_t)y * CANVAS_W + offset;
    const uint16_t *newRow =
      target + (size_t)y * CANVAS_W;

    if (oldWidth > 0) {
      memcpy(
        slideLineBuffer,
        oldRow,
        (size_t)oldWidth * sizeof(uint16_t)
      );
    }

    if (offset > 0) {
      memcpy(
        slideLineBuffer + oldWidth,
        newRow,
        (size_t)offset * sizeof(uint16_t)
      );
    }

    tft.writePixels(slideLineBuffer, CANVAS_W, true, false);
  }

  tft.endWrite();
}

void finishSlideTransition(unsigned long now) {
  // Leave the exact target image on the display.
  tft.drawRGBBitmap(
    CANVAS_X,
    CANVAS_Y,
    eyeCanvas->getBuffer(),
    CANVAS_W,
    CANVAS_H
  );

  slideActive = false;

  SlideAction finishedAction = currentSlideAction;
  currentSlideAction = SLIDE_NONE;

  if (finishedAction == SLIDE_ROULETTE_FINAL) {
    isRollingGacha = false;
    rouletteResultUntil = now + 1100;

    scheduleNextGaze(now);
    nextBlinkAt = now + random(900, 1800);

    Serial.print("ROULETTE RESULT: ");
    Serial.println(getEyeStyleName(currentEyeStyle));
  } else if (finishedAction == SLIDE_NAME_INTRO) {
    currentMood = MOOD_NAME;
    Serial.println("DINARA marquee is now scrolling.");
  } else if (finishedAction == SLIDE_NAME_EXIT) {
    currentMood = MOOD_IDLE;
    Serial.println("DINARA marquee finished. Pet eyes slid back in.");
  }
}

void drawRouletteOverlay(unsigned long now) {
  tft.fillRect(0, 210, 240, 30, BG_COLOR);
  tft.setTextWrap(false);
  tft.setTextSize(1);
  tft.setTextColor(NAME_WHITE, BG_COLOR);

  if (isRollingGacha) {
    unsigned long elapsed = now - gachaStartTime;
    float progress = constrain(
      (float)elapsed / (float)GACHA_DURATION_MS,
      0.0f,
      1.0f
    );

    tft.setCursor(91, 214);
    tft.print("ROLLING...");

    tft.drawRoundRect(24, 228, 192, 7, 3, NAME_WHITE);
    int fillWidth = (int)(188.0f * progress);
    if (fillWidth > 0) {
      tft.fillRoundRect(26, 230, fillWidth, 3, 1, NAME_PINK);
    }
  } else {
    tft.setCursor(65, 218);
    tft.print("STYLE: ");
    tft.print(getEyeStyleName(currentEyeStyle));
  }

  rouletteOverlayDrawn = true;
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

  // Save one outgoing frame and compose each horizontal slide with a scanline.
  slideFromBuffer = (uint16_t *)malloc(FRAME_BUFFER_BYTES);
  slideLineBuffer = (uint16_t *)malloc(CANVAS_W * sizeof(uint16_t));

  if (slideFromBuffer == nullptr || slideLineBuffer == nullptr) {
    Serial.println("ERROR: Slide transition buffer allocation failed!");

    while (true) {
      delay(1000);
    }
  }

  // Measure DINARA using the bundled Adafruit GFX font.
  eyeCanvas->setFont(&FreeSansBoldOblique18pt7b);
  eyeCanvas->setTextSize(1);
  eyeCanvas->getTextBounds(
    NAME_TEXT,
    0,
    0,
    &nameBoundsX,
    &nameBoundsY,
    &nameTextWidth,
    &nameTextHeight
  );
  eyeCanvas->setFont(NULL);

  randomSeed((unsigned long)micros());

  unsigned long now = millis();

  nextGazeAt = now + 700;
  nextBlinkAt = now + random(1800, 3500);

  Serial.println();
  Serial.println("DIGITAL PET V3.2 - HORIZONTAL SLIDING CAROUSEL");
  Serial.println("TTP223B active HIGH on GPIO 4");
  Serial.println("1-4 short taps: happy reaction");
  Serial.println("5 short taps within 3 seconds: fear reaction");
  Serial.println("Hold 700 ms: eye styles slide horizontally, then roulette slows to a result");
  Serial.println("Continuous hold for 3 seconds: DINARA marquee");
  Serial.println("Eye styles: NORMAL, CUTE, STAR, HEART, FIRE");
  Serial.println("3-second continuous hold: DINARA marquee has priority.");
  Serial.println("Eye styles slide left; each next style slides in from the right.");
  Serial.println("DINARA entrance and return to eyes also use horizontal slides.");
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
  lastRenderedEyeStyle = (int)currentEyeStyle;
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

  // Normal marquee motion runs only when a horizontal transition is idle.
  if (currentMood == MOOD_NAME && !slideActive) {
    updateNameMarquee(now);

    if (currentMood == MOOD_NAME && !slideActive) {
      renderNameMarquee(now, true);
      return;
    }
  }

  // Roulette changes styles only after the previous slide has completed.
  if (isRollingGacha && !slideActive) {
    updateRoulette(now);
  }

  // Shared renderer for style changes, DINARA entrance, and eye return.
  if (slideActive) {
    drawSlideFrame(now);

    if (now - slideStartedAt >= slideDurationMs) {
      finishSlideTransition(now);
    }

    if (isRollingGacha ||
        (long)(rouletteResultUntil - now) > 0) {
      drawRouletteOverlay(now);
    } else if (rouletteOverlayDrawn) {
      tft.fillRect(0, 210, 240, 30, BG_COLOR);
      rouletteOverlayDrawn = false;
    }

    return;
  }

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
      currentMood != lastRenderedMood ||
      (int)currentEyeStyle != lastRenderedEyeStyle) {

    renderPetEyes(
      renderHeight,
      renderX,
      renderY,
      currentMood,
      true
    );

    lastRenderedHeight = renderHeight;
    lastRenderedGazeX = renderX;
    lastRenderedGazeY = renderY;
    lastRenderedColor = activeColor;
    lastRenderedMood = currentMood;
    lastRenderedEyeStyle = (int)currentEyeStyle;
  }

  // Roulette status remains visible outside the eye canvas.
  if (isRollingGacha || (long)(rouletteResultUntil - now) > 0) {
    drawRouletteOverlay(now);
  } else if (rouletteOverlayDrawn) {
    tft.fillRect(0, 210, 240, 30, BG_COLOR);
    rouletteOverlayDrawn = false;
  }
}
