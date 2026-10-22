# Bài 5 - Dashboard đồng bộ Web và Mobile

Dashboard Node-RED cho ESP32, bố cục responsive dùng chung trên Web Console và
điện thoại:

- **🌡️ Khu vực cảm biến**: gauge nhiệt độ/độ ẩm và hai biểu đồ lịch sử.
- **🎛️ Khu vực điều khiển**: công tắc đèn, bật đèn và đảo trạng thái.
- **ℹ️ Khu vực hệ thống**: thiết bị, trạng thái đèn và thời điểm cập nhật.

## Chạy bài

```bash
npm install -g --unsafe-perm node-red
cd ~/.node-red
npm install node-red-dashboard
node-red
```

Trong Node-RED chọn **Menu → Import → select a file**, chọn `flows.json`, rồi
**Deploy**. Truy cập `http://127.0.0.1:1880/ui` trên máy tính; trên điện thoại
dùng `http://<IP-máy-chạy-Node-RED>:1880/ui` trong cùng Wi-Fi.

Flow dùng broker thử nghiệm `broker.hivemq.com:1883` và các topic:

| Dữ liệu | MQTT topic |
|---|---|
| Cảm biến | `home/living_room/sensor` |
| Lệnh điều khiển | `home/living_room/control` |
| Trạng thái | `home/living_room/status` |

Khi mở trên mobile, các widget rộng 6 cột sẽ tự xếp dọc; trên Web chúng nằm cạnh
nhau. `flows.json` là cấu hình dashboard đã lưu để import/nộp bài.
