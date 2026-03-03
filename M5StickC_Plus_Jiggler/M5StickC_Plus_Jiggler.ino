#include <M5StickCPlus.h>
#include <BleMouse.h>
#include <Preferences.h>

BleMouse bleMouse("M5StickC Plus Jiggler", "M5Jiggler", 100);
Preferences prefs;

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
bool btnBLongPressed = false;
bool keepScreenAwake = false;

enum JigglePhase { JIGGLE_IDLE, JIGGLE_RETURN };
JigglePhase jigglePhase = JIGGLE_IDLE;
unsigned long jigglePhaseTime = 0;
const unsigned long jiggleReturnDelay = 70;

unsigned long lastUIRefresh = 0;
unsigned long lastProgressRefresh = 0;
const unsigned long uiRefreshInterval = 1000;
const unsigned long progressRefreshInterval = 100;
int lastProgressWidth = -1;
bool lastConnectedState = false;

const int barWidth = 230;
const int barHeight = 10;
const int barX = 5;
const int barY = 125;
const uint16_t bgTop = 0x10A2;
const uint16_t bgBottom = 0x0000;
const uint16_t cardColor = 0x18E3;
const uint16_t cardEdge = 0x39A7;
const uint16_t accentBlue = 0x04DF;
const uint16_t accentMint = 0x3FB5;
const uint16_t accentAmber = 0xFD20;

void drawStaticLayout();
void drawConnectionStatus(bool connected);
void drawBatteryStatus();
void drawJiggleStatus();
void drawSettings();
void drawControls();
void drawCountdown();
void drawProgressBar();
void wakeScreen();
void sleepScreenIfIdle();
void startJiggle();
void runJiggleStateMachine();
void loadSettings();
void saveSettings();
const char* formatInterval(unsigned long ms);
void updateDisplay(bool forceFull = false);
void drawMouseIcon(int x, int y, uint16_t color);
void drawBleIcon(int x, int y, uint16_t color);
void drawBatteryIcon(int x, int y, int pct, uint16_t color);
void drawCard(int x, int y, int w, int h);

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Axp.ScreenBreath(defaultBrightness);
  M5.Lcd.fillScreen(TFT_BLACK);

  loadSettings();
  bleMouse.begin();

  lastInteractionTime = millis();
  lastJiggleTime = millis();
  updateDisplay(true);
}

void loop() {
  unsigned long now = millis();
  M5.update();

  // BtnA short press toggles ON/OFF, long press cycles distance.
  if (M5.BtnA.wasPressed()) {
    wakeScreen();
    btnALongPressed = false;
  }

  if (M5.BtnA.pressedFor(1000) && !btnALongPressed) {
    btnALongPressed = true;
    distanceIndex = (distanceIndex + 1) % numDistances;
    jiggleDistance = jiggleDistances[distanceIndex];
    saveSettings();
    wakeScreen();
    updateDisplay(true);
  }

  if (M5.BtnA.wasReleased()) {
    if (!btnALongPressed) {
      jiggleActive = !jiggleActive;
      if (jiggleActive) {
        lastJiggleTime = now;
      }
      wakeScreen();
      updateDisplay(true);
    }
    btnALongPressed = false;
  }

  // BtnB short press cycles interval, long press toggles sleep policy.
  if (M5.BtnB.wasPressed()) {
    wakeScreen();
    btnBLongPressed = false;
  }

  if (M5.BtnB.pressedFor(1000) && !btnBLongPressed) {
    btnBLongPressed = true;
    keepScreenAwake = !keepScreenAwake;
    wakeScreen();
    updateDisplay(true);
  }

  if (M5.BtnB.wasReleased()) {
    if (!btnBLongPressed) {
      intervalIndex = (intervalIndex + 1) % numIntervals;
      jiggleInterval = intervals[intervalIndex];
      saveSettings();
      wakeScreen();
      updateDisplay(true);
    }
    btnBLongPressed = false;
  }

  runJiggleStateMachine();

  // Trigger a new jiggle cycle when interval is reached.
  if (jiggleActive && jigglePhase == JIGGLE_IDLE && now - lastJiggleTime >= jiggleInterval) {
    startJiggle();
    lastJiggleTime = now;
  }

  // Refresh status periodically and on BLE state transitions.
  if (!screenIsOff && (now - lastUIRefresh >= uiRefreshInterval || bleMouse.isConnected() != lastConnectedState)) {
    updateDisplay(false);
    lastUIRefresh = now;
  }

  if (!screenIsOff && now - lastProgressRefresh >= progressRefreshInterval) {
    drawProgressBar();
    lastProgressRefresh = now;
  }

  sleepScreenIfIdle();
}

void wakeScreen() {
  if (screenIsOff) {
    M5.Axp.ScreenBreath(defaultBrightness);
    screenIsOff = false;
    updateDisplay(true);
  }
  lastInteractionTime = millis();
}

