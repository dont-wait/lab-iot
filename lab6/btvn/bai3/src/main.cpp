#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace
{
constexpr uint8_t LIGHT_PIN = 18;
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";
constexpr char MQTT_HOST[] = "broker.hivemq.com";
constexpr uint16_t MQTT_PORT = 1883;
constexpr char COMMAND_TOPIC[] = "home/device/cmd";
constexpr char STATUS_TOPIC[] = "home/device/status";

WiFiClient network;
PubSubClient mqtt(network);
bool lightOn = false;
}

void publishAck(bool success, const char *message)
{
    JsonDocument doc;
    doc["device"] = "light";
    doc["status"] = lightOn ? "ON" : "OFF";
    doc["timestamp_ms"] = millis(); // Uptime timestamp; no wall clock/NTP configured.
    doc["message"] = message;
    doc["success"] = success;
    char payload[192];
    const size_t length = serializeJson(doc, payload, sizeof(payload));
    mqtt.publish(STATUS_TOPIC, reinterpret_cast<const uint8_t *>(payload), length);
    Serial.printf("ACK %s: %s\n", STATUS_TOPIC, payload);
}

void onMessage(char *topic, uint8_t *payload, unsigned int length)
{
    JsonDocument doc;
    if (deserializeJson(doc, payload, length))
    {
        publishAck(false, "Invalid JSON");
        return;
    }
    const char *device = doc["device"] | "light";
    const char *action = doc["action"] | "";
    if (strcmp(device, "light") != 0 || (strcmp(action, "ON") != 0 && strcmp(action, "OFF") != 0))
    {
        publishAck(false, "Unsupported device or action");
        return;
    }

    lightOn = strcmp(action, "ON") == 0;
    digitalWrite(LIGHT_PIN, lightOn ? HIGH : LOW);
    publishAck(true, "Execution successful");
    (void)topic;
}

void setup()
{
    Serial.begin(115200);
    pinMode(LIGHT_PIN, OUTPUT);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(onMessage);
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED) { delay(100); return; }
    if (!mqtt.connected())
    {
        const String clientId = "esp32-ack-" + String(static_cast<uint32_t>(ESP.getEfuseMac()), HEX);
        if (mqtt.connect(clientId.c_str())) mqtt.subscribe(COMMAND_TOPIC);
        else { delay(2000); return; }
    }
    mqtt.loop();
}
