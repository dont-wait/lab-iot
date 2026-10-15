#include <Arduino.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

namespace
{
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t DHT_TYPE = DHT22;
constexpr unsigned long SENSOR_INTERVAL_MS = 5000UL;
constexpr unsigned long WIFI_TIMEOUT_MS = 20000UL;

const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASSWORD = "";

const char *WEBHOOK_URL = "https://webhook.site/34dc0077-5ceb-4eed-8c24-96b47cf6f448";

DHT dht(DHT_PIN, DHT_TYPE);
unsigned long lastRequestAt = 0;
}

void connectToWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    Serial.printf("Dang ket noi Wi-Fi: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const unsigned long startedAt = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startedAt < WIFI_TIMEOUT_MS)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("Wi-Fi da ket noi, dia chi IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("Khong ket noi duoc Wi-Fi.");
    }
}

void sendSensorData(float temperature, float humidity)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Bo qua HTTP POST vi Wi-Fi chua ket noi.");
        return;
    }

    String payload = "{\"device\":\"ESP32\",\"sensor\":\"DHT22\",\"temperature_c\":";
    payload += String(temperature, 2);
    payload += ",\"humidity_percent\":";
    payload += String(humidity, 2);
    payload += ",\"uptime_ms\":";
    payload += String(millis());
    payload += "}";

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(10000);
    if (!http.begin(client, WEBHOOK_URL))
    {
        Serial.println("Khong khoi tao duoc HTTP client.");
        return;
    }

    http.addHeader("Content-Type", "application/json");
    Serial.println("HTTP POST payload:");
    Serial.println(payload);

    const int statusCode = http.POST(payload);
    if (statusCode > 0)
    {
        Serial.printf("HTTP Response code: %d\n", statusCode);
        Serial.println("Phan hoi server:");
        Serial.println(http.getString());
    }
    else
    {
        Serial.printf("HTTP POST that bai: %s\n", http.errorToString(statusCode).c_str());
    }

    http.end();
}

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println();
    Serial.println("=== ESP32 DHT22 HTTP POST ===");

    dht.begin();
    connectToWiFi();
}

void loop()
{
    connectToWiFi();

    const unsigned long now = millis();
    if (now - lastRequestAt < SENSOR_INTERVAL_MS)
    {
        delay(50);
        return;
    }
    lastRequestAt = now;

    const float humidity = dht.readHumidity();
    const float temperature = dht.readTemperature();
    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Loi doc du lieu tu DHT22.");
        return;
    }

    Serial.printf("Nhiet do: %.2f C | Do am: %.2f %%\n", temperature, humidity);
    sendSensorData(temperature, humidity);
}
