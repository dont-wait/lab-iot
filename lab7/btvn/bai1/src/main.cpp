#define BLYNK_TEMPLATE_ID "TMPL_REPLACE_ME"
#define BLYNK_TEMPLATE_NAME "Lab7 Environment Dashboard"
#define BLYNK_AUTH_TOKEN "REPLACE_WITH_DEVICE_TOKEN"
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

namespace {
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";
constexpr uint8_t DHT_PIN = 15;
constexpr uint8_t DHT_TYPE = DHT22;
constexpr uint32_t SENSOR_INTERVAL_MS = 2000;

DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

void publishConnectionState() {
    Blynk.virtualWrite(V2, Blynk.connected() ? "ONLINE" : "OFFLINE");
}

void publishSensorData() {
    if (!Blynk.connected()) {
        Serial.println("Blynk dang offline; chua gui du lieu.");
        return;
    }

    const float temperature = dht.readTemperature();
    const float humidity = dht.readHumidity();

    publishConnectionState();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Khong doc duoc du lieu DHT22.");
        return;
    }

    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);

    Serial.printf(
        "Nhiet do: %.1f C | Do am: %.1f %% | Blynk: ONLINE\n",
        temperature,
        humidity
    );
}
}  // namespace

BLYNK_CONNECTED() {
    publishConnectionState();
    Serial.println("Blynk da ket noi.");
}

void setup() {
    Serial.begin(115200);
    delay(500);

    dht.begin();
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
    timer.setInterval(SENSOR_INTERVAL_MS, publishSensorData);
}

void loop() {
    Blynk.run();
    timer.run();
}
