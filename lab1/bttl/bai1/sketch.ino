#include <Arduino.h>

const int LED_DO = 8;
const int LED_VANG = 9;
const int LED_XANH = 10;

void setup()
{
    pinMode(LED_DO, OUTPUT);
    pinMode(LED_VANG, OUTPUT);
    pinMode(LED_XANH, OUTPUT);

    digitalWrite(LED_DO, LOW);
    digitalWrite(LED_VANG, LOW);
    digitalWrite(LED_XANH, LOW);
}

void loop()
{
    digitalWrite(LED_XANH, HIGH);
    delay(3000);
    digitalWrite(LED_XANH, LOW);

    digitalWrite(LED_VANG, HIGH);
    delay(1000);
    digitalWrite(LED_VANG, LOW);

    digitalWrite(LED_DO, HIGH);
    delay(3000);
    digitalWrite(LED_DO, LOW);
}