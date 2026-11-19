#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Lab 9 - Bài 3 (BTVN): Xây dựng logic tự động phục hồi và xóa cảnh báo (Clear Alarm)
Script mô phỏng chu trình khép kín:
  Bình thường -> Quá nhiệt (>= 40°C: Create Alarm) -> Hạ nhiệt vùng đệm (28..40°C) -> Phục hồi an toàn (< 28°C: Clear Alarm & Gửi Email xác nhận)
Sử dụng 100% thư viện chuẩn (Standard Library) của Python - KHÔNG CẦN CÀI THÊM THƯ VIỆN NGOÀI.
"""

import sys
import time
import json
import argparse
import urllib.request
import urllib.error

DEFAULT_HOST = "localhost"
DEFAULT_PORT = 8080
DEFAULT_TOKEN = "YOUR_DEVICE_ACCESS_TOKEN"

def print_banner():
    banner = """
========================================================================
   THINGSBOARD - TỰ ĐỘNG PHỤC HỒI & XÓA CẢNH BÁO (CLEAR ALARM)
   Lab 9 - Bài 3 (BTVN): Kiểm thử vòng đời Alarm & Email thông báo phụ bản
   Thư viện: Python Standard Library (urllib.request) - Zero Dependencies
========================================================================
"""
    print(banner)

def send_telemetry_http(host, port, token, data):
    url = f"http://{host}:{port}/api/v1/{token}/telemetry"
    json_bytes = json.dumps(data).encode("utf-8")
    
    headers = {
        "Content-Type": "application/json",
        "Content-Length": str(len(json_bytes)),
        "User-Agent": "ThingsBoard-Recovery-Client/1.0"
    }
    
    req = urllib.request.Request(url, data=json_bytes, headers=headers, method="POST")
    start_time = time.time()
    
    try:
        with urllib.request.urlopen(req, timeout=10) as response:
            latency = (time.time() - start_time) * 1000
            status_code = response.getcode()
            return True, status_code, latency, None
    except urllib.error.HTTPError as e:
        latency = (time.time() - start_time) * 1000
        return False, e.code, latency, f"HTTP Error {e.code}: {e.reason}"
    except urllib.error.URLError as e:
        latency = (time.time() - start_time) * 1000
        return False, 0, latency, f"URL Error: {e.reason}"
    except Exception as e:
        latency = (time.time() - start_time) * 1000
        return False, 0, latency, str(e)

def run_recovery_lifecycle(host, port, token, delay=2.5):
    """
    Kịch bản kiểm thử toàn diện vòng đời cảnh báo:
    - Giai đoạn 1: Nhiệt độ bình thường (25.0°C) -> Hệ thống ổn định
    - Giai đoạn 2: Tăng vọt lên 42.0°C (>= 40°C) -> Kích hoạt Create Alarm + Gửi Email cảnh báo sự cố
    - Giai đoạn 3: Hạ nhiệt về 32.0°C (28°C <= T < 40°C) -> Vùng trễ (Dead-band/Hysteresis), Alarm vẫn giữ nguyên
    - Giai đoạn 4: Nhiệt độ giảm về 26.5°C (< 28°C) -> Kích hoạt Clear Alarm + Gửi Email xác nhận ổn định
    - Giai đoạn 5: Tiếp tục ở mức an toàn 25.0°C (< 28°C) -> Clear Alarm relation: False (Không spam email)
    """
    lifecycle_steps = [
        {
            "phase": "Pha 1: Ổn định",
            "desc": "Nhiệt độ phòng ban đầu bình thường",
            "payload": {"temperature": 25.5, "humidity": 60.0},
            "expected_state": "BÌNH THƯỜNG",
            "action_expected": "Lưu timeseries, không kích hoạt alarm"
        },
        {
            "phase": "Pha 2: Quá nhiệt",
            "desc": "Nhiệt độ bùng phát vượt ngưỡng cảnh báo (>= 40°C)",
            "payload": {"temperature": 41.5, "humidity": 42.0},
            "expected_state": "BÁO ĐỘNG (ALARM ACTIVE)",
            "action_expected": "[!] CREATE ALARM + GỬI EMAIL CẢNH BÁO SỰ CỐ"
        },
        {
            "phase": "Pha 3: Giảm nhiệt",
            "desc": "Bật máy lạnh, nhiệt độ hạ về 32°C (Vùng đệm Hysteresis)",
            "payload": {"temperature": 32.0, "humidity": 50.0},
            "expected_state": "VẪN GIỮ ALARM ACTIVE",
            "action_expected": "Chưa an toàn (< 28°C), không xóa alarm tránh chập chờn (anti-flapping)"
        },
        {
            "phase": "Pha 4: Phục hồi",
            "desc": "Nhiệt độ giảm sâu về 26.5°C (Dưới ngưỡng an toàn < 28°C)",
            "payload": {"temperature": 26.5, "humidity": 58.0},
            "expected_state": "TỰ ĐỘNG XÓA (CLEARED)",
            "action_expected": "[+] KÍCH HOẠT CLEAR ALARM + GỬI EMAIL XÁC NHẬN ỔN ĐỊNH"
        },
        {
            "phase": "Pha 5: Duy trì",
            "desc": "Nhiệt độ tiếp tục giữ mức ổn định 25.0°C",
            "payload": {"temperature": 25.0, "humidity": 61.0},
            "expected_state": "CLEARED / ỔN ĐỊNH",
            "action_expected": "Clear Alarm trả về False (không còn active alarm) -> KHÔNG SPAM EMAIL"
        }
    ]

    print(f"[*] Bắt đầu kịch bản kiểm thử Tự động Phục hồi (Clear Alarm) tại: http://{host}:{port}")
    print(f"[*] Device Access Token: {token}")
    print(f"[*] Khoảng thời gian giữa các pha: {delay}s\n")
    print("=" * 88)
    print(f"{'Pha':<18} | {'Nhiệt độ':<10} | {'Trạng thái kỳ vọng':<22} | {'Hành vi Rule Engine':<30}")
    print("=" * 88)

    for idx, step in enumerate(lifecycle_steps, start=1):
        payload = step["payload"]
        temp = payload["temperature"]
        phase = step["phase"]
        expected = step["expected_state"]
        action = step["action_expected"]

        success, code, latency, err = send_telemetry_http(host, port, token, payload)
        
        status_info = f"{code} OK" if success else f"ERR: {err}"
        print(f"{phase:<18} | {temp:>5.1f} °C    | {expected:<22} | {action}")
        print(f"   -> Dữ liệu gửi: {json.dumps(payload)} | HTTP Phản hồi: {status_info} ({latency:.1f}ms)")
        print(f"   -> Mô tả: {step['desc']}")
        
        if "CREATE ALARM" in action:
            print("   ------------------------------------------------------------------------")
            print("   >>> [BƯỚC 2: KIỂM TRA BÁO ĐỘNG]")
            print("       - Tab Alarms: Alarm 'High Temperature Alarm' chuyển sang ACTIVE_UNACK")
            print("       - Email: Nhận thư cảnh báo quá nhiệt [CẢNH BÁO KHẨN CẤP]")
            print("   ------------------------------------------------------------------------")
        elif "CLEAR ALARM" in action:
            print("   ------------------------------------------------------------------------")
            print("   >>> [BƯỚC 4: KIỂM TRA PHỤC HỒI]")
            print("       - Tab Alarms: Alarm chuyển sang trạng thái CLEARED_UNACK (Đã xóa)")
            print("       - Email: Nhận thư phụ bản xác nhận [PHỤC HỒI] Hệ thống trở lại ổn định")
            print("   ------------------------------------------------------------------------")
            
        print()
        time.sleep(delay)

    print("=" * 88)
    print("[*] Kết thúc kịch bản kiểm thử tự động phục hồi và xóa cảnh báo thành công!")

def main():
    print_banner()
    parser = argparse.ArgumentParser(description="Kiểm thử tự động phục hồi và Clear Alarm trên ThingsBoard")
    parser.add_argument("--host", default=DEFAULT_HOST, help=f"Địa chỉ ThingsBoard server (mặc định: {DEFAULT_HOST})")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help=f"Cổng HTTP ThingsBoard (mặc định: {DEFAULT_PORT})")
    parser.add_argument("--token", default=DEFAULT_TOKEN, help="Device Access Token trên ThingsBoard")
    parser.add_argument("--delay", type=float, default=2.5, help="Độ trễ giữa các pha (giây)")
    parser.add_argument("--temp", type=float, default=None, help="Gửi một nhiệt độ tùy chọn duy nhất")
    
    args = parser.parse_args()
    
    if args.token == DEFAULT_TOKEN:
        print("[LƯU Ý QUAN TRỌNG]")
        print("Bạn đang dùng Access Token mặc định ('YOUR_DEVICE_ACCESS_TOKEN').")
        print("Để kết nối với thiết bị thực tế trên ThingsBoard, vui lòng chạy lệnh:")
        print("   python simulate_recovery.py --token <DEVICE_ACCESS_TOKEN_CUA_BAN>\n")

    if args.temp is not None:
        payload = {"temperature": args.temp}
        print(f"[*] Gửi giá trị đơn lẻ: {payload}")
        success, code, latency, err = send_telemetry_http(args.host, args.port, args.token, payload)
        if success:
            print(f"[+] Phản hồi thành công: {code} OK ({latency:.1f}ms)")
        else:
            print(f"[-] Gửi thất bại: {err}")
    else:
        run_recovery_lifecycle(args.host, args.port, args.token, args.delay)

if __name__ == "__main__":
    main()
