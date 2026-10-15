# Bài 3: ESP32 gửi dữ liệu DHT22 bằng HTTP POST

## Mục tiêu

- Kết nối ESP32 vào Wi-Fi.
- Đọc nhiệt độ và độ ẩm từ DHT22.
- Gửi dữ liệu JSON lên Webhook.site bằng `HTTPClient`.
- Quan sát kết quả trên Serial Monitor.

## Chuẩn bị Webhook.site

1. Truy cập <https://webhook.site> và tạo một webhook mới.
2. Sao chép URL có dạng `https://webhook.site/<UUID>`.
3. Mở `src/main.cpp` và thay `YOUR-UUID` trong `WEBHOOK_URL` bằng UUID thật.

## Chạy bằng PlatformIO

```bash
pio run
pio device monitor
```

Khi chạy Wokwi, Wi-Fi sử dụng SSID `Wokwi-GUEST` và không có mật khẩu.

## Kết quả mong đợi

Serial Monitor phải hiển thị dữ liệu cảm biến, JSON gửi đi và mã phản hồi HTTP:

```text
Wi-Fi da ket noi, dia chi IP: ...
Nhiet do: 25.00 C | Do am: 60.00 %
HTTP POST payload:
{"device":"ESP32","sensor":"DHT22","temperature_c":25.00,"humidity_percent":60.00,"uptime_ms":...}
HTTP Response code: 200
```

Chụp `diagram.json` trên Wokwi và Serial Monitor có `HTTP Response code: 200` để đưa vào báo cáo.
