# Bài 4 - Widget điều khiển thiết bị

Project điều khiển LED từ Blynk theo hai chiều: Switch V3 bật/tắt thiết bị và Slider V4 đặt độ sáng từ 0 đến 100%. ESP32 áp dụng PWM cho LED rồi gửi lại giá trị thực tế để giao diện luôn đồng bộ.

## 1. Datastream trên Blynk Console

Tạo Template và hai Virtual Pin Datastream:

| Tên | Pin | Kiểu dữ liệu | Min | Max | Giá trị mặc định |
|---|---|---:|---:|---:|---:|
| Bật tắt đèn | V3 | Integer | 0 | 1 | 0 |
| Độ sáng | V4 | Integer | 0 | 100 | 50 |

Trong `src/main.cpp`, thay `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME` và `BLYNK_AUTH_TOKEN` bằng thông tin của thiết bị.

## 2. Cấu hình widget

1. Trên **Web Dashboard**, thêm widget **Switch** và chọn datastream **Bật tắt đèn (V3)**.
2. Thêm widget **Slider**, chọn datastream **Độ sáng (V4)**, đặt Min = 0, Max = 100 và Step = 1.
3. Nếu dùng Mobile Dashboard, thêm Switch/Slider tương tự và gán đúng V3/V4.
4. Lưu dashboard, mở thiết bị và chờ trạng thái Online.

## 3. Chạy bằng PlatformIO và Wokwi

```powershell
cd lab7/bttl/bai4
pio run
```

Mở `diagram.json` bằng Wokwi for VS Code và Start Simulation. Switch V3 quyết định LED bật hay tắt. Slider V4 thay đổi độ sáng khi công tắc đang bật.

## 4. Luồng điều khiển và phản hồi

- Đổi Switch: Blynk gửi V3 xuống ESP32, `BLYNK_WRITE(V3)` cập nhật trạng thái và PWM.
- Kéo Slider: Blynk gửi V4 xuống ESP32, `BLYNK_WRITE(V4)` giới hạn giá trị 0-100 rồi đổi sang PWM 0-255.
- Sau mỗi lệnh, ESP32 ghi lại V3 và V4 lên Blynk nên widget phản ánh đúng trạng thái đã áp dụng.
- Khi kết nối lại, `Blynk.syncVirtual(V3, V4)` lấy giá trị gần nhất từ cloud thay vì tự ý đặt lại widget.

## 5. Kết quả cần kiểm tra

- Switch ON làm LED sáng theo mức Slider; Switch OFF tắt LED.
- Slider 0 làm LED tắt, 50 cho độ sáng trung bình, 100 cho độ sáng tối đa.
- Serial Monitor in trạng thái công tắc, độ sáng và duty PWM sau mỗi thao tác.
- Sau khi ESP32 mất mạng rồi kết nối lại, V3/V4 được đồng bộ và giao diện vẫn nhất quán.

