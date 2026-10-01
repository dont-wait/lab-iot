#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";
constexpr char NTP_SERVER[] = "pool.ntp.org";
constexpr char TIME_ZONE[] = "ICT-7"; // UTC+7, múi giờ Việt Nam.
constexpr uint32_t NTP_SYNC_INTERVAL_MS = 6UL * 60UL * 60UL * 1000UL;
constexpr uint32_t NTP_RETRY_INTERVAL_MS = 10UL * 1000UL;
constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 10UL * 1000UL;

uint32_t lastSync = 0;
uint32_t lastSyncAttempt = 0;
uint32_t lastWiFiAttempt = 0;
bool hasSynced = false;

void printTime(const char *label, time_t timestamp)
{
    struct tm localTime;
    localtime_r(&timestamp, &localTime);
    char text[24];
    strftime(text, sizeof(text), "%Y-%m-%d %H:%M:%S", &localTime);
    Serial.printf("%s: %s\n", label, text);
}

bool syncFromNtp()
{
    lastSyncAttempt = millis();
    const time_t before = time(nullptr);
    configTzTime(TIME_ZONE, NTP_SERVER);

    struct tm synchronizedTime;
    if (!getLocalTime(&synchronizedTime, 10000))
    {
        Serial.println("NTP: khong lay duoc thoi gian.");
        return false;
    }

    const time_t after = mktime(&synchronizedTime);
    if (before > 1000000000)
    {
        Serial.printf("NTP: chenh lech truoc/sau = %lld giay.\n",
                      static_cast<long long>(after - before));
        printTime("  Truoc dong bo", before);
    }
    else
    {
        Serial.println("NTP: lan dong bo dau tien, chua co moc thoi gian cu.");
    }
    printTime("  Sau dong bo", after);
    hasSynced = true;
    lastSync = millis();
    return true;
}

void maintainWiFi(uint32_t now)
{
    if (WiFi.status() == WL_CONNECTED || now - lastWiFiAttempt < WIFI_RETRY_INTERVAL_MS)
        return;

    Serial.println("Wi-Fi mat ket noi, dang thu ket noi lai...");
    WiFi.reconnect();
    lastWiFiAttempt = now;
}

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastWiFiAttempt = millis();
    Serial.println("Bat dau dong bo NTP.");
}

void loop()
{
    const uint32_t now = millis();
    maintainWiFi(now);

    const bool syncDue = !hasSynced
                      ? now - lastSyncAttempt >= NTP_RETRY_INTERVAL_MS
                      : now - lastSync >= NTP_SYNC_INTERVAL_MS;
    if (WiFi.status() == WL_CONNECTED && syncDue)
    {
        Serial.println("Dang cap nhat thoi gian tu NTP...");
        syncFromNtp();
    }

    delay(100);
}

