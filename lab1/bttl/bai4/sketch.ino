#include <Arduino.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22

const int LED_CANH_BAO = 8;
const float NGUONG_NHIET_DO = 30.0; // °C

DHT dht(DHTPIN, DHTTYPE);

unsigned long thoiDiemNhapNhay = 0;
bool trangThaiLed = false;
const unsigned long CHU_KY_NHAP_NHAY = 300; // ms

void setup()
{
    Serial.begin(9600);
    pinMode(LED_CANH_BAO, OUTPUT);
    digitalWrite(LED_CANH_BAO, LOW);

    dht.begin();
}

void loop()
{
    float nhietDo = dht.readTemperature();
    float doAm = dht.readHumidity();

    if (isnan(nhietDo) || isnan(doAm))
    {
        Serial.println("Loi doc cam bien DHT22!");
        delay(2000);
        return;
    }

    // In dữ liệu ra Serial Monitor
    Serial.print("Nhiet do: ");
    Serial.print(nhietDo);
    Serial.print(" *C | Do am: ");
    Serial.print(doAm);
    Serial.println(" %");

    if (nhietDo > NGUONG_NHIET_DO)
    {
        // Vượt ngưỡng -> nhấp nháy đèn cảnh báo (không dùng delay để vẫn đọc/in dữ liệu đều đặn)
        unsigned long hienTai = millis();
        if (hienTai - thoiDiemNhapNhay >= CHU_KY_NHAP_NHAY)
        {
            thoiDiemNhapNhay = hienTai;
            trangThaiLed = !trangThaiLed;
            digitalWrite(LED_CANH_BAO, trangThaiLed);
        }
    }
    else
    {
        // Dưới ngưỡng -> tắt đèn
        digitalWrite(LED_CANH_BAO, LOW);
        trangThaiLed = false;
    }

    delay(1000); // DHT22 chỉ nên đọc mỗi ~1-2 giây
}
