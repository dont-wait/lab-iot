# Bài 4: Gửi HTTP POST kèm API Key

## Mục tiêu

Đính kèm một HTTP header tùy chỉnh trong request POST và quan sát header trên Webhook.site. Webhook.site chỉ ghi nhận header; bài thực hành này không bật xác thực hay từ chối request thiếu key.

## Chuẩn bị

1. Cài thư viện nếu chưa có: `python -m pip install requests`.
2. Tạo webhook tại <https://webhook.site> và sao chép URL.
3. Thay `YOUR-WEBHOOK-URL` trong `main.py` bằng URL đó.

## Chạy bài

```bash
python main.py
```

Mã gửi header `X-API-Key: my_secret_key_123`. Mở request vừa nhận trên Webhook.site và xem mục Headers để xác nhận header tùy chỉnh. Đây là key giả để học tập, không dùng bí mật thật trong mã nguồn.

## Kết quả mong đợi

Terminal hiển thị status code và response body. Chi tiết request trên Webhook.site có JSON cảm biến và header `X-API-Key`.
