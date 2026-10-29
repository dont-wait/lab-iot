#pragma once

#include <Arduino.h>

// Blynk Console > Template > Device info. Replace placeholders before connecting.
#define BLYNK_TEMPLATE_ID "TMPL_REPLACE_ME"
#define BLYNK_TEMPLATE_NAME "Lab 8 State Mapping"
#define BLYNK_AUTH_TOKEN "REPLACE_WITH_DEVICE_TOKEN"
#define BLYNK_PRINT Serial

namespace config {
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";

constexpr uint8_t DHT_PIN = 15;
constexpr uint8_t RELAY_PIN = 2;
constexpr uint8_t RELAY_ACTIVE_LEVEL = HIGH;
constexpr uint8_t RELAY_INACTIVE_LEVEL = LOW;

constexpr uint8_t VPIN_TEMPERATURE = 0;
constexpr uint8_t VPIN_HUMIDITY = 1;
constexpr uint8_t VPIN_RELAY = 2;
constexpr uint8_t VPIN_WIFI_STATUS = 3;

constexpr uint32_t SENSOR_INTERVAL_MS = 2000;
}  // namespace config
