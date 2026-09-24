#include <Arduino.h>

constexpr uint8_t HEARTBEAT_LED = 2;
constexpr uint32_t BLINK_MS = 500; // Sang 500 ms + tat 500 ms = chu ky 1 giay.
constexpr uint32_t SERIAL_MS = 3000;
uint32_t lastBlink = 0;
uint32_t lastSerial = 0;
bool ledState = false;

void updateHeartbeat(uint32_t now)
{
    const uint32_t elapsed = now - lastBlink;
    if (elapsed >= BLINK_MS)
    {
        const uint32_t periods = elapsed / BLINK_MS;
        lastBlink += periods * BLINK_MS;
        if (periods % 2 != 0)
        {
            ledState = !ledState;
            digitalWrite(HEARTBEAT_LED, ledState);
        }
    }
}

void updateSerial(uint32_t now)
{
    const uint32_t elapsed = now - lastSerial;
    if (elapsed >= SERIAL_MS)
    {
        lastSerial += (elapsed / SERIAL_MS) * SERIAL_MS;
        Serial.printf("He thong dang hoat dong | Uptime: %lu ms\n",
                      static_cast<unsigned long>(now));
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(HEARTBEAT_LED, OUTPUT);
    digitalWrite(HEARTBEAT_LED, LOW);
    lastBlink = lastSerial = millis();
}

void loop()
{
    const uint32_t now = millis();
    updateHeartbeat(now);
    updateSerial(now);
}
