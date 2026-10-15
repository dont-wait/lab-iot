#include <Arduino.h>
#include <ArduinoJson.h>

#define SERIAL_BAUD 115200
#define DEVICE_ID   "ESP32_NODE_01"

float readTemperature() {
    return 25.0f + (random(0, 1000) / 100.0f);
}

float readHumidity() {
    return 40.0f + (random(0, 4000) / 100.0f);
}

uint32_t getTimestamp() {
    return millis() / 1000;
}

String buildSensorJson() {
    JsonDocument doc;

    doc["device_id"]   = DEVICE_ID;
    doc["temperature"] = readTemperature();
    doc["humidity"]    = readHumidity();
    doc["timestamp"]   = getTimestamp();

    String output;
    serializeJson(doc, output);
    return output;
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(1000);
    randomSeed(esp_random());

    Serial.println();
    Serial.println("=== ESP32 JSON Sensor Demo ===");
}

void loop() {
    String json = buildSensorJson();

    Serial.println("--- JSON Payload ---");
    Serial.println(json);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);

    if (err) {
        Serial.print("[ERROR] JSON khong hop le: ");
        Serial.println(err.c_str());
    } else {
        Serial.println("[OK] JSON hop le. Pretty print:");
        serializeJsonPretty(doc, Serial);
        Serial.println();
    }

    delay(5000);
}