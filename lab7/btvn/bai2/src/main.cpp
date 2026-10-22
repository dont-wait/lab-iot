#include "secrets.h"

#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <WiFi.h>

namespace
{
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t DHT_TYPE = DHT22;
constexpr unsigned long SEND_INTERVAL_MS = 2000UL;

constexpr float TEMP_THRESHOLD = 35.0f;   // vuot nguong -> canh bao
constexpr float TEMP_RESET = 33.0f;       // xuong duoi muc nay moi cho canh bao lai

const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASSWORD = "";

const char *COLOR_NORMAL = "#23C48E"; // xanh
const char *COLOR_ALERT = "#D3435C";  // do

DHT dht(DHT_PIN, DHT_TYPE);
unsigned long lastSendAt = 0;
bool alertActive = false;
}

void setup()
{
    Serial.begin(115200);
    dht.begin();
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
}

void loop()
{
    Blynk.run();

    if (millis() - lastSendAt < SEND_INTERVAL_MS)
    {
        return;
    }
    lastSendAt = millis();

    const float temperature = dht.readTemperature();
    if (isnan(temperature))
    {
        Serial.println("Khong doc duoc DHT22");
        return;
    }

    Serial.printf("Nhiet do: %.1f C\n", temperature);

    // Gui nhiet do len Blynk (Virtual Pin V0)
    Blynk.virtualWrite(V0, temperature);

    // Canh bao: chi kich hoat 1 lan khi vuot nguong
    if (temperature > TEMP_THRESHOLD && !alertActive)
    {
        alertActive = true;
        Blynk.setProperty(V0, "color", COLOR_ALERT);
        Blynk.logEvent("high_temp", String("Nhiet do cao: ") + temperature + " C");
        Serial.println("=> CANH BAO: da gui event high_temp");
    }
    else if (temperature < TEMP_RESET && alertActive)
    {
        alertActive = false;
        Blynk.setProperty(V0, "color", COLOR_NORMAL);
        Serial.println("=> Nhiet do binh thuong tro lai");
    }
}