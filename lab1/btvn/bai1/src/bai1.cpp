#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t DHT_TYPE = DHT22;
constexpr uint8_t BUTTON_PIN = 18;
constexpr uint8_t LED_PIN = 2;
constexpr uint8_t LCD_ADDRESS = 0x27;
constexpr uint8_t LCD_COLUMNS = 16;
constexpr uint8_t LCD_ROWS = 2;
constexpr unsigned long DHT_READ_INTERVAL = 2000UL;
constexpr unsigned long FAHRENHEIT_TIME = 5000UL;

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
float temperatureC = NAN;
float humidity = NAN;
unsigned long lastDhtRead = 0;
unsigned long fahrenheitStartedAt = 0;
bool showFahrenheit = false;
bool lastButtonState = HIGH;

void clearLine(uint8_t row) {
  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
}

void showSensorData() {
  clearLine(0);
  clearLine(1);
  if (isnan(temperatureC) || isnan(humidity)) {
    lcd.setCursor(0, 0);
    lcd.print("Loi cam bien");
    lcd.setCursor(0, 1);
    lcd.print("Dang cho doc...");
    return;
  }
  const float displayedTemperature = showFahrenheit
      ? (temperatureC * 1.8f) + 32.0f
      : temperatureC;
  lcd.setCursor(0, 0);
  lcd.print("Nhiet do:");
  lcd.print(displayedTemperature, 1);
  lcd.write((uint8_t)223);
  lcd.print(showFahrenheit ? "F" : "C");
  lcd.setCursor(0, 1);
  lcd.print("Do am:");
  lcd.print(humidity, 1);
  lcd.print("%");
}

void readSensor() {
  const float newHumidity = dht.readHumidity();
  const float newTemperatureC = dht.readTemperature();
  if (!isnan(newHumidity) && !isnan(newTemperatureC)) {
    humidity = newHumidity;
    temperatureC = newTemperatureC;
    digitalWrite(LED_PIN, humidity > 80.0f ? HIGH : LOW);
  }
  showSensorData();
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  lcd.init();
  lcd.backlight();
  dht.begin();
  showSensorData();
  readSensor();
  lastDhtRead = millis();
}

void loop() {
  const unsigned long now = millis();
  const bool buttonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && buttonState == LOW) {
    showFahrenheit = true;
    fahrenheitStartedAt = now;
    showSensorData();
  }
  lastButtonState = buttonState;
  if (showFahrenheit && now - fahrenheitStartedAt >= FAHRENHEIT_TIME) {
    showFahrenheit = false;
    showSensorData();
  }
  if (now - lastDhtRead >= DHT_READ_INTERVAL) {
    lastDhtRead = now;
    readSensor();
  }
}
