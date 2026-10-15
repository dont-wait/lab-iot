# Lab 6 - Bài tập về nhà 2: Điều khiển nhiều thiết bị bằng JSON

ESP32 subscribe topic `vnhome/esp32-dontwait/device/cmd` và phân luồng theo trường `target`. 

| Lệnh | Tác dụng |
|---|---|
| `{"target":"light","action":"toggle"}` | Đảo trạng thái đèn phòng khách |
| `{"target":"light","action":"on"}` | Bật đèn |
| `{"target":"light","action":"off"}` | Tắt đèn |
| `{"target":"fan","speed":2}` | Đặt quạt ở mức 2, chấp nhận 0-3 |

Trong Wokwi, LED vàng GPIO18 mô phỏng đèn và LED xanh GPIO19 mô phỏng quạt; các mức tốc độ quạt được in ra Serial, còn LED biểu diễn trạng thái chạy/dừng. Mạch thật có thể thay LED bằng relay/driver phù hợp; không cấp tải trực tiếp từ GPIO.

Kết nối Wokwi với `Wokwi-GUEST` và broker `broker.hivemq.com:1883`. Từ MQTT Explorer publish các payload trên vào command topic, sau đó quan sát LED và Serial Monitor ở 115200 baud. Broker công khai không có bảo mật, chỉ dùng để thực hành.
