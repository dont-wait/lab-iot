#define BLYNK_TEMPLATE_ID "TMPL_REPLACE_ME"
#define BLYNK_TEMPLATE_NAME "Lab7 Device Control"
#define BLYNK_AUTH_TOKEN "REPLACE_WITH_DEVICE_TOKEN"
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <BlynkSimpleEsp32.h>

namespace {
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";
constexpr uint8_t LED_PIN = 5;
constexpr uint8_t PWM_CHANNEL = 0;
constexpr uint16_t PWM_FREQUENCY = 5000;
constexpr uint8_t PWM_RESOLUTION = 8;

bool deviceEnabled = false;
int brightnessPercent = 50;

void applyOutput() {
    const int duty = deviceEnabled ? map(brightnessPercent, 0, 100, 0, 255) : 0;
    ledcWrite(PWM_CHANNEL, duty);
    Serial.printf("Cong tac: %s | Do sang: %d %% | PWM: %d\n",
                  deviceEnabled ? "BAT" : "TAT", brightnessPercent, duty);
}

void sendFeedback() {
    Blynk.virtualWrite(V3, deviceEnabled ? 1 : 0);
    Blynk.virtualWrite(V4, brightnessPercent);
}
}  // namespace

BLYNK_CONNECTED() {
    // Lấy trạng thái gần nhất đã lưu trên Blynk khi ESP32 kết nối lại.
    Blynk.syncVirtual(V3, V4);
}

BLYNK_WRITE(V3) {
    deviceEnabled = param.asInt() == 1;
    applyOutput();
    sendFeedback();
}

BLYNK_WRITE(V4) {
    brightnessPercent = constrain(param.asInt(), 0, 100);
    applyOutput();
    sendFeedback();
}

void setup() {
    Serial.begin(115200);
    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(LED_PIN, PWM_CHANNEL);
    applyOutput();
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
    Blynk.run();
}

