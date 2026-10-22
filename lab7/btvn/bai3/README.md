# Bài 3 - Phân quyền truy cập và chia sẻ Dashboard đa người dùng

Dashboard giám sát IoT chạy trên **Node-RED Dashboard**, có nhiều tài khoản:
`admin` được xem và điều khiển thiết bị, `viewer` **chỉ được xem** thông số.
Mọi thao tác điều khiển của `viewer` bị vô hiệu hóa ở giao diện **và** bị máy chủ
từ chối bằng HTTP `403`.

## 1. Tìm hiểu User Management / Share Dashboard trên nền tảng

Bài 3 dùng **Node-RED Dashboard** (cùng nền tảng với `lab7/bttl/bai5`) - một nền
tảng low-code/no-code để dựng dashboard bằng cách kéo thả widget.

Kết quả tìm hiểu:

| Nền tảng | Cơ chế có sẵn | Giới hạn |
|---|---|---|
| Node-RED Editor | `adminAuth` trong `settings.js` | Chỉ bảo vệ trình soạn flow (`/`), **không** bảo vệ dashboard `/ui` |
| Node-RED Dashboard 3.x | Không có ACL theo từng người dùng | Mọi người vào được `/ui` đều thấy và bấm được mọi widget |
| Blynk IoT | User Management / chia sẻ thiết bị - dashboard | Là nền tảng của bài trên lớp (`lab7/bttl/bai3`), không dùng cho bài này |

Vì Node-RED Dashboard không có sẵn phân quyền theo người dùng, bài làm bổ sung
một lớp **RBAC (Role-Based Access Control)**:

- Tài khoản lưu trong `users.json`, mật khẩu hash bằng `scrypt`, không lưu mật khẩu rõ.
- Đăng nhập bằng cookie phiên `HttpOnly`, `SameSite=Strict`, ký HMAC-SHA256.
- Phân quyền theo vai trò; kiểm tra lại ở máy chủ cho **mọi** request điều khiển.
- Giao diện chỉ ẩn/disable nút theo quyền, không thay thế kiểm tra phía máy chủ.
- Ghi audit log cho đăng nhập, bị từ chối và điều khiển thiết bị.
- Lệnh hợp lệ được đưa vào hàng đợi; flow Node-RED publish lên MQTT.

### Ma trận phân quyền

| Quyền | admin | viewer | gateway |
|---|:---:|:---:|:---:|
| `device:view` - xem thông số | ✅ | ✅ | ❌ |
| `dashboard:view` - mở `/ui` | ✅ | ✅ | ❌ |
| `device:control` - bật/tắt/đảo đèn | ✅ | ❌ | ❌ |
| `sensor:ingest` - ghi dữ liệu cảm biến | ✅ | ❌ | ✅ |
| `users:read` - xem danh sách tài khoản | ✅ | ❌ | ❌ |
| `audit:read` - xem audit log | ✅ | ❌ | ❌ |

## 2. Tài khoản demo

| Tài khoản | Mật khẩu | Vai trò | Dùng để |
|---|---|---|---|
| `admin` | `Admin@123` | admin | Kiểm tra quyền điều khiển và quản lý người dùng |
| `viewer` | `Viewer@123` | viewer | **Tài khoản phụ read-only** - kiểm tra nút bị vô hiệu hóa |
| `gateway` | `Gateway@123` | gateway | Thiết bị gửi dữ liệu cảm biến qua `/api/sensor` |

Tạo thêm tài khoản:

```bash
node tools/add_user.js --username viewer2 --name "Người xem 2" \
  --role viewer --password 'Viewer2@123'
```

## 3. Cài đặt và chạy

```bash
cd lab7/btvn/bai3
npm install
npm start
```

Mở:

- Dashboard: <http://127.0.0.1:1880/ui>
- Đăng nhập: <http://127.0.0.1:1880/login>
- Editor Node-RED (chỉ admin editor): <http://127.0.0.1:1880/>

Đổi cổng bằng biến môi trường `PORT` (ví dụ `PORT=1881 npm start`).

