#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";

const char *NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 7 * 3600;
const int DAYLIGHT_OFFSET_SEC = 0;

const unsigned long LCD_UPDATE_INTERVAL = 1000;
const unsigned long WIFI_CHECK_INTERVAL = 5000;
const unsigned long WIFI_RETRY_INTERVAL = 10000;

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long lastLcdUpdate = 0;
unsigned long lastWifiCheck = 0;
unsigned long lastWifiRetry = 0;

bool timeSynced = false;
bool wifiWasConnected = false;

void updateWifiStatus()
{
    lcd.setCursor(15, 1);
    if (WiFi.status() == WL_CONNECTED)
        lcd.print("W");
    else
        lcd.print("x");
}

void connectWiFi()
{
    Serial.print("[WIFI] Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password, 6);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
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

void updateLcd()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100))
    {
        Serial.println("[TIME] Failed to get time");
        return;
    }

    char line1[17];
    char line2[17];

    strftime(line1, sizeof(line1), "%d/%m/%Y", &timeinfo);
    strftime(line2, sizeof(line2), "%H:%M:%S", &timeinfo);

    lcd.setCursor(0, 0);
    lcd.print(line1);

    lcd.setCursor(0, 1);
    lcd.print(line2);

    updateWifiStatus();

    Serial.print("[TIME] ");
    Serial.print(line1);
    Serial.print(" ");
    Serial.println(line2);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Wire.begin(19, 18);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting...");

    connectWiFi();

    if (WiFi.status() == WL_CONNECTED)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Syncing NTP...");
        syncTime();
        lcd.clear();
        wifiWasConnected = true;
    }
    else
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi failed!");
    }
}

void loop()
{
    unsigned long now = millis();

    if (now - lastWifiCheck >= WIFI_CHECK_INTERVAL)
    {
        lastWifiCheck = now;

        if (WiFi.status() == WL_CONNECTED)
        {
            if (!wifiWasConnected)
            {
                Serial.println("[WIFI] Reconnected!");
                wifiWasConnected = true;
                if (!timeSynced)
                    syncTime();
            }
        }
        else
        {
            if (wifiWasConnected)
            {
                Serial.println("[WIFI] Disconnected!");
                wifiWasConnected = false;
            }

            if (now - lastWifiRetry >= WIFI_RETRY_INTERVAL)
            {
                lastWifiRetry = now;
                Serial.println("[WIFI] Retrying...");
                WiFi.disconnect();
                delay(100);
                WiFi.begin(ssid, password, 6);
            }
        }
    }

    if (now - lastLcdUpdate >= LCD_UPDATE_INTERVAL)
    {
        lastLcdUpdate = now;

        if (WiFi.status() == WL_CONNECTED && timeSynced)
            updateLcd();
        else
            updateWifiStatus();
    }
}