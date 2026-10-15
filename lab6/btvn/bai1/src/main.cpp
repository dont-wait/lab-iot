#include <Arduino.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace
{
constexpr uint8_t DHT_PIN = 15;
constexpr uint8_t DHT_TYPE = DHT22; // Change to DHT11 when using a DHT11.
constexpr uint8_t LDR_PIN = 34;
constexpr uint32_t PUBLISH_INTERVAL_MS = 10000;
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";
constexpr char MQTT_HOST[] = "broker.hivemq.com";
constexpr uint16_t MQTT_PORT = 1883;
constexpr char MQTT_TOPIC[] = "vnhome/esp32-dontwait/sensor/data";

WiFiClient network;
PubSubClient mqtt(network);
DHT dht(DHT_PIN, DHT_TYPE);
uint32_t lastPublishAt = 0;
}

void connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED) return;
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectMqtt()
{
    if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
    const String clientId = "esp32-sensor-" + String(static_cast<uint32_t>(ESP.getEfuseMac()), HEX);
    if (mqtt.connect(clientId.c_str())) Serial.println("Da ket noi MQTT broker.");
}

void publishReading()
{
    const float humidity = dht.readHumidity();
    const float temperature = dht.readTemperature();
    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Khong doc duoc DHT; bo qua ban tin.");
        return;
    }

    JsonDocument doc;
    doc["device"] = "esp32-environment";
    doc["temperature_c"] = temperature;
    doc["humidity_percent"] = humidity;
    doc["light_analog"] = analogRead(LDR_PIN);
    doc["rssi_dbm"] = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
    char payload[192];
    const size_t length = serializeJson(doc, payload, sizeof(payload));
    if (mqtt.publish(MQTT_TOPIC, reinterpret_cast<const uint8_t *>(payload), length))
    {
        Serial.printf("Published %s: %s\n", MQTT_TOPIC, payload);
    }
    else Serial.println("Publish that bai.");
}

void setup()
{
    Serial.begin(115200);
    analogReadResolution(12);
    dht.begin();
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    connectWiFi();
}

void loop()
{
    connectWiFi();
    connectMqtt();
    mqtt.loop();
    const uint32_t now = millis();
    if (mqtt.connected() && now - lastPublishAt >= PUBLISH_INTERVAL_MS)
    {
        lastPublishAt = now;
        publishReading();
    }
    delay(10);
}
