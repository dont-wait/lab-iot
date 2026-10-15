#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";

const char *NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 7 * 3600;
const int DAYLIGHT_OFFSET_SEC = 0;

const unsigned long PRINT_INTERVAL = 1000;

unsigned long lastPrint = 0;
bool timeSynced = false;

void connectWiFi()
{
    Serial.print("[WIFI] Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
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
        Serial.println("[WIFI] Connected!");
        Serial.print("[WIFI] IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("[WIFI] Connection failed!");
    }
}

void syncTime()
{
    Serial.println("[NTP] Syncing time...");

    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

    struct tm timeinfo;
    unsigned long start = millis();
    while (!getLocalTime(&timeinfo, 1000))
    {
        Serial.print(".");
        if (millis() - start > 20000)
        {
            Serial.println();
            Serial.println("[NTP] Sync failed!");
            return;
        }
    }

    Serial.println();
    Serial.println("[NTP] Time synced!");
    timeSynced = true;
}

void printTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100))
    {
        Serial.println("[TIME] Failed to get time");
        return;
    }

    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.print("[TIME] ");
    Serial.println(buffer);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    connectWiFi();

    if (WiFi.status() == WL_CONNECTED)
    {
        syncTime();
    }
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        return;
    }

    if (!timeSynced)
    {
        syncTime();
        return;
    }

    unsigned long now = millis();
    if (now - lastPrint >= PRINT_INTERVAL)
    {
        lastPrint = now;
        printTime();
    }
}