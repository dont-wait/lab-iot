#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define SERIAL_BAUD 115200
#define DEVICE_ID   "ESP32_NODE_01"

const char* ssid        = "Wokwi-GUEST";
const char* password    = "";
const char* mqtt_server = "broker.hivemq.com";
const int   mqtt_port   = 1883;

const char* topic_pub = "home/living_room/sensor";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 5000;

float readTemperature() {
    return 25.0f + (random(0, 1000) / 100.0f);
}

float readHumidity() {
    return 40.0f + (random(0, 4000) / 100.0f);
}

uint32_t getTimestamp() {
    return millis() / 1000;
}

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    randomSeed(esp_random());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}

void publishSensorData() {
    JsonDocument doc;
    doc["device_id"]   = DEVICE_ID;
    doc["temperature"] = readTemperature();
    doc["humidity"]    = readHumidity();
    doc["timestamp"]   = getTimestamp();

    char buffer[256];
    size_t n = serializeJson(doc, buffer);

    Serial.print("Publishing: ");
    Serial.println(buffer);

    client.publish(topic_pub, buffer, n);
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(1000);

    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);

    Serial.println("Setup complete");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > interval) {
        lastMsg = now;
        publishSensorData();
    }
}