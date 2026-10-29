# Bài 3 - Tối ưu băng thông: Debounce, Throttling và Dead-band

Project ESP32 đọc DHT22, điều khiển LED bằng nút nhấn và gửi dữ liệu lên Blynk.

## Mạch mô phỏng

- DHT22: GPIO15.
- Nút nhấn: GPIO4 và 3V3; firmware dùng `INPUT_PULLDOWN`.
- LED: GPIO2 qua điện trở 220 ohm.

## Cơ chế đã cài đặt

### 1. Debounce nút nhấn

Firmware đọc nút liên tục nhưng chỉ xác nhận trạng thái mới sau khi tín hiệu ổn định
`50 ms`. Một lần nhấn chỉ tạo một sự kiện ở sườn nhấn, tránh sinh nhiều request do
tiếp điểm cơ học bị dội.

### 2. Throttling lệnh

Sau mỗi lệnh điều khiển LED, firmware khóa việc gửi lệnh mới trong `1000 ms`.
Điều này bảo vệ Platform ngay cả khi nút bị giữ hoặc có nhiễu kéo dài.

### 3. Dead-band cảm biến

Firmware đọc DHT22 mỗi 2 giây nhưng chỉ gửi lên App khi:

- Nhiệt độ thay đổi vượt `0.5 °C`, hoặc
- Độ ẩm thay đổi vượt `1.0 %`, hoặc
- Đã quá `30 giây` chưa gửi bản tin heartbeat.

Lần đọc hợp lệ đầu tiên luôn được gửi. Vì vậy dữ liệu nhỏ hơn ngưỡng không chiếm
băng thông, nhưng App vẫn nhận được bản tin định kỳ để biết thiết bị còn hoạt động.

## Cấu hình Blynk

Trong `src/main.cpp`, thay:

```cpp
#define BLYNK_TEMPLATE_ID "TMPL_REPLACE_ME"
#define BLYNK_AUTH_TOKEN "REPLACE_WITH_DEVICE_TOKEN"
```

Tạo các datastream: `V0` nhiệt độ, `V1` độ ẩm, `V2` trạng thái kết nối và `V3`
trạng thái LED.

WiFi Wokwi dùng `Wokwi-GUEST`; khi chạy ESP32 thật, đổi SSID và password.

## Chạy và kiểm tra

```bash
cd lab8/btvn/bai3
pio run
```

Mở `diagram.json` bằng Wokwi for VS Code rồi chạy mô phỏng. Nhấn nhanh/liên tục
nút và xem Serial Monitor ở tốc độ 115200:

```text
Gui lenh den: ON
Bo qua lenh nut: dang trong cua so throttle.
Bo qua: thay doi nho hon dead-band (28.1 C, 60.2 %)
```

Các dòng `Gui lenh den` và `Gui sensor` tương ứng với request thực sự gửi lên
Blynk; các dòng `Bo qua` chứng minh debounce/throttling/dead-band đang hoạt động.
