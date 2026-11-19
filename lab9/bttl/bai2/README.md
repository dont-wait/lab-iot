# Bài 2: Filter Node theo ngưỡng nhiệt độ

## Các bước thực hiện

1. Mở rule chain `Smart_Environment_Alert` (hoặc import `Smart_Environment_Alert_bai2.json`).
2. Kéo **Filter → script** vào, đặt tên `Check Temperature > 35`, nhập:

```javascript
return msg.temperature > 35;
```

   (`msg` là payload JSON, ví dụ `{"temperature": 38, "humidity": 60}`.)
3. Nối `Message Type Switch` →(`Post telemetry`)→ `Check Temperature > 35`.
4. Định tuyến:
   - Nhánh **True** → node **Log** `Log ALERT` (nhánh cảnh báo; thực tế có thể thay bằng Create alarm / Send email).
   - Nhánh **False** → node **Log** `Log NORMAL` (dữ liệu bình thường).
5. Lưu chain; bật **Debug mode** cho node script để xem input/output.

## Kiểm thử

Gửi telemetry bằng thiết bị mô phỏng:

```bash
# thay $TOKEN bằng Access Token của device
curl -X POST http://localhost:8080/api/v1/$TOKEN/telemetry \
  -H "Content-Type: application/json" -d '{"temperature": 40}'   # -> True / ALERT
curl -X POST http://localhost:8080/api/v1/$TOKEN/telemetry \
  -H "Content-Type: application/json" -d '{"temperature": 28}'   # -> False / NORMAL
```

| temperature | Kết quả script | Nhánh | Log |
|---|---|---|---|
| 40 | true | True | ALERT: temperature = 40 vuot nguong 35 |
| 35 | false | False | NORMAL: temperature = 35 |
| 28 | false | False | NORMAL: temperature = 28 |

Xem kết quả tại Rule chain → tab **Events** của node, hoặc log server (`/var/log/thingsboard/thingsboard.log`).

## Sơ đồ

```
Message Type Switch ─[Post telemetry]─► Check Temperature > 35 ─[True]──► Log ALERT
                                                               └─[False]─► Log NORMAL
                    └─[Post telemetry]─► Save Timeseries
```

File: `Smart_Environment_Alert_bai2.json`
