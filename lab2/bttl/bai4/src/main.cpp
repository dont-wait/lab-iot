#include <Arduino.h>
#include <DHTesp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

constexpr uint8_t BUTTON_PIN = 18;
constexpr uint8_t LED_1 = 4;
constexpr uint8_t LED_2 = 5;
constexpr uint32_t DEBOUNCE_MS = 30;

bool ledState2 = false;
int lastReading = HIGH;
int stableButton = HIGH;
uint32_t lastButtonChange = 0;

void updateButton(uint32_t now)
{
    const int reading = digitalRead(BUTTON_PIN);
    if (reading != lastReading)
    {
        lastReading = reading;
        lastButtonChange = now;
    }
    // Chi dao LED khi nhan xuong on dinh, khong lap lai khi giu nut.
    if (now - lastButtonChange >= DEBOUNCE_MS && reading != stableButton)
    {
        stableButton = reading;
        if (stableButton == LOW)
        {
            ledState2 = !ledState2;
            digitalWrite(LED_1, ledState2);
            digitalWrite(LED_2, ledState2);
        }
    }
}

constexpr uint8_t DHT_PIN = 23;
constexpr uint32_t SAMPLE_MS = 2000;

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
        // Moc tuyet doi: thoi gian doc khong cong don vao chu ky 2 giay.
        // Chi ngu tac vu nay; loop() tren core 1 van quet nut.
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(SAMPLE_MS));
        dht.resetTimer(); // Buoc doc moi, khong tra lai mau cache.
        const TempAndHumidity value = dht.getTempAndHumidity();
        if (dht.getStatus() != DHTesp::ERROR_NONE || isnan(value.temperature))
        {
            Serial.printf("[%lu ms] Loi DHT: %s\n",
                          millis(), dht.getStatusString());
        }
        else
        {
            Serial.printf("[%lu ms] Nhiet do: %.1f C | Do am: %.1f %%\n",
                          millis(), value.temperature, value.humidity);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_1, OUTPUT);
    digitalWrite(LED_1, LOW);
    pinMode(LED_2, OUTPUT);
    digitalWrite(LED_2, LOW);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    // ESP32 DevKit: doc DHT tren core 0, loop Arduino chay tren core 1.
    if (xTaskCreatePinnedToCore(readSensorTask, "DHT", 4096, nullptr,
                                1, nullptr, 0) != pdPASS)
    {
        Serial.println("Khong tao duoc tac vu DHT.");
    }
}

void loop()
{
    updateButton(millis());
}