void sleepScreenIfIdle() {
  if (screenIsOff || keepScreenAwake) return;

  if (millis() - lastInteractionTime > screenSleepTimeout) {
    M5.Axp.ScreenBreath(0);
    screenIsOff = true;
  }
}

void startJiggle() {
  if (!bleMouse.isConnected()) return;

  int dy = (millis() / 1000) % 2 == 0 ? 1 : -1;
  bleMouse.move(jiggleDistance, dy);
  jigglePhase = JIGGLE_RETURN;
  jigglePhaseTime = millis();
}

void runJiggleStateMachine() {
  if (jigglePhase == JIGGLE_RETURN && millis() - jigglePhaseTime >= jiggleReturnDelay) {
    int dy = (millis() / 1000) % 2 == 0 ? -1 : 1;
    bleMouse.move(-jiggleDistance, dy);
    jigglePhase = JIGGLE_IDLE;
  }
}

void loadSettings() {
  prefs.begin("jiggler", true);
  int savedInterval = prefs.getInt("interval_idx", 0);
  int savedDistance = prefs.getInt("distance_idx", 0);
  prefs.end();

  if (savedInterval >= 0 && savedInterval < numIntervals) {
    intervalIndex = savedInterval;
  }

  if (savedDistance >= 0 && savedDistance < numDistances) {
    distanceIndex = savedDistance;
  }

  jiggleInterval = intervals[intervalIndex];
  jiggleDistance = jiggleDistances[distanceIndex];
}

void saveSettings() {
  prefs.begin("jiggler", false);
  prefs.putInt("interval_idx", intervalIndex);
  prefs.putInt("distance_idx", distanceIndex);
  prefs.end();
}

const char* formatInterval(unsigned long ms) {
  switch (ms) {
    case 5000: return "5s";
    case 15000: return "15s";
    case 30000: return "30s";
    case 60000: return "1m";
    case 300000: return "5m";
    case 600000: return "10m";
    default: return "?";
  }
}

void updateDisplay(bool forceFull) {
  if (screenIsOff) return;

  bool connected = bleMouse.isConnected();

  if (forceFull) {
    drawStaticLayout();
    drawControls();
  }

  drawConnectionStatus(connected);
  drawBatteryStatus();
  drawJiggleStatus();
  drawSettings();
  drawCountdown();
  drawProgressBar();

  lastConnectedState = connected;
}

void drawStaticLayout() {
  // Vertical gradient background.
  for (int y = 0; y < 135; ++y) {
    uint8_t mix = (uint8_t)((y * 255) / 134);
    uint8_t r1 = (bgTop >> 11) & 0x1F;
    uint8_t g1 = (bgTop >> 5) & 0x3F;
    uint8_t b1 = bgTop & 0x1F;
    uint8_t r2 = (bgBottom >> 11) & 0x1F;
    uint8_t g2 = (bgBottom >> 5) & 0x3F;
    uint8_t b2 = bgBottom & 0x1F;
    uint8_t r = (uint8_t)((r1 * (255 - mix) + r2 * mix) / 255);
    uint8_t g = (uint8_t)((g1 * (255 - mix) + g2 * mix) / 255);
    uint8_t b = (uint8_t)((b1 * (255 - mix) + b2 * mix) / 255);
    uint16_t c = (r << 11) | (g << 5) | b;
    M5.Lcd.drawFastHLine(0, y, 240, c);
  }

  drawCard(4, 4, 232, 20);
  drawCard(4, 28, 232, 42);
  drawCard(4, 74, 232, 42);

  drawMouseIcon(10, 8, accentBlue);

  M5.Lcd.setTextColor(TFT_WHITE, cardColor);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(26, 8);
  M5.Lcd.print("Mouse Jiggler");
}

void drawConnectionStatus(bool connected) {
  M5.Lcd.fillRect(144, 6, 90, 16, cardColor);
  drawBleIcon(146, 9, connected ? accentMint : TFT_RED);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(155, 10);
  M5.Lcd.setTextColor(connected ? accentMint : TFT_RED, cardColor);
  M5.Lcd.print(connected ? "CONNECTED" : "WAITING");
}

void drawBatteryStatus() {
  int voltageMv = (int)(M5.Axp.GetBatVoltage() * 1000.0f);
  int pct = (voltageMv - 3300) * 100 / (4200 - 3300);

  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;

  uint16_t color = pct > 40 ? TFT_GREEN : (pct > 20 ? TFT_YELLOW : TFT_RED);

  M5.Lcd.fillRect(10, 32, 100, 12, cardColor);
  drawBatteryIcon(10, 33, pct, color);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(color, cardColor);
  M5.Lcd.setCursor(32, 35);
  M5.Lcd.printf("%d%%", pct);
}

