#define TOUCH_PIN 4

int lastTouchState = LOW;

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT);

  Serial.println();
  Serial.println("DIGITAL PET - TOUCH SENSOR TEST");
  Serial.println("Touch the round sensor pad...");
  Serial.println("Waiting for input...");
}

void loop() {
  int touchState = digitalRead(TOUCH_PIN);

  if (touchState != lastTouchState) {
    delay(25);
    touchState = digitalRead(TOUCH_PIN);

    if (touchState != lastTouchState) {
      lastTouchState = touchState;

      if (touchState == HIGH) {
        Serial.println("TOUCH DETECTED!");
      } else {
        Serial.println("TOUCH RELEASED!");
      }
    }
  }

  delay(5);
}
