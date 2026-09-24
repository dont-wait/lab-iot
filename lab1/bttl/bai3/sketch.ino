#include <Arduino.h>

const int NUT_BAM = 2;
const int LED = 8;

bool ledState = false;         // trạng thái hiện tại của đèn (sáng/tắt)
int trangThaiOnDinh = HIGH;    // trạng thái nút đã được xác nhận là ổn định (không nhiễu)
int docTruoc = HIGH;           // giá trị đọc được ở lần loop() trước

unsigned long thoiDiemDoiTruoc = 0;
const unsigned long THOI_GIAN_CHONG_NHIEU = 50; // ms

void setup()
{
    pinMode(NUT_BAM, INPUT_PULLUP);
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
}

void loop()
{
    int docNut = digitalRead(NUT_BAM);

    // Mỗi khi tín hiệu vừa đổi (do nhiễu hoặc do nhấn thật), reset lại mốc thời gian
    if (docNut != docTruoc)
    {
        thoiDiemDoiTruoc = millis();
    }

    // Tín hiệu đã đứng yên đủ lâu (> 50ms) mới được xem là hợp lệ, không phải nhiễu
    if ((millis() - thoiDiemDoiTruoc) > THOI_GIAN_CHONG_NHIEU)
    {
        // Nếu trạng thái ổn định thực sự thay đổi so với lần xác nhận trước
        if (docNut != trangThaiOnDinh)
        {
            trangThaiOnDinh = docNut;

            // Chỉ đảo đèn tại thời điểm phát hiện NHẤN xuống (LOW),
            // không đảo khi nhả ra -> mỗi lần bấm chỉ toggle đúng 1 lần
            if (trangThaiOnDinh == LOW)
            {
                ledState = !ledState;
                digitalWrite(LED, ledState);
            }
        }
    }

    docTruoc = docNut;
}
