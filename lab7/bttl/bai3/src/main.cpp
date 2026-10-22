#define BLYNK_TEMPLATE_ID "TMPL_REPLACE_ME"
#define BLYNK_TEMPLATE_NAME "Lab7 History Chart"
#define BLYNK_AUTH_TOKEN "REPLACE_WITH_DEVICE_TOKEN"
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

namespace {
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";
constexpr uint8_t DHT_PIN = 15;
constexpr uint8_t LDR_PIN = 34;
constexpr uint32_t SEND_INTERVAL_MS = 2000;

DHT dht(DHT_PIN, DHT22);
BlynkTimer timer;

float readLightPercent() {
    const int raw = analogRead(LDR_PIN);
    return constrain(raw * 100.0f / 4095.0f, 0.0f, 100.0f);
}

void publishSensorData() {
    const float humidity = dht.readHumidity();
    const float light = readLightPercent();

    if (isnan(humidity)) {
        Serial.println("Khong doc duoc cam bien DHT22");
        return;
    }

    Blynk.virtualWrite(V1, humidity);
    Blynk.virtualWrite(V2, light);

    Serial.printf("Do am: %.1f %% | Anh sang: %.1f %%\n", humidity, light);
}
}  // namespace

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    dht.begin();
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
    timer.setInterval(SEND_INTERVAL_MS, publishSensorData);
}

void loop() {
    Blynk.run();
    timer.run();
}

