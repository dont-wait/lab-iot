#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;
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
            digitalWrite(LED_2, ledState2);
        }
    }
}

constexpr uint8_t LED_1 = 4;
constexpr uint32_t BLINK_MS = 300;
constexpr uint32_t REPORT_MS = 1000;
bool ledState1 = false;
uint32_t lastBlink = 0;
uint32_t lastReport = 0;
uint32_t loopCount = 0;

void updateBlink(uint32_t now)
{
    const uint32_t elapsed = now - lastBlink;
    if (elapsed >= BLINK_MS)
    {
        // Giu moc thoi gian va bu cac chu ky bo lo, ke ca khi millis tran.
        const uint32_t periods = elapsed / BLINK_MS;
        lastBlink += periods * BLINK_MS;
        if (periods % 2 != 0)
        {
            ledState1 = !ledState1;
            digitalWrite(LED_1, ledState1);
        }
    }
}

void reportLoopCount(uint32_t now)
{
    const uint32_t elapsed = now - lastReport;
    if (elapsed >= REPORT_MS)
    {
        const uint32_t count = loopCount;
        loopCount = 0;
        lastReport = now;
        Serial.printf("loop(): %lu lan / %lu ms\n",
                      static_cast<unsigned long>(count),
                      static_cast<unsigned long>(elapsed));
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
    lastBlink = lastReport = millis();
}

void loop()
{
    const uint32_t now = millis();
    ++loopCount;
    updateBlink(now);      // Time-triggered: 300 ms.
    updateButton(now);     // Event-driven: canh nhan sau chong doi.
    reportLoopCount(now);  // Time-triggered: 1000 ms.
}
