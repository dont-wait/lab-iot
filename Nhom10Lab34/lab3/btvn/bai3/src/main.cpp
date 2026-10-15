#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <time.h>

constexpr char FALLBACK_SSID[] = "Wokwi-GUEST";
constexpr char FALLBACK_PASSWORD[] = "";
constexpr char NTP_SERVER[] = "pool.ntp.org";
constexpr char TIME_ZONE[] = "ICT-7";
constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 10UL * 1000UL;
constexpr uint32_t NTP_RETRY_INTERVAL_MS = 10UL * 1000UL;
constexpr uint32_t DISPLAY_INTERVAL_MS = 1000;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Preferences preferences;
uint32_t lastWiFiAttempt = 0;
uint32_t lastDisplay = 0;
uint32_t lastNtpAttempt = 0;
time_t baseEpoch = 0;
uint32_t baseMillis = 0;
bool hasValidTime = false;

void setDisplayLine(uint8_t row, const char *text)
{
    lcd.setCursor(0, row);
    lcd.print("                ");
    lcd.setCursor(0, row);
    lcd.print(text);
}

time_t displayedTime()
{
    if (!hasValidTime)
        return 0;
    return baseEpoch + static_cast<time_t>((millis() - baseMillis) / 1000UL);
}

bool syncNtp()
{
    lastNtpAttempt = millis();
    const time_t before = displayedTime();
    configTzTime(TIME_ZONE, NTP_SERVER);
    struct tm localTime;
    if (!getLocalTime(&localTime, 10000))
    {
        Serial.println("NTP that bai; tiep tuc dung dong ho noi bo.");
        return false;
    }

    const time_t after = mktime(&localTime);
    if (before > 1000000000)
    {
        Serial.printf("NTP: chenh lech truoc/sau = %lld giay.\n",
                      static_cast<long long>(after - before));
    }
    baseEpoch = after;
    baseMillis = millis();
    hasValidTime = true;
    preferences.putLong64("last_epoch", static_cast<int64_t>(baseEpoch));
    Serial.println("NTP dong bo thanh cong.");
    return true;
}

void updateDisplay()
{
    if (!hasValidTime)
    {
        setDisplayLine(0, "Dang cho NTP...");
        setDisplayLine(1, WiFi.status() == WL_CONNECTED ? "WiFi OK" : "! MAT WIFI");
        return;
    }

    const time_t now = displayedTime();
    struct tm localTime;
    localtime_r(&now, &localTime);
    char line[17];
    strftime(line, sizeof(line), "%H:%M:%S %d/%m", &localTime);
    setDisplayLine(0, line);
    setDisplayLine(1, WiFi.status() == WL_CONNECTED ? "WiFi OK" : "! MAT WIFI");
}

void connectWiFi()
{
    // WiFiManager ưu tiên thông tin đã lưu; fallback giúp mô phỏng Wokwi dễ dàng.
    WiFiManager manager;
    manager.setConfigPortalTimeout(30);
    if (!manager.autoConnect("ESP32-SmartClock"))
    {
        Serial.println("Khong dung duoc WiFiManager, thu Wokwi-GUEST...");
        WiFi.begin(FALLBACK_SSID, FALLBACK_PASSWORD);
    }
    lastWiFiAttempt = millis();
}

void setup()
{
    Serial.begin(115200);
    preferences.begin("smart-clock", false);
    baseEpoch = static_cast<time_t>(preferences.getLong64("last_epoch", 0));
    baseMillis = millis();
    hasValidTime = baseEpoch > 1000000000;
    if (hasValidTime)
        Serial.println("Dung moc gio da luu trong bo nho noi.");

    lcd.init();
    lcd.backlight();
    setDisplayLine(0, "Smart Clock");
    setDisplayLine(1, "Dang ket noi...");

    WiFi.mode(WIFI_STA);
    connectWiFi();
    if (WiFi.status() == WL_CONNECTED)
        syncNtp();
}

void loop()
{
    const uint32_t now = millis();
    if (WiFi.status() != WL_CONNECTED && now - lastWiFiAttempt >= WIFI_RETRY_INTERVAL_MS)
    {
        WiFi.reconnect();
        lastWiFiAttempt = now;
    }

    static bool wasConnected = WiFi.status() == WL_CONNECTED;
    const bool connected = WiFi.status() == WL_CONNECTED;
    const bool retryNtp = connected && !hasValidTime
                       && now - lastNtpAttempt >= NTP_RETRY_INTERVAL_MS;
    if (connected && (!wasConnected || retryNtp))
        syncNtp();
    wasConnected = connected;

    if (now - lastDisplay >= DISPLAY_INTERVAL_MS)
    {
        lastDisplay = now;
        updateDisplay();
    }
    delay(20);
}

