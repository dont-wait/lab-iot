#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

enum class StopwatchState {
  STOPPED,
  RUNNING,
};

constexpr uint8_t BUTTON_A_PIN = 18;
constexpr uint8_t BUTTON_B_PIN = 19;
constexpr uint8_t LCD_ADDRESS = 0x27;
constexpr uint8_t LCD_COLUMNS = 16;
constexpr uint8_t LCD_ROWS = 2;

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

StopwatchState state = StopwatchState::STOPPED;
unsigned long accumulatedMilliseconds = 0;
unsigned long startedAt = 0;
bool lastButtonAState = HIGH;
bool lastButtonBState = HIGH;

unsigned long elapsedMilliseconds() {
  if (state == StopwatchState::RUNNING) {
    return accumulatedMilliseconds + (millis() - startedAt);
  }
  return accumulatedMilliseconds;
}

void displayTime() {
  const unsigned long elapsed = elapsedMilliseconds();
  const unsigned long minutes = (elapsed / 60000UL) % 100UL;
  const unsigned long seconds = (elapsed / 1000UL) % 60UL;
  const unsigned long milliseconds = elapsed % 1000UL;

  char timeText[10];
  snprintf(timeText, sizeof(timeText), "%02lu:%02lu:%03lu",
           minutes, seconds, milliseconds);

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(3, 0);
  lcd.print(timeText);

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(3, 1);
  lcd.print(state == StopwatchState::RUNNING ? "RUNNING" : "PAUSED");
}

void toggleStopwatch() {
  if (state == StopwatchState::STOPPED) {
    startedAt = millis();
    state = StopwatchState::RUNNING;
  } else {
    accumulatedMilliseconds += millis() - startedAt;
    state = StopwatchState::STOPPED;
  }
  displayTime();
}

void resetStopwatch() {
  if (state != StopwatchState::STOPPED) {
    return;
  }

  accumulatedMilliseconds = 0;
  displayTime();
}

void setup() {
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  displayTime();
}

void loop() {
  const bool buttonAState = digitalRead(BUTTON_A_PIN);
  const bool buttonBState = digitalRead(BUTTON_B_PIN);

  // Nút A: nhấn lần đầu để chạy, nhấn lần nữa để tạm dừng.
  if (lastButtonAState == HIGH && buttonAState == LOW) {
    toggleStopwatch();
  }

  // Nút B chỉ reset khi đồng hồ đang dừng/tạm dừng.
  if (lastButtonBState == HIGH && buttonBState == LOW) {
    resetStopwatch();
  }

  lastButtonAState = buttonAState;
  lastButtonBState = buttonBState;

  if (state == StopwatchState::RUNNING) {
    displayTime();
  }

  delay(10);
}
