# Bài 3 - Node-RED Gateway Dashboard

Flow Node-RED trong bài này nhận dữ liệu từ MQTT broker cục bộ trên Gateway và
hiển thị nhiệt độ, độ ẩm bằng Gauge và Chart tại:

```text
http://127.0.0.1:1880/ui
```

Flow hỗ trợ cả hai nguồn dữ liệu:

- `sensor/+/data`: dữ liệu trực tiếp từ sensor node.
- `gateway/aggregated_report`: report tổng hợp do Bài 1 publish.

## 1. Cài đặt Node-RED

Chạy script cài đặt trên Gateway:

```bash
cd /home/kilious-kry/Code/school/IoT/lab-iot/lab5.1/btvn/bai3
chmod +x install_node_red.sh
./install_node_red.sh
```

Script cài Node.js/npm nếu còn thiếu, cài Node-RED và cài package
`node-red-dashboard` vào thư mục user `~/.node-red`.

## 2. Import flow

Khởi động Node-RED:

```bash
node-red
```

Mở `http://127.0.0.1:1880`, chọn **Menu → Import → select a file** và chọn
`flows.json`. Sau đó bấm **Deploy**.

### Cấu hình xác thực MQTT

Flow đã cấu hình broker mặc định:

```text
host: localhost
port: 1883
```

Nếu đang dùng cấu hình bảo mật của Bài 2, mở node **Local Broker (Gateway)**,
nhập:

```text
Username: gateway-client
Password: mật khẩu đã tạo bằng setup_auth.sh ở Bài 2
```

Sau đó bấm **Done → Deploy**. Không ghi mật khẩu thật vào file `flows.json`.

Nếu broker vẫn cho phép anonymous, có thể để trống username/password.

## 3. Kiểm thử dữ liệu

Giữ Node-RED chạy, mở terminal khác và gửi dữ liệu trực tiếp từ sensor:

```bash
mosquitto_pub -h localhost -p 1883 \
  -u sensor-client -P 'MAT_KHAU_SENSOR' \
  -t sensor/node-01/data \
  -m '{"temperature":28.5,"humidity":65}'
```

Hoặc gửi một report tổng hợp tương thích với Bài 1:

```bash
mosquitto_pub -h localhost -p 1883 \
  -u gateway-client -P 'MAT_KHAU_GATEWAY' \
  -t gateway/aggregated_report \
  -m '{"gateway_id":"gateway-01","sensors":{"node-01":[{"data":{"temperature":29.1,"humidity":64}}]}}'
```

Mở lại `http://127.0.0.1:1880/ui` để xem:

- Gauge nhiệt độ hiện tại.
- Gauge độ ẩm hiện tại.
- Chart lịch sử nhiệt độ theo từng node.
- Chart lịch sử độ ẩm theo từng node.

## 4. Kiểm tra kết nối

Trong Node-RED, node MQTT sẽ báo trạng thái `connected` khi kết nối thành công.
Nếu báo `connection refused`, hãy kiểm tra broker đang chạy tại cổng 1883.
Nếu báo lỗi xác thực, kiểm tra username/password trong node **Local Broker
(Gateway)**.

Nhấn `Ctrl+C` trong terminal chạy Node-RED để dừng chương trình.
