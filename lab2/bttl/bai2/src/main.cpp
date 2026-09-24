#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;
constexpr uint8_t LED_1 = 4;
constexpr uint8_t LED_2 = 5;
constexpr uint32_t DEBOUNCE_MS = 30;

bool ledState2 = false;
int lastReading = HIGH;
int stableButton = HIGH;
uint32_t lastButtonChange = 0;

void updateButton(uint32_t now)
{
    const int reading = digitalRead(BUTTON_PIN);
    if (reading != lastReading)
    {
        lastReading = reading;
        lastButtonChange = now;
    }
    // Chi dao LED khi nhan xuong on dinh, khong lap lai khi giu nut.
    if (now - lastButtonChange >= DEBOUNCE_MS && reading != stableButton)
    {
        stableButton = reading;
        if (stableButton == LOW)
        {
            ledState2 = !ledState2;
            digitalWrite(LED_1, ledState2);
            digitalWrite(LED_2, ledState2);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop()
{
    updateButton(millis());
}
