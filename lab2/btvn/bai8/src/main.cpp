#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;
constexpr uint8_t PWM_LED = 4;
constexpr uint8_t POWER_LED = 2;
constexpr uint8_t PWM_CHANNEL = 0;
constexpr uint32_t PWM_HZ = 5000;
constexpr uint8_t PWM_BITS = 8;
constexpr uint8_t DUTY_LEVELS[] = {0, 64, 128, 191, 255};
constexpr uint32_t DEBOUNCE_MS = 30;
constexpr uint32_t POWER_HALF_PERIOD_MS = 750; // Chu ky day du 1500 ms.

uint8_t brightnessLevel = 0;
int lastReading = HIGH;
int stableButton = HIGH;
uint32_t lastButtonChange = 0;
uint32_t lastPowerBlink = 0;
bool powerLedOn = false;

void updateButton(uint32_t now)
{
    const int reading = digitalRead(BUTTON_PIN);
    if (reading != lastReading)
    {
        lastReading = reading;
        lastButtonChange = now;
    }
    if (now - lastButtonChange >= DEBOUNCE_MS && reading != stableButton)
    {
        stableButton = reading;
        if (stableButton == LOW)
        {
            brightnessLevel = (brightnessLevel + 1) % 5;
            ledcWrite(PWM_CHANNEL, DUTY_LEVELS[brightnessLevel]);
            Serial.printf("Do sang: %u %%\n",
                          static_cast<unsigned int>(brightnessLevel * 25));
        }
    }
}

void updatePowerLed(uint32_t now)
{
    const uint32_t elapsed = now - lastPowerBlink;
    if (elapsed >= POWER_HALF_PERIOD_MS)
    {
        const uint32_t periods = elapsed / POWER_HALF_PERIOD_MS;
        lastPowerBlink += periods * POWER_HALF_PERIOD_MS;
        if (periods % 2 != 0)
        {
            powerLedOn = !powerLedOn;
            digitalWrite(POWER_LED, powerLedOn);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(POWER_LED, OUTPUT);
    digitalWrite(POWER_LED, LOW);
    ledcSetup(PWM_CHANNEL, PWM_HZ, PWM_BITS);
    ledcAttachPin(PWM_LED, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, DUTY_LEVELS[brightnessLevel]);
    lastPowerBlink = millis();
}

void loop()
{
    const uint32_t now = millis();
    updateButton(now);
    updatePowerLed(now);
}
