#include <Arduino.h>
#include <ESP32Ping.h>
#include <WiFi.h>

// Với Wokwi có thể giữ nguyên hai giá trị này. Khi chạy mạch thật,
// thay bằng SSID và mật khẩu của mạng Wi-Fi cần sử dụng.
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";
constexpr IPAddress PING_TARGET(8, 8, 8, 8);
constexpr uint32_t INTERNET_CHECK_INTERVAL_MS = 5UL * 60UL * 1000UL;
constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 10UL * 1000UL;
constexpr uint8_t MAX_FAILED_CHECKS = 3;

uint32_t lastInternetCheck = 0;
uint32_t lastWifiAttempt = 0;
uint8_t consecutiveFailures = 0;

void recordFailedCheck()
{
    ++consecutiveFailures;
    Serial.printf("Kiem tra Internet that bai (%u/%u).\n",
                  static_cast<unsigned int>(consecutiveFailures),
                  static_cast<unsigned int>(MAX_FAILED_CHECKS));

    if (consecutiveFailures >= MAX_FAILED_CHECKS)
    {
        Serial.println("Mat ket noi Internet 3 lan lien tiep, khoi dong lai ESP32.");
        ESP.restart();
    }
}

void maintainWiFi(uint32_t now)
{
    if (WiFi.status() == WL_CONNECTED || now - lastWifiAttempt < WIFI_RETRY_INTERVAL_MS)
    {
        return;
    }

    Serial.println("Dang thu ket noi lai Wi-Fi...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastWifiAttempt = now;
}

void checkInternet(uint32_t now)
{
    if (now - lastInternetCheck < INTERNET_CHECK_INTERVAL_MS)
    {
        return;
    }
    lastInternetCheck = now;

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Wi-Fi dang mat ket noi; bo qua ping Internet.");
        recordFailedCheck();
        return;
    }

    Serial.println("Dang ping 8.8.8.8...");
    if (Ping.ping(PING_TARGET, 1))
    {
        Serial.println("Internet hoat dong.");
        consecutiveFailures = 0;
        return;
    }

    recordFailedCheck();
}

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastWifiAttempt = millis();
    lastInternetCheck = millis();
    Serial.println("Watchdog Wi-Fi da khoi dong.");
    Serial.println("Se ping 8.8.8.8 moi 5 phut.");
}

void loop()
{
    const uint32_t now = millis();
    maintainWiFi(now);
    checkInternet(now);
    delay(100);
}
