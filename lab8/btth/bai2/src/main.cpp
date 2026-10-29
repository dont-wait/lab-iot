#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

#include "config.h"

namespace {
DHT dht(config::DHT_PIN, DHT22);
BlynkTimer timer;

float temperatureC = NAN;
float humidityPercent = NAN;
bool relayState = false;

void setRelay(bool enabled) {
    relayState = enabled;
    digitalWrite(config::RELAY_PIN,
                 relayState ? config::RELAY_ACTIVE_LEVEL : config::RELAY_INACTIVE_LEVEL);
    if (Blynk.connected()) {
        Blynk.virtualWrite(config::VPIN_RELAY, relayState ? 1 : 0);
    }
}

void readAndPublishSensors() {
    const float nextTemperature = dht.readTemperature();
    const float nextHumidity = dht.readHumidity();

    if (!isnan(nextTemperature)) {
        temperatureC = nextTemperature;
        Blynk.virtualWrite(config::VPIN_TEMPERATURE, temperatureC);
    } else {
        Serial.println("Khong doc duoc nhiet do DHT22");
    }

    if (!isnan(nextHumidity)) {
        humidityPercent = nextHumidity;
        Blynk.virtualWrite(config::VPIN_HUMIDITY, humidityPercent);
    } else {
        Serial.println("Khong doc duoc do am DHT22");
    }

    Blynk.virtualWrite(config::VPIN_RELAY, relayState ? 1 : 0);
    Blynk.virtualWrite(config::VPIN_WIFI_STATUS, WiFi.status() == WL_CONNECTED ? 1 : 0);
    Serial.printf("Nhiet do: %.1f C | Do am: %.1f %% | Relay: %s\n",
                  temperatureC, humidityPercent, relayState ? "BAT" : "TAT");
}
}  // namespace

BLYNK_CONNECTED() {
    // Restore dashboard switch state from the current device state after reconnect.
    Blynk.virtualWrite(config::VPIN_RELAY, relayState ? 1 : 0);
    Blynk.virtualWrite(config::VPIN_WIFI_STATUS, WiFi.status() == WL_CONNECTED ? 1 : 0);
}

BLYNK_WRITE(V2) {
    setRelay(param.asInt() != 0);
}

void setup() {
    Serial.begin(115200);
    pinMode(config::RELAY_PIN, OUTPUT);
    setRelay(false);
    dht.begin();

    Blynk.begin(BLYNK_AUTH_TOKEN, config::WIFI_SSID, config::WIFI_PASSWORD);
    timer.setInterval(config::SENSOR_INTERVAL_MS, readAndPublishSensors);
}

void loop() {
    Blynk.run();
    timer.run();
}
