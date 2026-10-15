# Lab 6 - Bài tập về nhà 1: Gửi dữ liệu môi trường qua MQTT

ESP32 đọc DHT22 và quang trở, sau đó publish một JSON mỗi 10 giây lên `vnhome/esp32-yourname/sensor/data`. Sửa `esp32-yourname` thành tên riêng để tránh trùng topic với người khác. DHT mặc định trong code là DHT22; nếu dùng DHT11, đổi `DHT_TYPE` trong `src/main.cpp`.

## Kết nối

| Linh kiện | ESP32 |
|---|---|
| DHT VCC / GND / DATA | 3V3 / GND / GPIO15 |
| LDR module VCC / GND / AO | 3V3 / GND / GPIO34 |

`diagram.json` dùng cảm biến DHT22 và module quang trở trong Wokwi. Với LDR rời, tạo cầu phân áp và nối điểm giữa vào GPIO34.

## MQTT

- Wi-Fi Wokwi: `Wokwi-GUEST`, mật khẩu rỗng.
- Broker thử nghiệm: `broker.hivemq.com:1883` (không mã hóa, không xác thực; chỉ dùng dữ liệu thực hành).
- JSON mẫu: `{"device":"esp32-environment","temperature_c":25,"humidity_percent":60,"light_analog":2048,"rssi_dbm":-45}`.
- Trong MQTT Explorer, kết nối cùng broker và subscribe topic ở trên để kiểm tra kiểu số và cấu trúc JSON.

Chạy `pio run`, mở mô phỏng Wokwi hoặc nạp lên ESP32 rồi xem Serial Monitor ở 115200 baud.
