#include <Arduino.h>
#include <DHTesp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

constexpr uint8_t DHT_PIN = 23;
constexpr uint8_t BUZZER_PIN = 15;
constexpr uint8_t BUZZER_CHANNEL = 0;
constexpr uint32_t SAMPLE_MS = 2000;
constexpr uint32_t BEEP_MS = 200;
constexpr uint32_t TONE_HZ = 2000;
constexpr float THRESHOLD_C = 30.0f;

struct SensorResult
{
    float temperature;
    bool valid;
};
QueueHandle_t sensorQueue = nullptr;
bool alarmActive = false;
bool buzzerOn = false;
uint32_t lastBeep = 0;

void setBuzzer(bool on)
{
    buzzerOn = on;
    // Coi thu dong: phat song vuong 2 kHz khi bat.
    ledcWriteTone(BUZZER_CHANNEL, on ? TONE_HZ : 0);
}

void readSensorTask(void *)
{
    DHTesp dht;
#ifdef WOKWI_DHT22
    dht.setup(DHT_PIN, DHTesp::DHT22);
#else
    dht.setup(DHT_PIN, DHTesp::DHT11);
#endif
    TickType_t lastWake = xTaskGetTickCount();
    for (;;)
    {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(SAMPLE_MS));
        dht.resetTimer();
        const TempAndHumidity value = dht.getTempAndHumidity();
        const SensorResult result = {
            value.temperature,
            dht.getStatus() == DHTesp::ERROR_NONE && !isnan(value.temperature)
        };
        // Queue sao chep du lieu an toan giua hai core; luon giu mau moi nhat.
        xQueueOverwrite(sensorQueue, &result);
        if (result.valid)
            Serial.printf("[%lu ms] Nhiet do: %.1f C\n", millis(), result.temperature);
        else
            Serial.printf("[%lu ms] Loi DHT: %s (giu trang thai canh bao)\n",
                          millis(), dht.getStatusString());
    }
}

void updateAlarm(uint32_t now)
{
    SensorResult result;
    if (sensorQueue != nullptr &&
        xQueueReceive(sensorQueue, &result, 0) == pdTRUE && result.valid)
    {
        const bool nextAlarm = result.temperature > THRESHOLD_C;
        if (nextAlarm != alarmActive)
        {
            alarmActive = nextAlarm;
            lastBeep = now;
            setBuzzer(alarmActive); // Bat ngay khi vuot nguong, tat ngay khi <= 30.
        }
    }
}

void updateBuzzer(uint32_t now)
{
    if (!alarmActive)
        return;
    const uint32_t elapsed = now - lastBeep;
    if (elapsed >= BEEP_MS)
    {
        const uint32_t periods = elapsed / BEEP_MS;
        lastBeep += periods * BEEP_MS;
        if (periods % 2 != 0)
            setBuzzer(!buzzerOn);
    }
}

void setup()
{
    Serial.begin(115200);
    ledcSetup(BUZZER_CHANNEL, TONE_HZ, 8);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    setBuzzer(false);
    sensorQueue = xQueueCreate(1, sizeof(SensorResult));
    if (sensorQueue == nullptr)
    {
        Serial.println("Khong tao duoc queue DHT.");
        return;
    }
    // DHT tren core 0; loop va nhip coi tren core 1.
    if (xTaskCreatePinnedToCore(readSensorTask, "DHT", 4096, nullptr,
                                1, nullptr, 0) != pdPASS)
        Serial.println("Khong tao duoc tac vu DHT.");
}

void loop()
{
    const uint32_t now = millis();
    updateAlarm(now);
    updateBuzzer(now);
}
