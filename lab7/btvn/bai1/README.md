# Bài 1 - Dashboard trạm quan sát môi trường

Project ESP32 đọc nhiệt độ và độ ẩm từ DHT22, sau đó gửi dữ liệu lên Blynk
Cloud mỗi 2 giây. Dashboard cần có 2 Gauge, 1 Line Chart và 1 Value Display
để hiển thị trạng thái kết nối.

## 1. Cấu hình Blynk Template

Tạo một Template mới trên Blynk Console:

```text
Tên: Smart Environment Dashboard
Hardware: ESP32
Connection: WiFi
```

Tạo các Datastream sau:

| Tên | Virtual Pin | Kiểu dữ liệu | Min | Max | Đơn vị |
|---|---:|---|---:|---:|---|
| Nhiệt độ | V0 | Double | 0 | 50 | °C |
| Độ ẩm | V1 | Double | 0 | 100 | % |
| Trạng thái kết nối | V2 | String | - | - | - |

Tạo một Device từ Template và lấy các thông tin:

- Template ID.
- Template Name.
- Auth Token.

## 2. Cập nhật chương trình

Mở `src/main.cpp` và thay ba giá trị sau bằng thông tin từ Blynk:

```cpp
#define BLYNK_TEMPLATE_ID "TMPL_REPLACE_ME"
#define BLYNK_TEMPLATE_NAME "Lab7 Environment Dashboard"
#define BLYNK_AUTH_TOKEN "REPLACE_WITH_DEVICE_TOKEN"
```

Chương trình sử dụng WiFi mô phỏng của Wokwi:

```cpp
WIFI_SSID = "Wokwi-GUEST"
WIFI_PASSWORD = ""
```

Khi chạy trên ESP32 thật, thay hai giá trị này bằng WiFi thực tế.

## 3. Thiết kế Dashboard

Trong **Web Dashboard** của Template, thêm các widget:

1. **Gauge - Nhiệt độ**
   - Datastream: `Nhiệt độ (V0)`.
   - Min: `0`, Max: `50`.
   - Đơn vị: `°C`.
2. **Gauge - Độ ẩm**
   - Datastream: `Độ ẩm (V1)`.
   - Min: `0`, Max: `100`.
   - Đơn vị: `%`.
3. **Chart - Nhiệt độ theo thời gian**
   - Thêm data series `Nhiệt độ (V0)`.
   - Chọn kiểu Line và bật khoảng thời gian Live/1h/1d.
4. **Value Display - Trạng thái kết nối**
   - Datastream: `Trạng thái kết nối (V2)`.
   - Đặt nhãn `Gateway status`.

Có thể tạo bố cục gồm một khu vực cảm biến với hai Gauge, một khu vực biểu đồ
và một Value Display ở đầu trang để quan sát trạng thái thiết bị.

## 4. Build và chạy với PlatformIO/Wokwi

```bash
cd lab7/btvn/bai1
pio run
```

Sau khi build, mở `diagram.json` bằng Wokwi for VS Code và chọn **Start
Simulation**. DHT22 được nối với GPIO 15. Thay đổi giá trị nhiệt độ/độ ẩm trên
linh kiện DHT22 để kiểm tra Dashboard.

Serial Monitor chạy ở tốc độ 115200:

```text
Nhiet do: 28.0 C | Do am: 60.0 % | Blynk: ONLINE
```

## 5. Kết quả cần kiểm tra

- Device hiển thị **Online** trên Blynk Console.
- Gauge nhiệt độ và độ ẩm cập nhật khoảng mỗi 2 giây.
- Line Chart nhận dữ liệu V0 liên tục.
- Value Display hiển thị `ONLINE` khi Blynk kết nối.
- Khi mất kết nối, chương trình không gửi dữ liệu sai và tự kết nối lại theo
  cơ chế của Blynk.
- Độ trễ cập nhật dữ liệu không vượt quá 5 giây trong điều kiện mạng ổn định.
