#include <Arduino.h>
#include <BlynkSimpleEsp32.h>

#include "config.h"

namespace {
BlynkTimer timer;
bool ledState = false;
uint32_t commandStartedAt = 0;
uint32_t sampleCount = 0;
uint64_t latencyTotalMs = 0;
uint32_t latencyMinMs = UINT32_MAX;
uint32_t latencyMaxMs = 0;
uint32_t lastLatencyMs = 0;

void applyLedState(bool enabled) {
    ledState = enabled;
    digitalWrite(config::LED_PIN, ledState ? HIGH : LOW);
}

void sendFeedback() {
    if (!Blynk.connected()) {
        return;
    }
    Blynk.virtualWrite(config::VPIN_FEEDBACK, ledState ? 1 : 0);
}

void publishPeriodicTelemetry() {
    if (!Blynk.connected()) {
        return;
    }

    const uint32_t now = millis();
    Blynk.virtualWrite(config::VPIN_FEEDBACK, ledState ? 1 : 0);
    if (sampleCount > 0) {
        Blynk.virtualWrite(config::VPIN_LATENCY_MS, lastLatencyMs);
    }
    Serial.printf("HEARTBEAT t=%lu ms | LED=%s\n",
                  static_cast<unsigned long>(now), ledState ? "ON" : "OFF");
}

void printLatencySummary() {
    if (sampleCount == 0) {
        return;
    }
    const uint32_t average = static_cast<uint32_t>(latencyTotalMs / sampleCount);
    Serial.printf("LATENCY n=%lu | last=%lu ms | avg=%lu ms | min=%lu ms | max=%lu ms\n",
                  static_cast<unsigned long>(sampleCount),
                  static_cast<unsigned long>(lastLatencyMs),
                  static_cast<unsigned long>(average),
                  static_cast<unsigned long>(latencyMinMs),
                  static_cast<unsigned long>(latencyMaxMs));
}
}  // namespace

BLYNK_CONNECTED() {
    // Report the actual local output state when the platform reconnects.
    sendFeedback();
}

BLYNK_WRITE(V0) {
    commandStartedAt = millis();
    applyLedState(param.asInt() != 0);
    sendFeedback();

    lastLatencyMs = millis() - commandStartedAt;
    ++sampleCount;
    latencyTotalMs += lastLatencyMs;
    latencyMinMs = min(latencyMinMs, lastLatencyMs);
    latencyMaxMs = max(latencyMaxMs, lastLatencyMs);
    Blynk.virtualWrite(config::VPIN_LATENCY_MS, lastLatencyMs);

    Serial.printf("COMMAND id=%lu | state=%s | callback_to_feedback_queue=%lu ms\n",
                  static_cast<unsigned long>(sampleCount),
                  ledState ? "ON" : "OFF",
                  static_cast<unsigned long>(lastLatencyMs));
}

void setup() {
    Serial.begin(115200);
    pinMode(config::LED_PIN, OUTPUT);
    applyLedState(false);

    Blynk.begin(BLYNK_AUTH_TOKEN, config::WIFI_SSID, config::WIFI_PASSWORD);
    timer.setInterval(config::TELEMETRY_INTERVAL_MS, publishPeriodicTelemetry);
    timer.setInterval(config::REPORT_INTERVAL_MS, printLatencySummary);
}

void loop() {
    Blynk.run();
    timer.run();
}
