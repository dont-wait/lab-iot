#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

const char *AP_SSID = "SmartDevice_AP";
const char *AP_PASS = "12345678";

WiFiManager wm;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    wm.setConfigPortalTimeout(180);

    Serial.println("[WM] Trying to connect to saved WiFi...");
    Serial.print("[WM] If failed, AP will open: ");
    Serial.println(AP_SSID);

    bool connected = wm.autoConnect(AP_SSID, AP_PASS);

    if (connected)
    {
        Serial.println("[WM] WiFi connected!");
        Serial.print("[WM] IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("[WM] SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("[WM] RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    }
    else
    {
        Serial.println("[WM] Connection failed or timeout!");
    }
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 10000)
        {
            lastPrint = millis();
            Serial.print("[MAIN] WiFi OK - IP: ");
            Serial.println(WiFi.localIP());
        }
    }
    delay(100);
}