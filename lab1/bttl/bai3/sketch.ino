#include <Arduino.h>

const int LED_PIN = 2;

const unsigned long LED_INTERVAL = 500;
const unsigned long SERIAL_INTERVAL = 3000;

unsigned long lastLedToggle = 0;
unsigned long lastSerialPrint = 0;

bool ledState = false;

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("=== BOOT OK ===");
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}

void loop()
{
    unsigned long now = millis();

    if (now - lastLedToggle >= LED_INTERVAL)
    {
        lastLedToggle = now;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
    }

    if (now - lastSerialPrint >= SERIAL_INTERVAL)
    {
        lastSerialPrint = now;
        Serial.print("Uptime: ");
        Serial.print(now);
        Serial.println(" ms");
    }

    delay(10);
}