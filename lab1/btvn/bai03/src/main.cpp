#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// =======================
// Pin configuration
// =======================

#define DHT_PIN 4
#define DHT_TYPE DHT22
#define BUTTON_PIN 15

// =======================
// Components
// =======================

DHT dht(DHT_PIN, DHT_TYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

// =======================
// Menu
// =======================

int page = 1;

// =======================
// Setup
// =======================

void setup() {
    Serial.begin(115200);

    // LCD
    lcd.init();
    lcd.backlight();

    // DHT22
    dht.begin();

    // Button
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Initial screen
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DHT22 Menu");
    delay(1000);
}

// =======================
// Display Page
// =======================

void showPage() {
    lcd.clear();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    switch (page) {

        // ===================
        // Page 1
        // ===================

        case 1:
            lcd.setCursor(0, 0);
            lcd.print("Trang 1");

            lcd.setCursor(0, 1);

            if (isnan(temperature)) {
                lcd.print("Temp: Error");
            } else {
                lcd.print("Nhiet do: ");
                lcd.print(temperature, 1);
                lcd.print((char)223);
                lcd.print("C");
            }

            break;

        // ===================
        // Page 2
        // ===================

        case 2:
            lcd.setCursor(0, 0);
            lcd.print("Trang 2");

            lcd.setCursor(0, 1);

            if (isnan(humidity)) {
                lcd.print("Humidity: Error");
            } else {
                lcd.print("Do am: ");
                lcd.print(humidity, 1);
                lcd.print("%");
            }

            break;

        // ===================
        // Page 3
        // ===================

        case 3:
            lcd.setCursor(0, 0);
            lcd.print("Trang 3");

            lcd.setCursor(0, 1);
            lcd.print("Trang thai: OK");

            break;
    }
}

// =======================
// Main loop
// =======================

void loop() {

    // Button pressed
    if (digitalRead(BUTTON_PIN) == LOW) {

        page++;

        if (page > 3) {
            page = 1;
        }

        showPage();

        // Simple debounce
        delay(200);

        // Wait until button released
        while (digitalRead(BUTTON_PIN) == LOW) {
            delay(10);
        }
    }
}