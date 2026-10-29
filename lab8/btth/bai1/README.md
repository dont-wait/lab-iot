# Bài 1 – Ánh xạ trạng thái Firmware lên Blynk

## Mục tiêu

Thống nhất tên, kiểu dữ liệu, đơn vị và chiều truyền dữ liệu giữa ESP32 và Blynk. Ví dụ này dùng DHT22, relay và Blynk Virtual Pin.

## Mapping table

| Trạng thái Firmware | Kiểu / đơn vị | Blynk datastream | Chiều | Ý nghĩa |
|---|---|---|---|---|
| `temperatureC` | `float`, °C | V0, Double, -40…80 | ESP32 → Blynk | Nhiệt độ DHT22 |
| `humidityPercent` | `float`, %RH | V1, Double, 0…100 | ESP32 → Blynk | Độ ẩm DHT22 |
| `relayState` | `bool`, 0/1 | V2, Integer, 0…1 | Hai chiều | Lệnh relay và trạng thái hiện tại |
| `wifiConnected` | `bool`, 0/1 | V3, Integer, 0…1 | ESP32 → Blynk | Trạng thái kết nối Wi-Fi |

Tạo Blynk Template và các Virtual Pin datastream với đúng kiểu/phạm vi trên. Đặt widget Value/Gauge cho V0, V1; Switch cho V2; LED/Value cho V3. Hằng số pin và thông tin Wi-Fi/Blynk đặt trong `include/config.h`.

## Cấu trúc

- `include/config.h`: chân phần cứng, Virtual Pin, chu kỳ lấy mẫu và cấu hình kết nối.
- `src/main.cpp`: đọc DHT22, xử lý trạng thái relay, gửi telemetry định kỳ và nhận lệnh Blynk.
- `platformio.ini`: cấu hình build ESP32 và khai báo dependencies cho người build sau này.

## Cấu hình trước khi sử dụng

Thay `BLYNK_TEMPLATE_ID`, `BLYNK_AUTH_TOKEN`, `WIFI_SSID`, `WIFI_PASSWORD` trong `include/config.h`; giữ đúng Template Name và datastream đã tạo trên Console. Relay được cấu hình active-high; đổi `RELAY_ACTIVE_LEVEL` nếu module của bạn active-low. Không đưa token thật lên Git.

Bài này chỉ chuẩn bị mã nguồn và mapping. Theo yêu cầu, không cài thư viện và không chạy thử/build.
