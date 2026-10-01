# Bài 1 - Periodic Sender

Chương trình tạo ngẫu nhiên nhiệt độ từ 20-40 °C và độ ẩm từ 50-90%, sau đó gửi dữ liệu dạng JSON lên Webhook.site mỗi 10 giây.

## Cài đặt và chạy

```powershell
cd lab4/btvn/bai1
python -m pip install -r requirements.txt
python periodic_sender.py --url "https://webhook.site/URL-CUA-BAN"
```

Cũng có thể đặt URL bằng biến môi trường:

```powershell
$env:WEBHOOK_URL = "https://webhook.site/URL-CUA-BAN"
python periodic_sender.py
```

Nhấn `Ctrl+C` để dừng chương trình an toàn.

