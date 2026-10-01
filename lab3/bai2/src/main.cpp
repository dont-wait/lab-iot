#include <Arduino.h>
#include <WiFi.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";

const unsigned long RECONNECT_INTERVAL = 5000;
const unsigned long CHECK_INTERVAL = 3000;
const int MAX_RETRY = 5;

unsigned long lastCheck = 0;
unsigned long lastReconnect = 0;
int retryCount = 0;
bool wasConnected = false;

void WiFiEvent(WiFiEvent_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("[EVENT] Connected to AP (no IP yet)");
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("[EVENT] Got IP: ");
        Serial.println(WiFi.localIP());
        retryCount = 0;
        wasConnected = true;
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("[EVENT] WiFi disconnected!");
        wasConnected = false;
        break;

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("[EVENT] Lost IP address");
        break;

    default:
        break;
    }
}

void tryReconnect()
{
    if (retryCount >= MAX_RETRY)
    {
        Serial.println("[RECONNECT] Max retries reached. Pausing 30s...");
        delay(30000);
        retryCount = 0;
    }

    retryCount++;
    Serial.print("[RECONNECT] Attempt ");
    Serial.print(retryCount);
    Serial.print("/");
    Serial.print(MAX_RETRY);
    Serial.println(" - Reconnecting...");

    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid, password, 6);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_STA);
    WiFi.onEvent(WiFiEvent);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);

    Serial.print("Connecting to: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password, 6);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("Connected! IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("Initial connection failed!");
        tryReconnect();
    }
}

void loop()
{
    unsigned long now = millis();

    if (now - lastCheck < CHECK_INTERVAL)
        return;
    lastCheck = now;

    if (WiFi.status() == WL_CONNECTED)
    {
        if (!wasConnected)
        {
            Serial.println("[CHECK] WiFi reconnected!");
            wasConnected = true;
            retryCount = 0;
        }
        Serial.print("[CHECK] WiFi OK - IP: ");
        Serial.print(WiFi.localIP());
        Serial.print(" - RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        return;
    }

    if (wasConnected)
    {
        Serial.println("[CHECK] Disconnection detected!");
        wasConnected = false;
    }

    if (now - lastReconnect >= RECONNECT_INTERVAL)
    {
        lastReconnect = now;
        tryReconnect();
    }
}