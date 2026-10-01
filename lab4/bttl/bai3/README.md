# Bài 3: Gửi HTTP POST bằng Python

## Mục tiêu

Tự động gửi dữ liệu cảm biến dạng JSON đến Webhook.site bằng thư viện `requests`, sau đó đọc mã trạng thái và nội dung phản hồi.

## Chuẩn bị

1. Cài Python 3.
2. Cài thư viện:

   ```bash
   python -m pip install requests
   ```

3. URL webhook đang được gán trực tiếp trong `main.py`; có thể thay bằng URL webhook riêng của bạn.

## Chạy bài

```bash
python main.py
```

Chương trình gửi dữ liệu cố định để kết quả dễ đối chiếu. Mỗi lần chạy, Webhook.site sẽ nhận một request POST với `Content-Type: application/json`.

## Kết quả mong đợi

Terminal hiển thị mã trạng thái HTTP cùng nội dung phản hồi từ Webhook.site. URL Webhook.site hiện trả về `302` và thông báo chưa cấu hình nội dung phản hồi mặc định; request vẫn xuất hiện trong lịch sử webhook. Trong trang webhook có thể mở request mới nhất để xem JSON đã nhận.
