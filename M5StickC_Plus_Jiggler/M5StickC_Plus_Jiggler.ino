#include <M5StickCPlus.h>
#include <BleMouse.h>

BleMouse bleMouse;

// Define intervals in milliseconds
const unsigned long intervals[] = {5000, 15000, 30000, 60000, 300000, 600000};
const int numIntervals = sizeof(intervals) / sizeof(intervals[0]);
int intervalIndex = 0;
unsigned long jiggleInterval = intervals[intervalIndex];

// Define jiggle distances
const int jiggleDistances[] = {5, 10, 25, 50};
const int numDistances = sizeof(jiggleDistances) / sizeof(jiggleDistances[0]);
int distanceIndex = 0;
int jiggleDistance = jiggleDistances[distanceIndex];

bool jiggleActive = false;
unsigned long lastJiggleTime = 0;
unsigned long lastInteractionTime = 0;
const unsigned long screenSleepTimeout = 30000; // 30 seconds
const int defaultBrightness = 10;
bool screenIsOff = false;
bool btnALongPressed = false;

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Axp.ScreenBreath(defaultBrightness);
  M5.Lcd.fillScreen(BLACK);
  bleMouse.begin();
  updateDisplay();
}

void wakeScreen() {
  if (screenIsOff) {
    M5.Axp.ScreenBreath(defaultBrightness);
    screenIsOff = false;
    updateDisplay();
  }
  lastInteractionTime = millis();
}

void loop() {
  M5.update();

  // Handle BtnA
  if (M5.BtnA.wasPressed()) {
    wakeScreen();
    btnALongPressed = false;
  }

  if (M5.BtnA.pressedFor(1000) && !btnALongPressed) {
    btnALongPressed = true;
    distanceIndex = (distanceIndex + 1) % numDistances;
    jiggleDistance = jiggleDistances[distanceIndex];
    wakeScreen();
    updateDisplay();
  }

  if (M5.BtnA.wasReleased()) {
    if (!btnALongPressed) {
      jiggleActive = !jiggleActive;
      wakeScreen();
      updateDisplay();
    }
    btnALongPressed = false;
  }

  // Handle BtnB press (change interval)
  if (M5.BtnB.wasPressed()) {
    intervalIndex = (intervalIndex + 1) % numIntervals;
    jiggleInterval = intervals[intervalIndex];
    wakeScreen();
    updateDisplay();
  }

  // Perform jiggle if active and interval has elapsed
  if (jiggleActive && millis() - lastJiggleTime >= jiggleInterval) {
    performJiggle();
    lastJiggleTime = millis();
    // Jiggle also counts as interaction to keep screen alive during active jiggling if desired,
    // but usually we want it to sleep anyway. Let's not wake screen on jiggle unless we want to see it.
    // However, the original code called updateDisplay() in performJiggle() which would redraw.
  }

  // Handle screen sleep
  if (!screenIsOff && millis() - lastInteractionTime > screenSleepTimeout) {
    M5.Axp.ScreenBreath(0);
    screenIsOff = true;
  }

  // Smooth progress bar update
  if (!screenIsOff) {
    drawProgressBar();
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

  if (!screenIsOff) {
    updateDisplay();
  }
}

void updateDisplay() {
  if (screenIsOff) return;

  M5.Lcd.fillScreen(BLACK);

  // Title
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(45, 5);
  M5.Lcd.print("Mouse Jiggler");

  // Connection Status
  bool connected = bleMouse.isConnected();
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(160, 25);
  M5.Lcd.setTextColor(connected ? GREEN : RED);
  M5.Lcd.printf(connected ? "Connected" : "Disconnected");

  // Display jiggle status
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.setTextColor(jiggleActive ? GREEN : RED);
  M5.Lcd.printf("STATUS: %s", jiggleActive ? "ON" : "OFF");

  // Display interval and distance
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 80);
  M5.Lcd.printf("Interval: %lu s", jiggleInterval / 1000);
  M5.Lcd.setCursor(10, 105);
  M5.Lcd.printf("Distance: %d px", jiggleDistance);

  drawProgressBar();
}

int lastProgressWidth = -1;

void drawProgressBar() {
  if (screenIsOff) return;

  unsigned long elapsed = millis() - lastJiggleTime;
  if (elapsed > jiggleInterval) elapsed = jiggleInterval;

  int barWidth = 230;
  int barHeight = 10;
  int barX = 5;
  int barY = 125;

  int progressWidth = map(elapsed, 0, jiggleInterval, 0, barWidth);

  if (progressWidth != lastProgressWidth) {
    // Only redraw the bar area to avoid flickering and unnecessary bus traffic
    M5.Lcd.fillRect(barX, barY, progressWidth, barHeight, YELLOW);
    M5.Lcd.fillRect(barX + progressWidth, barY, barWidth - progressWidth, barHeight, BLACK);
    M5.Lcd.drawRect(barX, barY, barWidth, barHeight, WHITE);
    lastProgressWidth = progressWidth;
  }
}
