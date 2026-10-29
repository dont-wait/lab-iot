# Bài 5 – Đo độ trễ và độ tin cậy của vòng lặp phản hồi

## Mô hình

ESP32 nhận lệnh Switch trên Blynk V0, áp dụng trạng thái cho LED ở GPIO 2, rồi gửi trạng thái thực tế lên V1 ngay trong callback. V2 hiển thị độ trễ xử lý nội bộ của callback (ms). Firmware cũng in kết quả từng lệnh và thống kê min/avg/max trên Serial Monitor.

| Datastream | Kiểu | Hướng | Mục đích |
|---|---|---|---|
| V0 – LED command | Integer 0/1 | App → ESP32 | Switch điều khiển |
| V1 – LED feedback | Integer 0/1 | ESP32 → App | Trạng thái đã áp dụng |
| V2 – Callback time | Integer (ms) | ESP32 → App | Thời gian nhận callback → ghi GPIO → xếp phản hồi |

V0–V2 cần được tạo trong cùng Blynk Template, rồi thay Template ID, Auth Token và Wi-Fi trong `include/config.h`. Nối LED với điện trở 220–330 Ω vào GPIO 2 và GND.

## Kịch bản đo

1. Mở Blynk Web Dashboard/App, Serial Monitor ở 115200 baud và bật/tắt Switch V0 luân phiên ít nhất 30 lần.
2. Xác nhận đèn GPIO 2 đổi theo lệnh; V1 phản hồi đúng cùng trạng thái. Serial in một dòng `COMMAND` cho mỗi lệnh và định kỳ in `LATENCY` với số mẫu, lần gần nhất, trung bình, nhỏ nhất và lớn nhất.
3. Ghi thời điểm thao tác trong App và thời điểm V1 đổi trên dashboard để ước lượng round-trip nhìn thấy được. So sánh với callback time V2/Serial.
4. Lặp lại trong điều kiện mạng ổn định và khi có lưu lượng khác, rồi so sánh số lệnh nhận đủ, sai lệch trạng thái và các thống kê.

Callback time đo được trong firmware chỉ gồm thời gian từ đầu callback đến khi trạng thái GPIO được áp dụng và lệnh phản hồi được đưa vào hàng gửi Blynk. Nó không đo trọn thời gian đi qua Internet hoặc thời gian render của Web/App. Độ trễ end-to-end cần bấm giờ từ thao tác App đến lúc V1 đổi; Blynk không cung cấp timestamp gốc của lần chạm thông qua callback này.

## Tần suất và tải mạng

Phản hồi sau lệnh được gửi ngay để giữ vòng điều khiển nhanh. Heartbeat/trạng thái định kỳ mặc định 5 giây, thống kê Serial 10 giây; không gửi telemetry liên tục trong từng vòng `loop()`. Khi đánh giá tải, thử chu kỳ 2 giây, 5 giây và 10 giây, mỗi mức trong cùng khoảng thời gian; theo dõi mất phản hồi và độ trễ. Chọn chu kỳ dài nhất vẫn đáp ứng nhu cầu giám sát để giảm lưu lượng và áp lực buffer. Khi vận hành, không gửi lại trạng thái nếu không đổi trừ heartbeat cần thiết.

Không cài thư viện hoặc build/chạy thử theo yêu cầu. `platformio.ini` chỉ khai báo dependency để có thể build sau này.
