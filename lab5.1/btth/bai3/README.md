# Bài 3 - Kết nối MQTT bằng Python Paho-MQTT

## Mục tiêu

Tạo môi trường Python riêng và kiểm tra Gateway có thể kết nối đến MQTT broker tại `127.0.0.1:1883`.

## Cài đặt môi trường

```bash
python3 -m venv venv
source venv/bin/activate
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

## Chạy bài

Đảm bảo Mosquitto ở Bài 1 đang chạy, sau đó:

```bash
python mqtt_client.py
```

Kết quả thành công:

```text
Kết nối MQTT thành công: 127.0.0.1:1883
```

Nếu broker chưa chạy, chương trình báo lỗi kết nối hoặc hết thời gian chờ. Script dùng Callback API phiên bản 2 của `paho-mqtt` và luôn dừng network loop trước khi thoát.

