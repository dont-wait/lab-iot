# Bài 1 - Gateway Data Aggregator

Gateway subscribe wildcard topic `sensor/+/data`, gom dữ liệu từ nhiều sensor
node trong 10 giây và publish một JSON report lên
`gateway/aggregated_report`.

## Cài đặt

```bash
cd lab5.1/btvn/bai1
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
```

## Chạy

Mặc định chương trình kết nối tới MQTT broker tại `localhost:1883`:

Sau khi đã chạy lệnh `source .venv/bin/activate` ở bước trên:

```bash
python aggregator.py
```

Có thể truyền cấu hình bằng tham số:

```bash
python aggregator.py \
  --broker-host localhost \
  --broker-port 1883 \
  --gateway-id gateway-01
```

Hoặc dùng biến môi trường:

```bash
export MQTT_BROKER_HOST=localhost
export MQTT_BROKER_PORT=1883
export GATEWAY_ID=gateway-01
python aggregator.py
```

Nếu broker yêu cầu xác thực, dùng `--username`, `--password` hoặc đặt
`MQTT_USERNAME`, `MQTT_PASSWORD`.

## Kiểm thử bằng Mosquitto

Mở một terminal để theo dõi report:

```bash
mosquitto_sub -h localhost -t gateway/aggregated_report -v
```

Mở terminal khác và gửi dữ liệu từ nhiều node:

```bash
mosquitto_pub -h localhost -t sensor/node-01/data \
  -m '{"temperature":28.4,"humidity":67}'

mosquitto_pub -h localhost -t sensor/node-02/data \
  -m '{"temperature":29.1,"humidity":64}'
```

Sau tối đa 10 giây, report có dạng:

```json
{
  "gateway_id": "gateway-01",
  "generated_at": "2026-10-15T10:00:10+00:00",
  "message_count": 2,
  "sensors": {
    "node-01": [
      {
        "received_at": "2026-10-15T10:00:02+00:00",
        "topic": "sensor/node-01/data",
        "data": {"temperature": 28.4, "humidity": 67}
      }
    ],
    "node-02": [
      {
        "received_at": "2026-10-15T10:00:04+00:00",
        "topic": "sensor/node-02/data",
        "data": {"temperature": 29.1, "humidity": 64}
      }
    ]
  }
}
```

Payload không phải JSON hợp lệ sẽ bị bỏ qua. Nếu publish report thất bại hoặc
Gateway tạm thời mất kết nối, dữ liệu của chu kỳ đó được giữ lại để thử ở chu
kỳ sau. Nhấn `Ctrl+C` để dừng chương trình an toàn.
