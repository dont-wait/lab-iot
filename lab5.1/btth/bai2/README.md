# Bài 2 - Kiểm tra kết nối MQTT bằng CLI

Bài này kiểm tra đường truyền bản tin giữa `mosquitto_sub` và `mosquitto_pub` thông qua broker local ở `localhost:1883`.

## Chuẩn bị

Đảm bảo broker từ Bài 1 đang chạy:

```bash
sudo systemctl status mosquitto --no-pager
```

## Thực hiện

Mở Terminal 1 và chạy subscriber:

```bash
mosquitto_sub -h localhost -p 1883 -t "gateway/test" -v
```

Mở Terminal 2 và chạy publisher:

```bash
mosquitto_pub -h localhost -p 1883 -t "gateway/test" -m "Hello Local MQTT"
```

Terminal 1 phải hiển thị:

```text
gateway/test Hello Local MQTT
```

Có thể gửi thêm bản tin có thời gian để kiểm tra nhiều lần:

```bash
mosquitto_pub -h localhost -t "gateway/test" -m "Hello Local MQTT $(date '+%F %T')"
```

## Kết luận

Subscriber nhận được đúng topic `gateway/test` và payload `Hello Local MQTT`, chứng minh MQTT client đã kết nối và truyền nhận thành công qua local broker.

