#include <Arduino.h>

const int NUT_BAM = 2;
const int LED = 8;

void setup()
{
    pinMode(NUT_BAM, INPUT_PULLUP); // Dùng điện trở kéo lên nội bộ
    pinMode(LED, OUTPUT);

    digitalWrite(LED, LOW);
}

void loop()
{
    // Vì dùng INPUT_PULLUP nên khi KHÔNG nhấn nút, chân đọc là HIGH.
    // Khi nhấn nút (nối GND), chân đọc là LOW.
    int trangThaiNut = digitalRead(NUT_BAM);

    if (trangThaiNut == LOW)
    {
        // Đang nhấn giữ nút -> bật đèn
        digitalWrite(LED, HIGH);
    }
    else
    {
        // Nhả nút ra -> tắt đèn
        digitalWrite(LED, LOW);
    }
}
