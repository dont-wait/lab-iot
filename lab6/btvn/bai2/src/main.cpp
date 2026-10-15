#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace
{
constexpr uint8_t LIGHT_PIN = 18;
constexpr uint8_t FAN_PIN = 19; // LED represents the fan in Wokwi.
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";
constexpr char MQTT_HOST[] = "broker.hivemq.com";
constexpr uint16_t MQTT_PORT = 1883;
constexpr char COMMAND_TOPIC[] = "vnhome/esp32-dontwait/device/cmd";

WiFiClient network;
PubSubClient mqtt(network);
bool lightOn = false;
uint8_t fanSpeed = 0;
}

void onMessage(char *topic, uint8_t *payload, unsigned int length)
{
    JsonDocument doc;
    if (deserializeJson(doc, payload, length))
    {
        Serial.println("JSON khong hop le.");
        return;
    }

    const char *target = doc["target"] | "";
    if (strcmp(target, "light") == 0)
    {
        const char *action = doc["action"] | "";
        if (strcmp(action, "toggle") == 0) lightOn = !lightOn;
        else if (strcmp(action, "on") == 0) lightOn = true;
        else if (strcmp(action, "off") == 0) lightOn = false;
        else { Serial.println("Action den khong hop le."); return; }
        digitalWrite(LIGHT_PIN, lightOn ? HIGH : LOW);
        Serial.printf("Den phong khach: %s\n", lightOn ? "ON" : "OFF");
    }
    else if (strcmp(target, "fan") == 0)
    {
        const int speed = doc["speed"] | -1;
        if (speed < 0 || speed > 3) { Serial.println("Toc do quat phai tu 0 den 3."); return; }
        fanSpeed = static_cast<uint8_t>(speed);
        digitalWrite(FAN_PIN, fanSpeed > 0 ? HIGH : LOW);
        Serial.printf("Quat speed: %u/3 (LED mo phong %s)\n", fanSpeed, fanSpeed ? "ON" : "OFF");
    }
    else Serial.println("Target khong ho tro.");
    (void)topic;
}

void setup()
{
    Serial.begin(115200);
    pinMode(LIGHT_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
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
        const String clientId = "esp32-control-" + String(static_cast<uint32_t>(ESP.getEfuseMac()), HEX);
        if (mqtt.connect(clientId.c_str())) mqtt.subscribe(COMMAND_TOPIC);
        else { delay(2000); return; }
    }
    mqtt.loop();
}
