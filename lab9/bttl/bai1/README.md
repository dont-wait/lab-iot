# Bài 1: Khảo sát Rule Chain và các Node cơ bản (ThingsBoard)

## Các bước thực hiện

1. Đăng nhập ThingsBoard (demo.thingsboard.io hoặc local `http://localhost:8080`).
2. Menu trái → **Rule chains** → nút **+** → **Create new rule chain**.
   - Name: `Smart_Environment_Alert` → **Add**.
   - (Hoặc **Import rule chain** và chọn file `Smart_Environment_Alert_bai1.json`.)
3. Mở rule chain mới, khảo sát bảng node bên trái (chụp ảnh từng nhóm):

| Nhóm | Vai trò | Node tiêu biểu |
|---|---|---|
| **Filter** (Lọc) | Kiểm tra điều kiện, rẽ nhánh True/False hoặc theo loại bản tin | Script, Message Type Switch, Check existence fields, Originator type switch |
| **Enrichment** | Bổ sung dữ liệu vào message | Originator attributes, Customer attributes |
| **Transformation** | Biến đổi nội dung/metadata/originator | Script (Transform), Change originator, To email |
| **Action** | Thực thi hành động | Save Timeseries, Save Attributes, Create alarm, Clear alarm, Log, RPC call request |
| **External** | Tích hợp hệ thống ngoài | REST API call, MQTT, Send email, Kafka, AWS SNS, Send SMS |
| **Flow** | Điều phối giữa các chain | Rule Chain, Output |

4. Kéo node **Message Type Switch** và **Save Timeseries** vào, nối bằng nhánh `Post telemetry`. Lưu (dấu ✓ xanh).
5. Nối từ Root Rule Chain sang chain mới:
   - Rule chains → **Root Rule Chain** → mở.
   - Kéo node **Flow → Rule Chain**, chọn `Smart_Environment_Alert`.
   - Nối `Message Type Switch` →(nhánh `Post telemetry`)→ node Rule Chain vừa thêm. Lưu.

## Sơ đồ luồng

```
Device ──► Root Rule Chain ──[Post telemetry]──► Smart_Environment_Alert
                                                   Message Type Switch ─► Save Timeseries
```

## Giải thích Event-driven

Mỗi bản tin (telemetry, attribute, RPC, connect/disconnect...) là một **sự kiện**; Rule Engine đưa nó vào chain, qua từng node, và node tiếp theo được chọn theo **relation type** (Success, True, False, Post telemetry, Failure...).

File: `Smart_Environment_Alert_bai1.json`
