# Bài 2 – Auto-recovery và đồng bộ trạng thái

Firmware ESP32 tiếp tục điều khiển đèn bằng nút vật lý khi Internet/Blynk mất kết nối. Nó thử phục hồi Wi-Fi và Blynk theo chu kỳ, không chờ vô hạn; sau khi Blynk trở lại, firmware gửi trạng thái cục bộ hiện tại lên dashboard.

## Phần cứng và datastream

| Thiết bị / dữ liệu | Kết nối |
|---|---|
| LED (qua điện trở) hoặc relay | GPIO 2 |
| Nút nhấn | GPIO 4 nối GND, dùng `INPUT_PULLUP` |
| Trạng thái đèn | Blynk V0, Integer, 0–1, Switch |
| Trạng thái Wi-Fi | Blynk V1, Integer, 0–1, Value/LED |

Điền Blynk Template ID, Auth Token và thông tin Wi-Fi trong `include/config.h`. Khi dùng thiết bị thật, kiểm tra mức kích hoạt relay. Không đưa Auth Token thật lên Git.

## Cơ chế phục hồi

- Wi-Fi được thử lại mỗi 5 giây; mỗi vòng chỉ khởi động một lần thử, không có vòng `while` chờ mạng.
- Khi Wi-Fi đã lên nhưng Blynk chưa kết nối, firmware gọi `Blynk.connect(1000)` tối đa một giây mỗi lần, cách nhau 5 giây. Trong lúc đó vòng lặp vẫn quét nút và chạy Blynk.
- Nút vật lý vẫn đổi `lightState` và GPIO khi offline. Giá trị app được bỏ qua khi không có kết nối.
- Khi Blynk kết nối lại, `BLYNK_CONNECTED()` publish `lightState` và trạng thái Wi-Fi lên V0/V1. ESP32 là nguồn dữ liệu chuẩn, nên trạng thái cũ trên cloud không ghi đè trạng thái đèn tại chỗ.

## Mô phỏng mất mạng

Trong Wokwi hoặc môi trường chạy thật, cho thiết bị bắt đầu online, sau đó ngắt Internet/Wi-Fi trong khi vẫn giữ ESP32 chạy. Bấm nút vật lý vài lần để thay đổi đèn khi offline; bật lại mạng và quan sát Serial/App. Sau khi Blynk kết nối lại, V0 phải nhận giá trị `lightState` cuối cùng và V1 phản ánh Wi-Fi.

Theo yêu cầu, mã nguồn đã được đóng gói nhưng không cài thư viện, không build hoặc chạy mô phỏng trong lượt này.
