#include <Arduino.h>
#include <BlynkSimpleEsp32.h>

#include "config.h"

namespace {
bool lightState = false;
bool lastRawButtonState = HIGH;
bool stableButtonState = HIGH;
uint32_t lastDebounceAt = 0;

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
}  // namespace

BLYNK_CONNECTED() {
    // The ESP32's current output state is authoritative on connect/reconnect.
    Blynk.virtualWrite(config::VPIN_LIGHT, lightState ? 1 : 0);
}

BLYNK_WRITE(V0) {
    applyLightState(param.asInt() != 0, false);
}

void setup() {
    Serial.begin(115200);
    pinMode(config::LIGHT_PIN, OUTPUT);
    pinMode(config::BUTTON_PIN, INPUT_PULLUP);
    applyLightState(false, false);

    Blynk.begin(BLYNK_AUTH_TOKEN, config::WIFI_SSID, config::WIFI_PASSWORD);
}

void loop() {
    Blynk.run();
    pollPhysicalButton();
}