void drawJiggleStatus() {
  M5.Lcd.fillRect(10, 47, 220, 18, cardColor);
  uint16_t pillColor = jiggleActive ? accentMint : 0xA104;
  M5.Lcd.fillRoundRect(12, 48, 90, 16, 8, pillColor);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_BLACK, pillColor);
  M5.Lcd.setCursor(30, 53);
  M5.Lcd.print(jiggleActive ? "ACTIVE" : "PAUSED");

  int pulse = 3 + ((millis() / 250) % 3);
  M5.Lcd.fillCircle(112, 56, pulse, jiggleActive ? accentMint : TFT_DARKGREY);

  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(124, 47);
  M5.Lcd.setTextColor(TFT_WHITE, cardColor);
  M5.Lcd.print(jiggleActive ? "ON" : "OFF");
}

void drawSettings() {
  M5.Lcd.fillRect(10, 79, 220, 31, cardColor);

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_WHITE, cardColor);
  M5.Lcd.setCursor(12, 81);
  M5.Lcd.printf("INT %s", formatInterval(jiggleInterval));
  M5.Lcd.setCursor(116, 81);
  M5.Lcd.printf("DIST %d", jiggleDistance);

  M5.Lcd.setCursor(12, 98);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(keepScreenAwake ? accentAmber : TFT_LIGHTGREY, cardColor);
  M5.Lcd.print(keepScreenAwake ? "Display lock: ON" : "Display lock: AUTO");
}

void drawControls() {
  M5.Lcd.fillRect(0, 112, 240, 11, TFT_BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(0xAD55, TFT_BLACK);
  M5.Lcd.setCursor(4, 114);
  M5.Lcd.print("A Toggle/Hold Dist | B Interval/Hold Screen");
}

void drawCountdown() {
  M5.Lcd.fillRect(158, 32, 72, 12, cardColor);
  M5.Lcd.setTextSize(1);

  if (!jiggleActive) {
    M5.Lcd.setTextColor(TFT_LIGHTGREY, cardColor);
    M5.Lcd.setCursor(160, 35);
    M5.Lcd.print("Next: --");
    return;
  }

  unsigned long elapsed = millis() - lastJiggleTime;
  unsigned long remaining = elapsed >= jiggleInterval ? 0 : (jiggleInterval - elapsed);

  M5.Lcd.setTextColor(TFT_WHITE, cardColor);
  M5.Lcd.setCursor(160, 35);
  M5.Lcd.printf("Next:%lus", remaining / 1000);
}

void drawProgressBar() {
  if (screenIsOff) return;

  int progressWidth = 0;

  if (jiggleActive) {
    unsigned long elapsed = millis() - lastJiggleTime;
    if (elapsed > jiggleInterval) elapsed = jiggleInterval;
    progressWidth = map(elapsed, 0, jiggleInterval, 0, barWidth);
  }

  if (progressWidth == lastProgressWidth) return;

  uint16_t barColor = jiggleActive ? (bleMouse.isConnected() ? TFT_GREEN : TFT_ORANGE) : TFT_DARKGREY;

  M5.Lcd.fillRoundRect(barX, barY, barWidth, barHeight, 5, 0x2965);
  if (progressWidth > 0) {
    M5.Lcd.fillRoundRect(barX, barY, progressWidth, barHeight, 5, barColor);
  }
  M5.Lcd.drawRoundRect(barX, barY, barWidth, barHeight, 5, 0x8C71);

  lastProgressWidth = progressWidth;
}

void drawMouseIcon(int x, int y, uint16_t color) {
  M5.Lcd.drawRoundRect(x, y, 12, 14, 4, color);
  M5.Lcd.drawFastVLine(x + 6, y + 2, 4, color);
  M5.Lcd.fillCircle(x + 6, y + 8, 1, color);
}

void drawBleIcon(int x, int y, uint16_t color) {
  M5.Lcd.drawLine(x + 3, y, x + 3, y + 10, color);
  M5.Lcd.drawLine(x + 3, y, x + 8, y + 3, color);
  M5.Lcd.drawLine(x + 3, y + 10, x + 8, y + 7, color);
  M5.Lcd.drawLine(x + 3, y + 5, x + 8, y + 1, color);
  M5.Lcd.drawLine(x + 3, y + 5, x + 8, y + 9, color);
}

void drawBatteryIcon(int x, int y, int pct, uint16_t color) {
  M5.Lcd.drawRect(x, y, 18, 8, TFT_LIGHTGREY);
  M5.Lcd.fillRect(x + 18, y + 2, 2, 4, TFT_LIGHTGREY);
  int fill = map(pct, 0, 100, 0, 16);
  if (fill > 0) M5.Lcd.fillRect(x + 1, y + 1, fill, 6, color);
}

void drawCard(int x, int y, int w, int h) {
  M5.Lcd.fillRoundRect(x, y, w, h, 7, cardColor);
  M5.Lcd.drawRoundRect(x, y, w, h, 7, cardEdge);
}
