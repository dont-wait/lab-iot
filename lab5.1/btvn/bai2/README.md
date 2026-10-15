# Bài 2 - Mosquitto Access Control

Cấu hình MQTT broker trên Gateway để:

- Tắt truy cập ẩn danh bằng `allow_anonymous false`.
- Xác thực client bằng file mật khẩu được tạo bởi `mosquitto_passwd`.
- Phân quyền topic bằng ACL:
  - `gateway-client`: đọc dữ liệu sensor và publish report.
  - `sensor-client`: chỉ publish dữ liệu sensor.
- Kiểm chứng client không có hoặc có sai thông tin đăng nhập sẽ bị từ chối.

## 1. Cài đặt Mosquitto

Trên Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y mosquitto mosquitto-clients
```

Nếu service Mosquitto mặc định đang chạy trên cổng 1883, dừng service trước
khi chạy cấu hình của bài:

```bash
sudo systemctl stop mosquitto
```

## 2. Tạo file mật khẩu

Di chuyển vào đúng thư mục bài và cấp quyền thực thi cho các script:

```bash
cd /home/kilious-kry/Code/school/IoT/lab-iot/lab5.1/btvn/bai2
chmod +x setup_auth.sh run_broker.sh test_auth.sh
```

Tạo file `passwd` bằng `mosquitto_passwd`:

```bash
./setup_auth.sh
```

Script sẽ hỏi hai mật khẩu:

| Tài khoản | Quyền |
|---|---|
| `gateway-client` | Subscribe `sensor/+/data`, publish `gateway/aggregated_report` |
| `sensor-client` | Publish `sensor/+/data` |

Mật khẩu không được lưu dạng rõ mà được lưu dưới dạng hash trong file
`passwd`. File này đã được thêm vào `.gitignore` và có permission `600`.

## 3. Khởi động broker có xác thực

```bash
./run_broker.sh
```

File `mosquitto.conf` sử dụng các thiết lập chính:

```conf
listener 1883 0.0.0.0
allow_anonymous false
password_file ./passwd
acl_file ./aclfile
```

Giữ terminal này để xem log của broker.

## 4. Kiểm chứng xác thực

Mở terminal thứ hai, chạy:

```bash
cd /home/kilious-kry/Code/school/IoT/lab-iot/lab5.1/btvn/bai2
./test_auth.sh
```

Kết quả mong đợi:

```text
PASS: Client dang nhap dung va duoc phep publish.
PASS: Client khong co username/password bi tu choi.
PASS: Client sai mat khau bi tu choi.
Hoan tat kiem tra access control.
```

Hoặc kiểm tra thủ công:

```bash
# Đúng tài khoản: thành công
mosquitto_pub -h 127.0.0.1 -p 1883 \
  -u gateway-client -P 'MAT_KHAU_GATEWAY' \
  -t gateway/aggregated_report -m '{"ok":true}'

# Không có tài khoản: thất bại
mosquitto_pub -h 127.0.0.1 -p 1883 \
  -t gateway/aggregated_report -m '{"anonymous":true}'

# Sai mật khẩu: thất bại
mosquitto_pub -h 127.0.0.1 -p 1883 \
  -u gateway-client -P 'sai-mat-khau' \
  -t gateway/aggregated_report -m '{"wrong":true}'
```

Kiểm tra quyền của sensor client:

```bash
# Được phép publish dữ liệu sensor
mosquitto_pub -h 127.0.0.1 -p 1883 \
  -u sensor-client -P 'MAT_KHAU_SENSOR' \
  -t sensor/node-01/data -m '{"temperature":28.5,"humidity":65}'
```

Sau khi cấu hình xong, Bài 1 có thể kết nối broker bảo mật bằng:

```bash
python aggregator.py \
  --broker-host 127.0.0.1 \
  --username gateway-client \
  --password 'MAT_KHAU_GATEWAY'
```

Nhấn `Ctrl+C` trong terminal chạy broker để dừng Mosquitto.
