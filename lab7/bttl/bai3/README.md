# Bài 3 - Biểu đồ theo dõi lịch sử dữ liệu

Project ESP32 gửi độ ẩm và mức ánh sáng lên Blynk IoT mỗi 2 giây. Widget Chart dùng dữ liệu đã lưu của hai Virtual Pin để xem trực tiếp, 1 giờ qua hoặc 1 ngày qua.

## 1. Datastream trên Blynk Console

Tạo Template mới, sau đó tạo hai Virtual Pin Datastream:

| Tên | Pin | Kiểu dữ liệu | Min | Max | Đơn vị |
|---|---|---:|---:|---:|---|
| Độ ẩm | V1 | Double | 0 | 100 | % |
| Ánh sáng | V2 | Double | 0 | 100 | % |

Trong `src/main.cpp`, thay `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME` và `BLYNK_AUTH_TOKEN` bằng thông tin của thiết bị trên Blynk.

## 2. Thêm History Chart

1. Mở **Web Dashboard** của Template và kéo widget **Chart** vào dashboard.
2. Trong Chart, thêm data series **Độ ẩm (V1)** và **Ánh sáng (V2)**. Chọn hai màu khác nhau và kiểu Line.
3. Lưu dashboard. Khi thiết bị Online, chọn các khoảng **Live**, **1h** và **1d** trên Chart để quan sát lịch sử.
4. Nếu dùng Wokwi, bấm vào DHT22 hoặc photoresistor trong lúc mô phỏng và thay đổi thanh giá trị. Đường biểu đồ sẽ nhận điểm dữ liệu mới sau tối đa khoảng 2 giây.

> Blynk lưu lịch sử ở phía cloud. Chế độ 1 giờ và 1 ngày chỉ có dữ liệu sau khi thiết bị đã gửi dữ liệu trong khoảng thời gian tương ứng.

## 3. Chạy bằng PlatformIO và Wokwi

```powershell
cd lab7/bttl/bai3
pio run
```

Sau khi build, mở `diagram.json` bằng Wokwi for VS Code và Start Simulation. Serial Monitor hiển thị độ ẩm và phần trăm ánh sáng đã gửi.

## 4. Kết quả cần kiểm tra

- Thiết bị hiện Online trên Blynk.
- Serial Monitor có dữ liệu mới mỗi 2 giây.
- Chart có hai đường V1 và V2.
- Khi đổi độ ẩm/ánh sáng trong Wokwi, đường tương ứng cập nhật.
- Có thể chuyển giữa Live, 1h và 1d mà không đổi chương trình ESP32.

