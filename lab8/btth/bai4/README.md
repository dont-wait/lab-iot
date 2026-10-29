# Bài 4 – Đồng bộ trạng thái khi khởi động

Sau khi cấp nguồn hoặc reset, `setup()` đặt relay về OFF trước khi kết nối Wi-Fi/Cloud. Khi kết nối thành công, `BLYNK_CONNECTED()` gọi `sendSensorData()` để đọc GPIO và gửi trạng thái hiện tại lên V2, cùng trạng thái Wi-Fi V3. Callback cũng chạy khi kết nối lại sau mất mạng.

Chiều đồng bộ là **ESP32 → App**: giá trị ON cũ trên Cloud được cập nhật thành OFF khi phần cứng vừa reset. Bài này không lưu trạng thái relay qua mất điện. Không gọi `Blynk.syncVirtual(V2)` vì lệnh đó lấy giá trị đã lưu trên Cloud và kích hoạt callback nhận lệnh, khác với yêu cầu gửi trạng thái GPIO lên App. Xem [tài liệu Blynk về đồng bộ trạng thái](https://docs.blynk.io/en/blynk-library-firmware-api/state-syncing).

Giữ cơ chế nút nhấn chống dội và phản hồi tức thì của bài 3. Kết nối được thử lại theo chu kỳ để nút vật lý tiếp tục hoạt động khi chưa có Cloud.

## Kiểm chứng reset cứng

1. Nạp firmware với credentials hợp lệ, chờ Online và bật relay bằng Switch V2. Xác nhận GPIO/LED/relay đang ON và V2 = 1.
2. Nhấn nút EN/RESET trên ESP32 hoặc ngắt rồi cấp nguồn. Không thao tác Switch trong lúc thiết bị khởi động lại.
3. Phần cứng về OFF; Serial in `BOOT relay=OFF`. Khi Cloud kết nối thành công, Serial in `SYNC ON CONNECT: GPIO -> App` và `STATE relay=OFF | cloud=connected`.
4. Xác nhận V2 tự về 0/OFF đúng với phần cứng. Không cần nhấn nút hoặc chờ timer cảm biến.
5. Thử reset khi Wi-Fi tắt, dùng nút GPIO 18 bật relay trước khi nối mạng lại. Khi Online, V2 phải nhận ON hiện tại thay vì ép lại OFF hoặc giá trị cũ trên Cloud.
6. Thử ngắt/nối Wi-Fi mà không reset: trạng thái relay giữ nguyên và được gửi lại sau kết nối. Ghi kết quả thực tế và độ trễ quan sát trên App.

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
