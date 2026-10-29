#include <Arduino.h>
#include <BlynkSimpleEsp32.h>

#include "config.h"

namespace {
bool lightState = false;
bool lastRawButtonState = HIGH;
bool stableButtonState = HIGH;
bool wasBlynkConnected = false;
uint32_t lastDebounceAt = 0;
uint32_t lastWiFiAttemptAt = 0;
uint32_t lastBlynkAttemptAt = 0;

void applyLightState(bool enabled, bool publishToApp) {
    lightState = enabled;
    digitalWrite(config::LIGHT_PIN, lightState ? HIGH : LOW);

    if (publishToApp && Blynk.connected()) {
        Blynk.virtualWrite(config::VPIN_LIGHT, lightState ? 1 : 0);
    }
    Serial.printf("Den: %s\n", lightState ? "BAT" : "TAT");
}

void pollPhysicalButton() {
    const bool rawState = digitalRead(config::BUTTON_PIN);
    const uint32_t now = millis();

    if (rawState != lastRawButtonState) {
        lastRawButtonState = rawState;
        lastDebounceAt = now;
    }

    if (now - lastDebounceAt < config::DEBOUNCE_MS || rawState == stableButtonState) {
        return;
    }

    stableButtonState = rawState;
    if (stableButtonState == LOW) {
        applyLightState(!lightState, true);
    }
}

void maintainWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    const uint32_t now = millis();
    if (now - lastWiFiAttemptAt < config::WIFI_RETRY_INTERVAL_MS) {
        return;
    }

    lastWiFiAttemptAt = now;
    Serial.println("Wi-Fi mat ket noi; thu ket noi lai...");
    WiFi.disconnect();
    WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
}

void maintainBlynk() {
    if (WiFi.status() != WL_CONNECTED || Blynk.connected()) {
        return;
    }

    const uint32_t now = millis();
    if (now - lastBlynkAttemptAt < config::BLYNK_RETRY_INTERVAL_MS) {
        return;
    }

    lastBlynkAttemptAt = now;
    Serial.println("Thu ket noi Blynk...");
    Blynk.connect(1000);  // Bound each attempt so button polling continues.
}

void publishCurrentState() {
    if (!Blynk.connected()) {
        return;
    }

    Blynk.virtualWrite(config::VPIN_LIGHT, lightState ? 1 : 0);
    Blynk.virtualWrite(config::VPIN_WIFI_STATUS, WiFi.status() == WL_CONNECTED ? 1 : 0);
    Serial.println("Da dong bo trang thai cuc bo len Blynk.");
}

void monitorConnectionTransition() {
    const bool connected = Blynk.connected();
    if (connected && !wasBlynkConnected) {
        publishCurrentState();
        Serial.println("Blynk online; da dong bo lai trang thai.");
    }
    wasBlynkConnected = connected;
}
}  // namespace

BLYNK_CONNECTED() {
    // ESP32 local values are authoritative after an outage; do not sync stale cloud values down.
    Blynk.virtualWrite(config::VPIN_LIGHT, lightState ? 1 : 0);
    Blynk.virtualWrite(config::VPIN_WIFI_STATUS, WiFi.status() == WL_CONNECTED ? 1 : 0);
}

BLYNK_WRITE(V0) {
    applyLightState(param.asInt() != 0, false);
}

void setup() {
    Serial.begin(115200);
    pinMode(config::LIGHT_PIN, OUTPUT);
    pinMode(config::BUTTON_PIN, INPUT_PULLUP);
    applyLightState(false, false);

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
    lastWiFiAttemptAt = millis() - config::WIFI_RETRY_INTERVAL_MS;

    Blynk.config(BLYNK_AUTH_TOKEN);
    lastBlynkAttemptAt = millis() - config::BLYNK_RETRY_INTERVAL_MS;
    Blynk.connect(1000);
    wasBlynkConnected = Blynk.connected();
    if (wasBlynkConnected) {
        publishCurrentState();
    }
}

void loop() {
    pollPhysicalButton();
    maintainWiFi();
    maintainBlynk();
    Blynk.run();
    monitorConnectionTransition();
}
