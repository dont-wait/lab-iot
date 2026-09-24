#include <Arduino.h>

constexpr uint8_t RED_LED = 4;
constexpr uint8_t YELLOW_LED = 5;
constexpr uint8_t GREEN_LED = 19;
constexpr uint32_t RED_MS = 5000;
constexpr uint32_t GREEN_MS = 4000;
constexpr uint32_t YELLOW_MS = 2000;
constexpr uint32_t CYCLE_MS = RED_MS + GREEN_MS + YELLOW_MS;

enum class TrafficState { RED, GREEN, YELLOW };
TrafficState state = TrafficState::RED;
uint32_t stateStarted = 0;

void showState()
{
    // Tat tat ca truoc khi bat den moi.
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    switch (state)
    {
    case TrafficState::RED:
        digitalWrite(RED_LED, HIGH);
        break;
    case TrafficState::GREEN:
        digitalWrite(GREEN_LED, HIGH);
        break;
    case TrafficState::YELLOW:
        digitalWrite(YELLOW_LED, HIGH);
        break;
    }
}

void updateTraffic(uint32_t now)
{
    // Bo qua chu ky day du neu loop bi tre, toi da 3 lan chuyen ben duoi.
    const uint32_t elapsed = now - stateStarted;
    stateStarted += (elapsed / CYCLE_MS) * CYCLE_MS;
    for (uint8_t i = 0; i < 3; ++i)
    {
        uint32_t duration;
        TrafficState next;
        switch (state)
        {
        case TrafficState::RED:
            duration = RED_MS;
            next = TrafficState::GREEN;
            break;
        case TrafficState::GREEN:
            duration = GREEN_MS;
            next = TrafficState::YELLOW;
            break;
        case TrafficState::YELLOW:
            duration = YELLOW_MS;
            next = TrafficState::RED;
            break;
        default:
            return;
        }
        if (now - stateStarted < duration)
            break;
        stateStarted += duration; // Giu moc lich, khong cong don sai so.
        state = next;
    }
    // Chi ghi GPIO khi trang thai thuc su thay doi.
    static TrafficState displayed = TrafficState::RED;
    if (displayed != state)
    {
        showState();
        displayed = state;
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    stateStarted = millis();
    showState();
}

void loop()
{
    updateTraffic(millis());
}
