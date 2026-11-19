# Bài 4: Send Email Node cảnh báo

## Luồng

```
Check Temperature > 35 ─[True]─► Add Temp To Metadata ─► Create Alarm HighTemperatureAlert ─[Created/Updated]─► To Email ─► Send Email
                        └[False]─► Log NORMAL
```

| Node | Vai trò |
|---|---|
| Add Temp To Metadata (Transform) | Chép `msg.temperature` vào metadata, vì sau node Create Alarm thì `msg` biến thành dữ liệu alarm |
| Create Alarm HighTemperatureAlert | Tạo alarm `HighTemperatureAlert`, mức CRITICAL |
| To Email | Soạn nội dung email từ template |
| Send Email | Gửi qua SMTP |

## SMTP (server test local bằng Mailpit)

```bash
docker network create tbnet
docker network connect tbnet mytb
docker run -d --name mailpit --network tbnet -p 8025:8025 axllent/mailpit
```

Cấu hình node **Send Email**: bỏ chọn *Use system SMTP settings*, `SMTP host = mailpit`, `port = 1025`, không dùng TLS và không cần username/password.
Hộp thư test xem tại http://localhost:8025.

Tài khoản tenant không có quyền sửa Mail settings của hệ thống (cần sysadmin), nên SMTP được cấu hình ngay trong node.

## Template (node To Email)

- From: `alert@smartenv.local`, To: `admin@smartenv.local`
- Subject: `[CANH BAO] ${deviceName} vuot nguong nhiet do`
- Body: `Thiet bi ${deviceName} ghi nhan nhiet do ${temperature}C, vuot nguong an toan 35C.`

Cú pháp biến của ThingsBoard: `${key}` lấy từ **metadata** (ở đây `deviceName`, `temperature`), còn `$[key]` lấy từ **msg**. Đề bài viết `${metadata.deviceName}` và `${msg.temperature}`, trong ThingsBoard tương đương `${deviceName}` và `$[temperature]`.

## Kiểm thử

```bash
curl -X POST http://localhost:8080/api/v1/<TOKEN>/telemetry -H "Content-Type: application/json" -d '{"temperature": 43}'
```

Kết quả: mailpit nhận email có subject `[CANH BAO] sensor1 vuot nguong nhiet do` và nội dung `Thiet bi sensor1 ghi nhan nhiet do 43C, vuot nguong an toan 35C.` Đồng thời có alarm tại Device → tab Alarms.

File: `Smart_Environment_Alert_bai4.json`
