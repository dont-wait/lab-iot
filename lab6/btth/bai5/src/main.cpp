#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define SERIAL_BAUD 115200
#define DEVICE_ID   "ESP32_NODE_01"
#define LED_PIN     2

const char* ssid        = "Wokwi-GUEST";
const char* password    = "";
const char* mqtt_server = "broker.hivemq.com";
const int   mqtt_port   = 1883;

const char* topic_pub     = "home/living_room/sensor";
const char* topic_control = "home/living_room/control";
const char* topic_status  = "home/living_room/status";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublish = 0;
unsigned long lastReconnect = 0;
const long publishInterval = 5000;
const long reconnectInterval = 5000;

bool ledState = false;

float readTemperature() {
    return 25.0f + (random(0, 1000) / 100.0f);
}

float readHumidity() {
    return 40.0f + (random(0, 4000) / 100.0f);
}

uint32_t getTimestamp() {
    return millis() / 1000;
}

void publishStatus(const char* state) {
    JsonDocument doc;
    doc["device_id"] = DEVICE_ID;
    doc["led"]       = state;
    doc["timestamp"] = getTimestamp();

    char buffer[128];
    size_t n = serializeJson(doc, buffer);
    client.publish(topic_status, buffer, n);

    Serial.print("Status published: ");
    Serial.println(buffer);
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.println();
    Serial.println("=== MQTT Command Received ===");
    Serial.print("Topic: ");
    Serial.println(topic);

    String message;
    message.reserve(length + 1);
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.print("Payload: ");
    Serial.println(message);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, message);

    if (err) {
        Serial.print("JSON parse failed: ");
        Serial.println(err.c_str());
        return;
    }

    const char* cmd = doc["command"] | "";
    Serial.print("Command: ");
    Serial.println(cmd);

    if (strcmp(cmd, "ON") == 0) {
        ledState = true;
        digitalWrite(LED_PIN, HIGH);
        publishStatus("ON");
    } else if (strcmp(cmd, "OFF") == 0) {
        ledState = false;
        digitalWrite(LED_PIN, LOW);
        publishStatus("OFF");
    } else if (strcmp(cmd, "TOGGLE") == 0) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        publishStatus(ledState ? "ON" : "OFF");
    } else {
        Serial.println("Unknown command");
    }

    Serial.println("==============================");
}

void setup_wifi() {
    Serial.println();
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password, 6);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        randomSeed(esp_random());
        Serial.println();
        Serial.println("WiFi connected");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println();
        Serial.println("WiFi failed, will retry in loop");
    }
}

void reconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
        return;
    }

    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
        Serial.println("connected");
        client.subscribe(topic_control);
        Serial.print("Subscribed to: ");
        Serial.println(topic_control);
        publishStatus(ledState ? "ON" : "OFF");
    } else {
        Serial.print("failed, rc=");
        Serial.println(client.state());
    }
}

void publishSensorData() {
    JsonDocument doc;
    doc["device_id"]   = DEVICE_ID;
    doc["temperature"] = readTemperature();
    doc["humidity"]    = readHumidity();
    doc["led"]         = ledState ? "ON" : "OFF";
    doc["timestamp"]   = getTimestamp();

    char buffer[256];
    size_t n = serializeJson(doc, buffer);

    Serial.print("Publishing sensor: ");
    Serial.println(buffer);

    client.publish(topic_pub, buffer, n);
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(1000);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    client.setKeepAlive(15);
    client.setBufferSize(512);

    Serial.println("Setup complete");
}

void loop() {
    if (!client.connected()) {
        unsigned long now = millis();
        if (now - lastReconnect > reconnectInterval) {
            lastReconnect = now;
            reconnect();
        }
    } else {
        client.loop();
    }

    unsigned long now = millis();
    if (client.connected() && now - lastPublish > publishInterval) {
        lastPublish = now;
        publishSensorData();
    }
}