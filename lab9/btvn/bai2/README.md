# BTVN Bài 2: Cảnh báo qua Telegram Bot (REST API Call)

## Luồng

```
Check Temperature > 35 ─[True]─► Build Telegram Message ─[Success]─► Send Telegram (REST API Call)
                                        │                                   │
                                        └──────────[Failure]──► Log Telegram Error ◄─[Failure]┘
                        └[False]─► Log NORMAL
```

| Node | Vai trò |
|---|---|
| Build Telegram Message (Transform) | Tạo body JSON `{chat_id, text}`, text gồm tên thiết bị, nhiệt độ và thời gian (GMT+7, tính từ `metadata.ts`) |
| Send Telegram (REST API Call) | `POST https://api.telegram.org/bot<BOT_TOKEN>/sendMessage`, header `Content-Type: application/json` |
| Log Telegram Error | Ghi log khi build hoặc gọi API lỗi |

## Chuẩn bị Telegram

1. Chat với **@BotFather** → `/newbot` → lấy **BOT_TOKEN**.
2. Tạo nhóm chat, thêm bot vào nhóm và gửi một tin nhắn bất kỳ.
3. Lấy chat id: mở `https://api.telegram.org/bot<BOT_TOKEN>/getUpdates`, đọc `message.chat.id`. Với nhóm, id là số âm, ví dụ `-1001234567890`.

## Cấu hình

Import `Smart_Environment_Alert_telegram.json`, rồi thay 2 placeholder:
- Node **Send Telegram**: `<BOT_TOKEN>` trong URL.
- Node **Build Telegram Message**: `<CHAT_ID>` trong script.

Nội dung tin nhắn:

```
⚠️ CANH BAO NHIET DO CAO
Thiet bi: sensor1
Nhiet do: 49 C (nguong 35 C)
Thoi gian: 2026-11-19 20:52:06 (GMT+7)
```

## Kiểm thử

```bash
curl -X POST http://localhost:8080/api/v1/<DEVICE_TOKEN>/telemetry -H "Content-Type: application/json" -d '{"temperature": 49}'
```

## Đã kiểm tra thế nào

Chưa có bot thật nên mình thay URL bằng một server giả lập Telegram trong cùng mạng Docker (`http://tgmock:9999/bot123/sendMessage`). ThingsBoard đã gửi đúng request:

```
POST /bot123/sendMessage
{"chat_id":"<CHAT_ID>","text":"⚠️ CANH BAO NHIET DO CAO\nThiet bi: sensor1\nNhiet do: 49 C (nguong 35 C)\nThoi gian: 2026-11-19 20:52:06 (GMT+7)"}
```

Chưa thử gọi Telegram thật; cần bạn điền token và chat id.

## Lưu ý

- Token là bí mật, không commit token thật lên git.
- Container ThingsBoard phải ra được Internet để gọi `api.telegram.org`.
