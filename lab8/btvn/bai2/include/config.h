#pragma once

#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL_REPLACE_ME"
#define BLYNK_TEMPLATE_NAME "Lab 8 Auto Recovery"
#define BLYNK_AUTH_TOKEN "REPLACE_WITH_DEVICE_TOKEN"
#define BLYNK_PRINT Serial

namespace config {
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";

constexpr uint8_t LIGHT_PIN = 2;
constexpr uint8_t BUTTON_PIN = 4;
constexpr uint8_t VPIN_LIGHT = 0;
constexpr uint8_t VPIN_WIFI_STATUS = 1;

constexpr uint32_t DEBOUNCE_MS = 35;
constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 5000;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 10000;
constexpr uint32_t BLYNK_RETRY_INTERVAL_MS = 5000;
}  // namespace config
