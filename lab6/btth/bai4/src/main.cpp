#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace
{
constexpr uint8_t RELAY_PIN = 2;

const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASSWORD = "";

const char *MQTT_SERVER = "broker.hivemq.com";
constexpr uint16_t MQTT_PORT = 1883;
const char *TOPIC_CONTROL = "hieu2001230244/living_room/control";
WiFiClient espClient;
PubSubClient client(espClient);
}

void connectToWiFi()
{
    Serial.printf("Dang ket noi Wi-Fi: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\nWi-Fi da ket noi, IP: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
    // 1. Doi mang byte thanh String
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
        message += static_cast<char>(payload[i]);
    }
    Serial.printf("Topic: %s\n", topic);
    Serial.printf("Noi dung: %s\n", message.c_str());

    // 2. Giai ma JSON
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, message);
    if (err)
    {
        Serial.printf("Loi parse JSON: %s\n", err.c_str());
        return;
    }

    const char *device = doc["device"];
    const char *state = doc["state"];
    if (device == nullptr || state == nullptr)
    {
        Serial.println("JSON thieu truong device hoac state");
        return;
    }

    // 3. Dieu khien theo lenh
    if (strcmp(device, "relay_1") != 0)
    {
        Serial.println("device khong hop le");
        return;
    }

    if (strcmp(state, "ON") == 0)
    {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("=> relay_1 BAT");
    }
    else if (strcmp(state, "OFF") == 0)
    {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("=> relay_1 TAT");
    }
    else
    {
        Serial.println("state khong hop le");
    }
}

void reconnectMqtt()
{
    while (!client.connected())
    {
        Serial.print("Dang ket noi MQTT...");
        String clientId = "esp32-bai4-" + String(random(0xffff), HEX);
        if (client.connect(clientId.c_str()))
        {
            Serial.println("OK");
            client.subscribe(TOPIC_CONTROL);
            Serial.printf("Da subscribe: %s\n", TOPIC_CONTROL);
        }
        else
        {
            Serial.printf("That bai, rc=%d, thu lai sau 3s\n", client.state());
            delay(3000);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    connectToWiFi();
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callback);
}

void loop()
{
    if (!client.connected())
    {
        reconnectMqtt();
    }
    client.loop();
}