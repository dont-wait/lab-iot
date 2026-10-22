#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace {
constexpr uint8_t LED_PIN = 2;
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";
constexpr char MQTT_HOST[] = "broker.hivemq.com";
constexpr uint16_t MQTT_PORT = 1883;
constexpr char DEVICE_ID[] = "ESP32_NODE_01";
constexpr char SENSOR_TOPIC[] = "home/living_room/sensor";
constexpr char CONTROL_TOPIC[] = "home/living_room/control";
constexpr char STATUS_TOPIC[] = "home/living_room/status";
WiFiClient network;
PubSubClient mqtt(network);
bool ledOn = false;
uint32_t lastPublish = 0, lastReconnect = 0;
}

void publishStatus() {
    JsonDocument doc;
    doc["device_id"] = DEVICE_ID;
    doc["led"] = ledOn ? "ON" : "OFF";
    doc["timestamp"] = millis() / 1000;
    char payload[160];
    size_t n = serializeJson(doc, payload, sizeof(payload));
    mqtt.publish(STATUS_TOPIC, reinterpret_cast<const uint8_t *>(payload), n);
}

void command(const char *value) {
    if (strcmp(value, "ON") == 0) ledOn = true;
    else if (strcmp(value, "OFF") == 0) ledOn = false;
    else if (strcmp(value, "TOGGLE") == 0) ledOn = !ledOn;
    else return;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    publishStatus();
}

void onMessage(char *, uint8_t *payload, unsigned int length) {
    JsonDocument doc;
    if (!deserializeJson(doc, payload, length)) command(doc["command"] | "");
}

void publishSensors() {
    JsonDocument doc;
    doc["device_id"] = DEVICE_ID;
    doc["temperature"] = 25.0f + random(0, 1000) / 100.0f;
    doc["humidity"] = 40.0f + random(0, 4000) / 100.0f;
    doc["led"] = ledOn ? "ON" : "OFF";
    doc["timestamp"] = millis() / 1000;
    char payload[240];
    size_t n = serializeJson(doc, payload, sizeof(payload));
    mqtt.publish(SENSOR_TOPIC, reinterpret_cast<const uint8_t *>(payload), n);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    randomSeed(esp_random());
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(onMessage);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) { delay(100); return; }
    uint32_t now = millis();
    if (!mqtt.connected()) {
        if (now - lastReconnect >= 5000) {
            lastReconnect = now;
            String id = String("dashboard-esp32-") + String(random(0xffff), HEX);
            if (mqtt.connect(id.c_str())) { mqtt.subscribe(CONTROL_TOPIC); publishStatus(); }
        }
        return;
    }
    mqtt.loop();
    if (now - lastPublish >= 5000) { lastPublish = now; publishSensors(); }
}
