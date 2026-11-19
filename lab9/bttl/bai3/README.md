# Bài 3: Create Alarm Node

## Luồng

```
Check Temperature > 35 ─[True]──► Create Alarm HighTemperatureAlert
                        └[False]─► Log NORMAL
```

## Cấu hình node Create Alarm

| Thuộc tính | Giá trị |
|---|---|
| Alarm type | `HighTemperatureAlert` |
| Severity | `CRITICAL` |
| Script ngôn ngữ | TBEL |
| Propagate | tắt |

Alarm details (cấu trúc tiêu đề hiển thị):

```
return {title: 'Canh bao nhiet do cao: ' + msg.temperature + 'C', temperature: msg.temperature};
```

Nối node vào nhánh **True** của `Check Temperature > 35` (bài 2).

## Kiểm thử

```bash
TOKEN=<access token của sensor1>
curl -X POST http://localhost:8080/api/v1/$TOKEN/telemetry -H "Content-Type: application/json" -d '{"temperature": 50}'   # tạo alarm
curl -X POST http://localhost:8080/api/v1/$TOKEN/telemetry -H "Content-Type: application/json" -d '{"temperature": 20}'   # không tạo alarm
```

Xem alarm: Entities → Devices → `sensor1` → tab **Alarms** (hoặc menu **Alarms** bên trái).

Kết quả đã chạy thử:

| Type | Severity | Status | Details |
|---|---|---|---|
| HighTemperatureAlert | CRITICAL | ACTIVE_UNACK | `{"title": "Canh bao nhiet do cao: 50C", "temperature": 50}` |

Nếu nhiệt độ tiếp tục vượt ngưỡng khi alarm còn active thì node cập nhật alarm cũ (relation `Updated`) chứ không tạo thêm alarm mới.

File: `Smart_Environment_Alert_bai3.json`
