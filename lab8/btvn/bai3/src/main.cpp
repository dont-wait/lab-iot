#define BLYNK_TEMPLATE_ID "TMPL_REPLACE_ME"
#define BLYNK_TEMPLATE_NAME "Lab8 Debounce Deadband"
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
constexpr uint8_t BUTTON_PIN = 4; // Nút nối giữa GPIO4 và 3V3, dùng INPUT_PULLDOWN.
constexpr uint8_t LED_PIN = 2;

constexpr uint32_t SENSOR_INTERVAL_MS = 2000;
constexpr uint32_t DEBOUNCE_MS = 50;
constexpr uint32_t COMMAND_THROTTLE_MS = 1000;
constexpr uint32_t SENSOR_HEARTBEAT_MS = 30000;
constexpr float TEMPERATURE_DEADBAND_C = 0.5f;
constexpr float HUMIDITY_DEADBAND_PERCENT = 1.0f;

DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

bool lastRawButtonState = false;
bool stableButtonState = false;
uint32_t lastButtonChangeAt = 0;
uint32_t lastCommandAt = 0;
bool ledState = false;

bool hasPublishedSensor = false;
float lastPublishedTemperature = NAN;
float lastPublishedHumidity = NAN;
uint32_t lastSensorPublishAt = 0;

void publishConnectionState() {
    if (Blynk.connected()) {
        Blynk.virtualWrite(V2, "ONLINE");
    }
}

bool sensorOutsideDeadband(float temperature, float humidity, uint32_t now) {
    if (!hasPublishedSensor) {
        return true;
    }

    const bool temperatureChanged =
        fabsf(temperature - lastPublishedTemperature) > TEMPERATURE_DEADBAND_C;
    const bool humidityChanged =
        fabsf(humidity - lastPublishedHumidity) > HUMIDITY_DEADBAND_PERCENT;
    const bool heartbeatDue = now - lastSensorPublishAt >= SENSOR_HEARTBEAT_MS;

    return temperatureChanged || humidityChanged || heartbeatDue;
}

void publishSensorData() {
    const float temperature = dht.readTemperature();
    const float humidity = dht.readHumidity();
    const uint32_t now = millis();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Khong doc duoc DHT22; bo qua lan gui nay.");
        return;
    }

    if (!Blynk.connected()) {
        Serial.printf("Sensor %.1f C, %.1f %% | Blynk offline\n", temperature, humidity);
        return;
    }

    if (!sensorOutsideDeadband(temperature, humidity, now)) {
        Serial.printf("Bo qua: thay doi nho hon dead-band (%.1f C, %.1f %%)\n",
                      temperature, humidity);
        return;
    }

    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);
    lastPublishedTemperature = temperature;
    lastPublishedHumidity = humidity;
    lastSensorPublishAt = now;
    hasPublishedSensor = true;

    Serial.printf("Gui sensor: %.1f C | %.1f %%\n", temperature, humidity);
}

void checkButton() {
    const uint32_t now = millis();
    const bool rawState = digitalRead(BUTTON_PIN) == HIGH;

    if (rawState != lastRawButtonState) {
        lastRawButtonState = rawState;
        lastButtonChangeAt = now;
    }

    // Chỉ chấp nhận trạng thái nếu ổn định đủ DEBOUNCE_MS.
    if (rawState == stableButtonState || now - lastButtonChangeAt < DEBOUNCE_MS) {
        return;
    }

    stableButtonState = rawState;
    if (!stableButtonState) {
        return; // Chỉ xử lý một lần ở sườn nhấn, không xử lý khi nhả.
    }

    // Throttling: không gửi các lệnh quá gần nhau dù nút bị dội hoặc bị giữ.
    if (lastCommandAt != 0 && now - lastCommandAt < COMMAND_THROTTLE_MS) {
        Serial.println("Bo qua lenh nut: dang trong cua so throttle.");
        return;
    }

    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    lastCommandAt = now;

    if (Blynk.connected()) {
        Blynk.virtualWrite(V3, ledState ? 1 : 0);
        Serial.printf("Gui lenh den: %s\n", ledState ? "ON" : "OFF");
    } else {
        Serial.println("Blynk offline; doi lenh den, khong gui request.");
    }
}
} // namespace

BLYNK_CONNECTED() {
    publishConnectionState();
    Blynk.virtualWrite(V3, ledState ? 1 : 0);
    Serial.println("Blynk da ket noi.");
}

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(BUTTON_PIN, INPUT_PULLDOWN);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    dht.begin();

    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
    timer.setInterval(SENSOR_INTERVAL_MS, publishSensorData);
}

void loop() {
    Blynk.run();
    timer.run();
    checkButton();
}
