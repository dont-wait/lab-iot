#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Địa chỉ I2C thường gặp của LCD là 0x27 (một số module là 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long soGiayTruoc = 999999; // giá trị khởi tạo để chắc chắn lần đầu sẽ in

void setup()
{
    lcd.init();
    lcd.backlight();

    lcd.setCursor(0, 0);
    lcd.print("Thoi gian chay:");
}

void loop()
{
    unsigned long soGiay = millis() / 1000;

    // Chỉ cập nhật LCD khi số giây thay đổi, tránh nháy màn hình liên tục
    if (soGiay != soGiayTruoc)
    {
        soGiayTruoc = soGiay;

        // Xóa sạch dòng 2 trước khi in số mới, để tránh dính ký tự thừa
        // khi số giây giảm số chữ số (VD: từ 10 giây về 9 giây sau khi reset)
        lcd.setCursor(0, 1);
        lcd.print("                "); // 16 khoảng trắng = đúng độ rộng màn hình

        lcd.setCursor(0, 1);
        lcd.print(soGiay);
        lcd.print(" giay");
    }
}
