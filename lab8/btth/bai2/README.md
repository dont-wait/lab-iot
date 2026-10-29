# Bài 2 – Tích hợp Firmware với Blynk IoT

## Mục tiêu

Ghép module DHT22/relay với kết nối Wi-Fi và Blynk trong một firmware ESP32. Blynk xử lý liên tục trong `loop()`; cảm biến được đọc bằng `BlynkTimer` mỗi 2 giây để tránh `delay()` chặn vòng lặp.

## Chức năng và datastream

| Virtual Pin | Datastream | Hướng | Chức năng |
|---|---|---|---|
| V0 | Double, °C | ESP32 → Blynk | Nhiệt độ DHT22 |
| V1 | Double, %RH | ESP32 → Blynk | Độ ẩm DHT22 |
| V2 | Integer, 0/1 | Hai chiều | Switch điều khiển relay; firmware gửi lại trạng thái |
| V3 | Integer, 0/1 | ESP32 → Blynk | Wi-Fi đang kết nối hay mất kết nối |

Tạo datastream và dashboard widgets tương ứng trong cùng Blynk Template. Các Virtual Pin này được khai báo tập trung trong `include/config.h`.

## Cấu hình

Điền Template ID, Auth Token, Wi-Fi SSID/mật khẩu trong `include/config.h`. Chân DHT22 mặc định GPIO 15, relay GPIO 2. Đầu vào relay mặc định active-high; sửa mức cấu hình nếu module relay của bạn active-low. Token chỉ dùng cục bộ, không commit lên kho mã nguồn.

## Cấu trúc mã

- `include/config.h`: credentials mẫu, chân nối, Virtual Pin và chu kỳ gửi.
- `src/main.cpp`: đối tượng Blynk/DHT/timer, callback V2, gửi telemetry và vòng lặp duy trì kết nối.
- `platformio.ini`: môi trường ESP32 và danh sách dependencies cần có khi người dùng chủ động build.

## Luồng hoạt động

1. `setup()` khởi tạo Serial, relay, DHT22 và kết nối Blynk/Wi-Fi.
2. Timer gọi tác vụ đọc cảm biến định kỳ; chỉ gửi giá trị hợp lệ.
3. Callback `BLYNK_WRITE(V2)` áp dụng lệnh bật/tắt relay từ dashboard.
4. `loop()` gọi `Blynk.run()` và `timer.run()` liên tục để duy trì truyền thông và chạy tác vụ không chặn.

Theo yêu cầu, không cài thư viện và không chạy thử/build. Cấu hình credentials thật và tạo đúng Template/Datastream trước khi sử dụng thiết bị.
