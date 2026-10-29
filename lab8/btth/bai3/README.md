# Bài 3 – Publish trạng thái ngay khi thay đổi

`setRelay()` ghi GPIO rồi gọi ngay `sendSensorData()` để đọc lại trạng thái chân và gửi lên V2. Cả lệnh từ App (`BLYNK_WRITE(V2)`) và nút vật lý đều đi qua hàm này, nên phản hồi relay không phụ thuộc timer cảm biến 2 giây.

Nút được chống dội 50 ms bằng `millis()`; chỉ cạnh nhấn LOW đảo trạng thái, giữ nút không đảo liên tục. Khi mất mạng, nút vẫn điều khiển relay; firmware thử kết nối lại theo chu kỳ 5 giây, mỗi lần gọi `Blynk.connect()` giới hạn 250 ms. Sau khi kết nối lại, `BLYNK_CONNECTED()` gửi trạng thái GPIO hiện tại. Không lưu hàng đợi các lần nhấn cũ.

## Kiểm chứng trên App

1. Cấu hình datastream, nạp firmware và chờ thiết bị Online.
2. Nhấn nút GPIO 18: relay đổi, Serial in `BUTTON pressed` và `STATE relay=ON/OFF`; Switch V2 phải đổi ngay theo, không đợi chu kỳ cảm biến. Độ trễ quan sát còn phụ thuộc mạng và App.
3. Giữ nút vài giây: chỉ đổi một lần. Nhả rồi nhấn lại: relay và V2 cùng đảo trạng thái.
4. Bật/tắt V2 trên App: quan sát GPIO/LED/relay và dòng `APP command received`, rồi phản hồi `STATE` tương ứng.
5. Ngắt Wi-Fi, nhấn nút vài lần rồi nối lại: phần cứng vẫn đổi khi offline, V2 nhận trạng thái cuối cùng sau khi kết nối lại.

## Cấu hình và đấu nối

Điền `BLYNK_TEMPLATE_ID`, `BLYNK_AUTH_TOKEN`, `WIFI_SSID`, `WIFI_PASSWORD` trong `include/config.h`. Token mẫu chưa dùng được; không commit token thật.

| Thành phần | Chân ESP32 | Kết nối |
|---|---|---|
| DHT22 | GPIO 15 | DATA → GPIO 15, VCC → 3V3, GND → GND |
| Relay | GPIO 2 | IN → GPIO 2, cấp nguồn phù hợp module và nối chung GND |
| Nút nhấn | GPIO 18 | Nối nút giữa GPIO 18 và GND; dùng `INPUT_PULLUP` |

Relay mặc định active-high. Với relay active-low, đổi `RELAY_ACTIVE_LEVEL` thành `LOW` và `RELAY_INACTIVE_LEVEL` thành `HIGH`. Có thể dùng LED kèm điện trở 220–330 Ω để quan sát GPIO 2 thay cho relay.

Tạo các datastream trong cùng Blynk Template và gán widget trên App/Web Dashboard:

| Virtual Pin | Kiểu / phạm vi | Widget | Ý nghĩa |
|---|---|---|---|
| V0 | Double, -40…80 °C | Value/Gauge | Nhiệt độ |
| V1 | Double, 0…100 %RH | Value/Gauge | Độ ẩm |
| V2 | Integer, 0…1 | Switch | Lệnh và phản hồi relay: 1 bật, 0 tắt |
| V3 | Integer, 0…1 | LED/Value | Trạng thái Wi-Fi tại thời điểm gửi |

Tắt tùy chọn tự đồng bộ giá trị Cloud xuống thiết bị khi kết nối cho V2 nếu đang bật. Firmware chủ động dùng trạng thái GPIO để cập nhật App. V3 là mẫu kết nối cuối cùng, không thay thế chỉ báo Device Online/Offline của Blynk khi thiết bị mất mạng.

## Build và chạy

```bash
pio run
pio run -t upload
pio device monitor
```

Chạy trong thư mục bài tương ứng. Nếu thư mục core PlatformIO mặc định không ghi được, dùng `PLATFORMIO_CORE_DIR=/tmp/pio-iot pio run`. Serial Monitor dùng 115200 baud. Cần cấu hình credentials và thiết bị thật để kiểm chứng App; build thành công chưa xác nhận đồng bộ qua Cloud.

GPIO đọc lại là mức logic của chân điều khiển; muốn xác nhận tiếp điểm relay/tải thực sự hoạt động cần cảm biến phản hồi riêng.
