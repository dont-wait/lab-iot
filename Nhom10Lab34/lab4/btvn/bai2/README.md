# Bài 2 - Local Flask Test Server

`server.py` cung cấp route `POST /api/sensor`; `client.py` tạo và gửi một mẫu dữ liệu cảm biến tới server.

## Cài đặt

```powershell
cd lab4/btvn/bai2
python -m pip install -r requirements.txt
```

## Chạy

Mở terminal thứ nhất để chạy server:

```powershell
python server.py
```

Mở terminal thứ hai, vẫn tại thư mục `bai2`, để gửi dữ liệu:

```powershell
python client.py
```

Server lắng nghe tại `http://127.0.0.1:5000/api/sensor`. Khi nhận JSON hợp lệ, server in dữ liệu ra console và trả về:

```json
{
  "status": "success",
  "message": "Data received successfully"
}
```
