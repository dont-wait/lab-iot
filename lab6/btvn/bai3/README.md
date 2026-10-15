# Lab 6 - Bài tập về nhà 3: Phản hồi ACK qua MQTT

ESP32 subscribe `home/device/cmd`. Gửi lệnh bật/tắt đèn:

```json
{"device":"light","action":"ON"}
```

Hoặc `{"device":"light","action":"OFF"}`. Sau khi cập nhật GPIO18, ESP32 publish trạng thái lên `home/device/status`, ví dụ:

```json
{"device":"light","status":"ON","timestamp_ms":12345,"message":"Execution successful","success":true}
```

`timestamp_ms` là thời gian chạy kể từ khởi động ESP32 (`millis()`), không phải Unix timestamp. Lệnh JSON không hợp lệ hoặc không hỗ trợ cũng nhận ACK với `success:false`.

Trong Wokwi, LED vàng GPIO18 mô phỏng đèn. Kết nối Wi-Fi `Wokwi-GUEST` và MQTT broker `broker.hivemq.com:1883`; trên MQTT Explorer subscribe `home/device/status`, publish lệnh lên `home/device/cmd`, rồi xác nhận LED và ACK. Broker công khai không có mã hóa/xác thực, chỉ dùng để thực hành.
