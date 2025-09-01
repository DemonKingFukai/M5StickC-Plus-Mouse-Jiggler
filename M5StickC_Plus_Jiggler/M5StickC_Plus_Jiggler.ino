#include <M5StickCPlus.h>
#include <BleMouse.h>

BleMouse bleMouse;

// Define intervals in milliseconds
const unsigned long intervals[] = {5000, 15000, 30000, 60000, 300000, 600000};
int intervalIndex = 0;
unsigned long jiggleInterval = intervals[intervalIndex];

// Define jiggle distances
const int jiggleDistances[] = {5, 10, 25, 50};
int distanceIndex = 0;
int jiggleDistance = jiggleDistances[distanceIndex];

bool jiggleActive = false;
unsigned long lastJiggleTime = 0;
unsigned long lastInteractionTime = 0;
const unsigned long screenSleepTimeout = 30000; // 30 seconds

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  bleMouse.begin();
  updateDisplay();
}

void loop() {
  M5.update();

  // Handle BtnA press (toggle jiggle)
  if (M5.BtnA.wasPressed()) {
    jiggleActive = !jiggleActive;
    lastInteractionTime = millis();
    updateDisplay();
  }

  // Handle BtnB press (change interval)
  if (M5.BtnB.wasPressed()) {
    intervalIndex = (intervalIndex + 1) % 6;
    jiggleInterval = intervals[intervalIndex];
    lastInteractionTime = millis();
    updateDisplay();
  }

  // Handle long press of BtnA (change distance)
  if (M5.BtnA.isPressed() && millis() - lastInteractionTime > 1000) {
    distanceIndex = (distanceIndex + 1) % 4;
    jiggleDistance = jiggleDistances[distanceIndex];
    lastInteractionTime = millis();
    updateDisplay();
  }

  // Perform jiggle if active and interval has elapsed
  if (jiggleActive && millis() - lastJiggleTime >= jiggleInterval) {
    performJiggle();
    lastJiggleTime = millis();
  }

  // Handle screen sleep
  if (millis() - lastInteractionTime > screenSleepTimeout) {
    M5.Lcd.fillScreen(BLACK);
  }
}

void performJiggle() {
  if (!bleMouse.isConnected()) return;

  // Move mouse right
  bleMouse.move(jiggleDistance, 0);
  delay(50); // Small delay for smooth motion

  // Move mouse left
  bleMouse.move(-jiggleDistance, 0);
  delay(50); // Small delay for smooth motion

  updateDisplay();
}

void updateDisplay() {
  M5.Lcd.fillScreen(BLACK);

  // Display jiggle status
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(jiggleActive ? GREEN : RED);
  M5.Lcd.printf("Jiggle: %s\n", jiggleActive ? "ON" : "OFF");

  // Display interval
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.printf("Interval: %lu s\n", jiggleInterval / 1000);

  // Display distance
  M5.Lcd.printf("Distance: %d px\n", jiggleDistance);

  // Display progress bar
  unsigned long elapsed = millis() - lastJiggleTime;
  int progressWidth = map(elapsed, 0, jiggleInterval, 0, 120);
  M5.Lcd.fillRect(5, 60, progressWidth, 8, YELLOW);
  M5.Lcd.drawRect(5, 60, 120, 8, WHITE);
}