> `server.js` nhúng Node-RED vào một Express app để gắn `rbac.middleware`
> **trước** `RED.httpNode`. Lý do: từ Node-RED 4/5, `httpNodeMiddleware` chỉ áp
> dụng cho từng node `http-in`, không còn là middleware toàn cục cho `/ui`.

## 4. Cấu trúc

| File | Nội dung |
|---|---|
| `rbac.js` | Toàn bộ xác thực, phiên, phân quyền, API, audit log |
| `server.js` | Khởi động Express + Node-RED, gắn middleware RBAC |
| `settings.js` | Cấu hình Node-RED, `adminAuth`, `functionGlobalContext.rbac` |
| `users.json` | Danh sách tài khoản (hash scrypt) |
| `ui/sensor_panel.html` | Widget hiển thị nhiệt độ, độ ẩm, biểu đồ |
| `ui/control_panel.html` | Widget điều khiển, khóa read-only, bảng quản lý user |
| `flows.json` | Flow Dashboard + hàng đợi lệnh MQTT (sinh bởi `tools/build_flows.js`) |
| `tests/verify_rbac.js` | Kiểm thử API/phân quyền |
| `tests/e2e_dashboard.js` | E2E đăng nhập thật + render DOM bằng jsdom |

Sau khi sửa `ui/*.html` hoặc `ui/dashboard.css`:

```bash
npm run build:flows
```

## 5. Kiểm thử

```bash
npm test          # 52 PASS
npm run test:e2e  # 26 PASS

# Voi server that dang chay:
BASE_URL=http://127.0.0.1:1880 bash tests/smoke_real_server.sh  # 12 PASS
```

Kết quả được ghi vào:

- `evidence/verify_rbac.log` - kiểm thử API, cookie, rate limit, audit.
- `evidence/e2e.log` - đăng nhập `viewer`, render Dashboard thật, xác thực nút
  `disabled`, bấm nút không gửi request, gọi thẳng API nhận `403`; sau đó
  đăng nhập `admin` và điều khiển thành công.
- `evidence/real_server.log` - smoke test trên server Node-RED thật: đăng nhập,
  mở `/ui`, gọi API, kiểm tra 403/200 và không lộ password hash.

Kịch bản chính của bài:

1. Chưa đăng nhập: `/ui` chuyển về `/login`, `/api/state` trả `401`.
2. Đăng nhập `viewer`: xem được `/ui` và `/api/state`.
3. Trên giao diện: cả 3 nút điều khiển có `disabled = true`, hộp
   "Chế độ chỉ xem" hiện ra, bảng quản lý người dùng bị ẩn.
4. Bấm nút: không có request nào được gửi.
5. Gọi thẳng `POST /api/device/control`: `403` với
   `requiredPermission = device:control`.
6. Đăng nhập `admin`: nút bật, điều khiển trả `200`, trạng thái đèn đổi và có
   audit log.

## 6. Đường đi của lệnh điều khiển

```
viewer bấm nút
  -> UI đã disable, không gửi request
  -> nếu cố gọi API: rbac.middleware kiểm tra device:control -> 403

admin bấm nút
  -> POST /api/device/control
  -> rbac.js kiểm tra quyền, đổi trạng thái, đưa lệnh vào commandQueue
  -> flow "Hàng đợi lệnh MQTT" (mỗi giây) gọi rbac.drainCommands()
  -> MQTT out publish tới home/living_room/control
```

Broker mặc định là `broker.hivemq.com:1883`, đổi bằng `MQTT_BROKER` /
`MQTT_PORT` khi build flow.

## 7. Ghi chú bảo mật

- `viewer` không có trong `adminAuth` nên không mở được editor Node-RED.
- Cookie phiên có `HttpOnly`, `SameSite=Strict`, hết hạn sau 2 giờ.
- Sai mật khẩu 5 lần trong 1 phút sẽ bị khóa 60 giây (`429` + `Retry-After`).
- `password_hash` không bao giờ trả về qua API.
- `.rbac-secret`, `node_modules/`, `.node-red/`, `evidence/audit.jsonl` nằm
  trong `.gitignore`; khi triển khai thật nên đặt `RBAC_SESSION_SECRET` và
  `RBAC_SECURE_COOKIE=1`.
