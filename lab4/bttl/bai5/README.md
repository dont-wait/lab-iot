# Bài 5: Xử lý timeout và ngoại lệ khi gửi HTTP POST

## Mục tiêu

Đặt giới hạn thời gian chờ và xử lý timeout hoặc lỗi kết nối để chương trình không dừng đột ngột khi request thất bại.

## Chuẩn bị

1. Cài thư viện nếu chưa có: `python -m pip install requests`.
2. URL webhook đang được gán trực tiếp trong `main.py`; có thể thay bằng URL webhook riêng của bạn.

## Chạy bài

```bash
python main.py
```

Để quan sát lỗi kết nối, có thể tạm sửa URL trong `main.py` thành địa chỉ không phân giải được như `https://invalid.invalid`. Để quan sát timeout, có thể dùng endpoint thử nghiệm phản hồi chậm nếu được cung cấp trong lớp; timeout được đặt 3 giây.

## Kết quả mong đợi

Khi server phản hồi, chương trình in status code và nội dung phản hồi. Webhook.site hiện trả về `302` cùng nội dung mặc định của dịch vụ. Khi hết thời gian chờ hoặc không thể kết nối, chương trình in thông báo tương ứng và tiếp tục kết thúc bình thường. Lỗi URL được hiển thị bởi `requests.exceptions.RequestException`.
