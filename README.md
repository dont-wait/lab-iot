# Bài tập IoT

> Kho lưu trữ các bài thực hành IoT sử dụng Arduino/ESP32, PlatformIO và mô phỏng Wokwi.

## Thành viên

| STT | MSSV | Họ và tên |
|---:|:---:|---|
| 1 | 2001230753 | Nguyễn Tấn Sang |
| 2 | 2001230692 | Võ Đức Phúc |
| 3 | 2001230244 | Phạm Đình Minh Hiếu |
| 4 | 2001230963 | Phan Khắc Duy Thức |
| 5 | 2001230778 | Hứa Quang Tâm |
| 6 | 2001230893 | Tô Trương Trường Thành |

## Nội dung các bài

### Lab 1

#### Bài trên lớp (`lab1/bttl`)

| Bài | Nội dung | Nền tảng |
|---|---|---|
| [Bài 1](lab1/bttl/bai1) | Mô phỏng đèn giao thông với ba LED đỏ, vàng, xanh; mỗi đèn sáng theo khoảng thời gian định trước. | Arduino Uno |
| [Bài 2](lab1/bttl/bai2) | Điều khiển LED bằng nút nhấn: nhấn nút bật đèn, nhả nút tắt đèn. | Arduino Uno |
| [Bài 3](lab1/bttl/bai3) | Điều khiển LED bằng nút nhấn có chống dội phím; mỗi lần nhấn hợp lệ sẽ đảo trạng thái LED. | Arduino Uno |
| [Bài 4](lab1/bttl/bai4) | Đọc nhiệt độ và độ ẩm từ DHT22, hiển thị trên Serial Monitor và nhấp nháy LED khi nhiệt độ vượt 30°C. | Arduino Uno |
| [Bài 5](lab1/bttl/bai5) | Hiển thị thời gian hoạt động của board theo giây trên LCD 16x2 giao tiếp I2C. | Arduino Uno |

#### Bài về nhà (`lab1/btvn`)

| Bài | Nội dung | Nền tảng |
|---|---|---|
| [Bài 1](lab1/btvn/bai1) | Đọc DHT22, hiển thị nhiệt độ và độ ẩm trên LCD; nhấn nút để xem nhiệt độ Fahrenheit trong 5 giây; LED cảnh báo khi độ ẩm trên 80%. | ESP32 |
| [Bài 2](lab1/btvn/bai2) | Xây dựng đồng hồ bấm giờ trên LCD với nút A để bắt đầu/tạm dừng và nút B để đặt lại. | ESP32 |
| [Bài 03](lab1/btvn/bai03) | Menu ba trang trên LCD: xem nhiệt độ, xem độ ẩm và xem trạng thái hệ thống; chuyển trang bằng nút nhấn. | ESP32 |

### Lab 2

#### Bài trên lớp (`lab2/bttl`)

| Bài | Nội dung | Nền tảng |
|---|---|---|
| [Bài 1](lab2/bttl/bai1) | Điều khiển hai LED nhấp nháy độc lập với các chu kỳ thời gian khác nhau. | ESP32 |
| [Bài 2](lab2/bttl/bai2) | Nhấn nút để bật/tắt đồng thời hai LED, có chống dội phím. | ESP32 |
| [Bài 3](lab2/bttl/bai3) | LED heartbeat nhấp nháy định kỳ và ghi uptime của hệ thống ra Serial Monitor. | ESP32 |
| [Bài 4](lab2/bttl/bai4) | Đọc DHT theo tác vụ FreeRTOS chạy trên core riêng, đồng thời xử lý nút nhấn và LED ở vòng lặp chính. | ESP32 |
| [Bài 5](lab2/bttl/bai5) | Kết hợp LED nhấp nháy theo thời gian, LED đổi trạng thái bằng nút nhấn và thống kê số lần chạy của `loop()`. | ESP32 |

#### Bài về nhà (`lab2/btvn`)

| Bài | Nội dung | Nền tảng |
|---|---|---|
| [Bài 6](lab2/btvn/bai6) | Mô phỏng đèn giao thông với máy trạng thái: đỏ 5 giây, xanh 4 giây, vàng 2 giây. | ESP32 |
| [Bài 7](lab2/btvn/bai7) | Đọc nhiệt độ DHT bằng FreeRTOS và queue; bật còi 2 kHz nhấp nháy khi nhiệt độ vượt 30°C. | ESP32 |
| [Bài 8](lab2/btvn/bai8) | Điều chỉnh độ sáng LED bằng PWM qua nút nhấn theo 5 mức, đồng thời cho LED nguồn nhấp nháy chu kỳ 1,5 giây. | ESP32 |

## Cấu trúc thư mục

```text
.
├── lab1/
│   ├── bttl/       # Bài thực hành trên lớp
│   └── btvn/       # Bài tập về nhà
└── lab2/
    ├── bttl/       # Bài thực hành trên lớp
    └── btvn/       # Bài tập về nhà
```

Mỗi thư mục bài thường gồm:

- `platformio.ini`: cấu hình board, framework và thư viện.
- `diagram.json`: sơ đồ linh kiện dùng trong Wokwi.
- `src/main.cpp` hoặc `sketch.ino`: mã nguồn chương trình.
- `wokwi.toml`: cấu hình mô phỏng Wokwi.

## Công cụ sử dụng

- [PlatformIO](https://platformio.org/)
- [Wokwi](https://wokwi.com/)
- Arduino Uno và ESP32 DevKit
- C++/Arduino Framework

## Cách chạy một bài

Di chuyển vào thư mục bài cần chạy, sau đó dùng PlatformIO:

```bash
pio run
pio run -t upload
pio device monitor
```

Các bài có `diagram.json` có thể mở trực tiếp bằng Wokwi để mô phỏng mạch và quan sát kết quả.
