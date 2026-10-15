# Bài 1 - Cài đặt và cấu hình Mosquitto Local Broker

## Mục tiêu

Cài Mosquitto trên Gateway và chạy MQTT broker tại cổng `1883` để các client trong mạng LAN có thể kết nối.

## Cài đặt

```bash
sudo apt update
sudo apt install -y mosquitto mosquitto-clients
```

## Áp dụng cấu hình

Sao chép file cấu hình của bài này vào thư mục cấu hình Mosquitto:

```bash
sudo cp mosquitto.conf /etc/mosquitto/conf.d/lab5.conf
sudo systemctl enable mosquitto
sudo systemctl restart mosquitto
sudo systemctl status mosquitto --no-pager
```

Kiểm tra broker đang lắng nghe cổng `1883`:

```bash
ss -ltn | grep ':1883'
```

File `mosquitto.conf` cho phép kết nối ẩn danh để thuận tiện kiểm thử trong lớp. Không dùng cấu hình này trên mạng Internet; bài bảo mật sẽ tắt chế độ ẩn danh.

