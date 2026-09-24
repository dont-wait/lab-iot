#include <Arduino.h>

#define LED_1 2
#define LED_2 4

unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;

const unsigned long interval1 = 500;
const unsigned long interval2 = 1200;

bool ledState1 = LOW;
bool ledState2 = LOW;

void setup()
{
    Serial.begin(115200);

    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);

    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
}

void loop()
{
    unsigned long currentMillis = millis();

    // LED 1: đổi trạng thái mỗi 500ms
    if (currentMillis - previousMillis1 >= interval1)
    {
        previousMillis1 = currentMillis;

        ledState1 = !ledState1;
        digitalWrite(LED_1, ledState1);
    }

    // LED 2: đổi trạng thái mỗi 1200ms
    if (currentMillis - previousMillis2 >= interval2)
    {
        previousMillis2 = currentMillis;

        ledState2 = !ledState2;
        digitalWrite(LED_2, ledState2);
    }
}