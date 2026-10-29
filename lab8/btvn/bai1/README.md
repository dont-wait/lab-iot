# Bài 1 – Điều khiển và giám sát đèn hai chiều

Firmware ESP32 điều khiển LED/relay bằng nút nhấn vật lý hoặc Switch trên Blynk. Cả hai đầu vào đều cập nhật cùng biến `lightState` và đầu ra GPIO, nhờ vậy trạng thái app và đèn theo cùng một nguồn trạng thái.

## Phần cứng

| Linh kiện | ESP32 | Ghi chú |
|---|---|---|
| LED (qua điện trở 220–330 Ω) hoặc module relay | GPIO 2 | Mặc định active-high |
| Nút nhấn | GPIO 4 và GND | Dùng `INPUT_PULLUP`, nhấn là LOW |

Không nối tải điện lưới trực tiếp vào ESP32. Với relay, dùng module phù hợp và kiểm tra mức kích hoạt của module.

## Blynk

Tạo Template/device, datastream **V0** kiểu Integer, min 0, max 1; thêm Switch gắn V0. Thay Template ID, Auth Token và Wi-Fi trong `include/config.h`. Token thật không nên commit vào Git.

## Đồng bộ trạng thái

- Nhấn nút vật lý: debounce bằng `millis()`, chỉ toggle một lần ở cạnh nhấn, cập nhật GPIO và gửi giá trị mới lên V0 khi online.
- Nhấn Switch trong app: `BLYNK_WRITE(V0)` cập nhật cùng biến và GPIO.
- Khi kết nối/reconnect Blynk: `BLYNK_CONNECTED()` gửi trạng thái thực tế hiện tại của ESP32 lên V0 để app khớp với thiết bị.

Project PlatformIO đặt mã trong `src/main.cpp`, cấu hình trong `include/config.h`. Không cài thư viện hoặc chạy thử theo yêu cầu.
