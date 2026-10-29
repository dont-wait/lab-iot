#include <Arduino.h>
#include "config.h"
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

namespace {
DHT dht(config::DHT_PIN, DHT22);
BlynkTimer timer;
bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
uint32_t buttonChangedAt = 0;
uint32_t lastConnectAttempt = 0;

bool readRelayState() {
    return digitalRead(config::RELAY_PIN) == config::RELAY_ACTIVE_LEVEL;
}

// Publish the GPIO level after applying an output change; never wait for a timer.
void sendSensorData() {
    const bool enabled = readRelayState();
    Serial.printf("STATE relay=%s | cloud=%s\n", enabled ? "ON" : "OFF",
                  Blynk.connected() ? "connected" : "offline");
    if (!Blynk.connected()) {
        return;
    }
    Blynk.virtualWrite(config::VPIN_RELAY, enabled ? 1 : 0);
    Blynk.virtualWrite(config::VPIN_WIFI_STATUS, WiFi.status() == WL_CONNECTED ? 1 : 0);
}

void setRelay(bool enabled) {
    digitalWrite(config::RELAY_PIN,
                 enabled ? config::RELAY_ACTIVE_LEVEL : config::RELAY_INACTIVE_LEVEL);
    sendSensorData();
}

void handleButton() {
    const uint32_t now = millis();
    const bool reading = digitalRead(config::BUTTON_PIN);
    if (reading != lastButtonReading) {
        lastButtonReading = reading;
        buttonChangedAt = now;
    }
    if (now - buttonChangedAt >= config::DEBOUNCE_MS && reading != stableButtonState) {
        stableButtonState = reading;
        if (stableButtonState == LOW) {
            Serial.println("BUTTON pressed");
            setRelay(!readRelayState());
        }
    }
}

void publishSensors() {
    const float temperature = dht.readTemperature();
    const float humidity = dht.readHumidity();
    if (!Blynk.connected()) {
        return;
    }
    if (!isnan(temperature)) {
        Blynk.virtualWrite(config::VPIN_TEMPERATURE, temperature);
    }
    if (!isnan(humidity)) {
        Blynk.virtualWrite(config::VPIN_HUMIDITY, humidity);
    }
}

void maintainConnection() {
    if (Blynk.connected()) {
        Blynk.run();
        return;
    }
    const uint32_t now = millis();
    if (now - lastConnectAttempt < config::RECONNECT_INTERVAL_MS) {
        return;
    }
    lastConnectAttempt = now;
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
        return;
    }
    // Bound each Cloud attempt so the physical button remains usable offline.
    Blynk.connect(250);
}
}  // namespace

BLYNK_CONNECTED() {
    // DEVICE -> CLOUD: overwrite a stale App value with the current GPIO state.
    // syncVirtual would instead retrieve the old Cloud value into BLYNK_WRITE.
    Serial.println("SYNC ON CONNECT: GPIO -> App");
    sendSensorData();
}

BLYNK_WRITE(V2) {
    Serial.println("APP command received");
    setRelay(param.asInt() != 0);
}

void setup() {
    Serial.begin(115200);
    digitalWrite(config::RELAY_PIN, config::RELAY_INACTIVE_LEVEL);
    pinMode(config::RELAY_PIN, OUTPUT);
    pinMode(config::BUTTON_PIN, INPUT_PULLUP);
    Serial.println("BOOT relay=OFF");
    dht.begin();

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
    Blynk.config(BLYNK_AUTH_TOKEN);
    timer.setInterval(config::SENSOR_INTERVAL_MS, publishSensors);
}

void loop() {
    handleButton();
    maintainConnection();
    timer.run();
}
