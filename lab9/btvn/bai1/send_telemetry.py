#!/usr/bin/env python3
"""Gửi telemetry kiểm thử BTVN Lab 9 Bài 1; chỉ dùng thư viện chuẩn Python."""

import argparse
import json
import math
import os
import sys
import time
import urllib.error
import urllib.parse
import urllib.request


SCENARIO = [
    ({"temperature": 29.9}, "Normal: không tạo/cập nhật Alarm, không email"),
    ({"temperature": 30}, "WARNING: biên dưới, không email"),
    ({"temperature": 32}, "WARNING: cập nhật Alarm, không email"),
    ({"temperature": 35}, "WARNING: biên trên, không email"),
    ({"temperature": 35.1}, "CRITICAL: nâng mức Alarm, gửi email qua Updated"),
    ({"temperature": 40}, "CRITICAL: cập nhật Alarm, gửi thêm email"),
    ({"humidity": 60}, "Thiếu temperature: không xử lý Alarm"),
    ({"temperature": "40"}, "Sai kiểu temperature: không xử lý Alarm"),
]


def finite_number(value):
    number = float(value)
    if not math.isfinite(number):
        raise argparse.ArgumentTypeError("Giá trị phải là số hữu hạn.")
    return number


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", default="http://localhost:8080", help="URL ThingsBoard, hỗ trợ HTTP/HTTPS")
    parser.add_argument("--token", default=os.getenv("TB_DEVICE_TOKEN"), help="Device Access Token; hoặc biến TB_DEVICE_TOKEN")
    parser.add_argument("--temperature", type=finite_number, help="Gửi một nhiệt độ; bỏ qua để chạy cả kịch bản")
    parser.add_argument("--delay", type=finite_number, default=2.0, help="Khoảng nghỉ giữa các bản tin (giây)")
    parser.add_argument("--timeout", type=finite_number, default=10.0, help="Timeout HTTP (giây)")
    parser.add_argument("--dry-run", action="store_true", help="Xem payload và kết quả mong đợi, không kết nối mạng")
    args = parser.parse_args(argv)
    parsed = urllib.parse.urlsplit(args.url)
    if parsed.scheme not in ("http", "https") or not parsed.netloc or parsed.query or parsed.fragment or parsed.username is not None:
        parser.error("--url phải là URL HTTP/HTTPS, không chứa thông tin đăng nhập, query hoặc fragment.")
    if args.delay < 0 or args.timeout <= 0:
        parser.error("--delay phải >= 0 và --timeout phải > 0.")
    if not args.dry_run and (not args.token or args.token == "YOUR_DEVICE_ACCESS_TOKEN"):
        parser.error("Cần --token hợp lệ hoặc đặt biến môi trường TB_DEVICE_TOKEN.")

    if args.temperature is None:
        steps = SCENARIO
    else:
        temperature = args.temperature
        expected = "CRITICAL: Alarm + email" if temperature > 35 else (
            "WARNING: chỉ Alarm, không email" if temperature >= 30 else "Normal: không xử lý Alarm, không email"
        )
        steps = [({"temperature": temperature}, expected)]

    endpoint = args.url.rstrip("/") + "/api/v1/" + urllib.parse.quote(args.token or "DRY_RUN", safe="") + "/telemetry"
    for index, (payload, expected) in enumerate(steps, 1):
        print(f"[{index}/{len(steps)}] {json.dumps(payload, ensure_ascii=False)}")
        print(f"  Mong đợi: {expected}")
        if args.dry_run:
            continue
        request = urllib.request.Request(
            endpoint,
            data=json.dumps(payload, allow_nan=False).encode("utf-8"),
            headers={"Content-Type": "application/json"},
            method="POST",
        )
        try:
            with urllib.request.urlopen(request, timeout=args.timeout) as response:
                print(f"  HTTP {response.status}: đã tiếp nhận telemetry.")
        except urllib.error.HTTPError as error:
            print(f"  Lỗi HTTP {error.code}. Kiểm tra URL, token và dịch vụ ThingsBoard.", file=sys.stderr)
            return 1
        except (urllib.error.URLError, TimeoutError, OSError):
            print("  Không gửi được telemetry. Kiểm tra kết nối, địa chỉ và cổng ThingsBoard.", file=sys.stderr)
            return 1
        if index < len(steps):
            time.sleep(args.delay)

    if args.dry_run:
        print("Dry run hoàn tất; chưa gửi dữ liệu đến ThingsBoard.")
    else:
        print("Đã gửi xong. Xác minh kết quả ở Debug Events, Alarms và hộp thư.")
        print("HTTP 2xx không xác nhận Rule Chain hoặc SMTP đã xử lý thành công.")
    return 0


if __name__ == "__main__":
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\nĐã dừng gửi dữ liệu.")
        sys.exit(130)
