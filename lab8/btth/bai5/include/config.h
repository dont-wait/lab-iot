#pragma once

#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL_REPLACE_ME"
#define BLYNK_TEMPLATE_NAME "Lab 8 Feedback Loop"
#define BLYNK_AUTH_TOKEN "REPLACE_WITH_DEVICE_TOKEN"
#define BLYNK_PRINT Serial

namespace config {
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";

constexpr uint8_t LED_PIN = 2;
constexpr uint8_t VPIN_COMMAND = 0;
constexpr uint8_t VPIN_FEEDBACK = 1;
constexpr uint8_t VPIN_LATENCY_MS = 2;

// Send heartbeat/state at a moderate rate; command response is sent immediately.
constexpr uint32_t TELEMETRY_INTERVAL_MS = 5000;
constexpr uint32_t REPORT_INTERVAL_MS = 10000;
}  // namespace config
