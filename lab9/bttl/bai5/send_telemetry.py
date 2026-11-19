#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Lab 9 - Bài 5: Kiểm thử toàn diện chuỗi Rule Chain (End-to-End Testing)
Script gửi dữ liệu Telemetry lên ThingsBoard qua giao thức HTTP REST API.
Sử dụng 100% thư viện chuẩn (Standard Library) của Python - KHÔNG CẦN CÀI THÊM THƯ VIỆN.
"""

import sys
import time
import json
import argparse
import urllib.request
import urllib.error

# Cấu hình mặc định
DEFAULT_HOST = "localhost"
DEFAULT_PORT = 8080
DEFAULT_TOKEN = "YOUR_DEVICE_ACCESS_TOKEN"  # Thay bằng Access Token của Device trên ThingsBoard

def print_banner():
    banner = """
========================================================================
   THINGSBOARD RULE ENGINE - END-TO-END TELEMETRY TEST SCRIPT (HTTP)
   Bài 5: Kiểm thử toàn diện chuỗi Rule Chain với chế độ Debug Mode
   Thư viện: Python Standard Library (urllib.request) - Zero Dependencies
========================================================================
"""
    print(banner)

def send_telemetry_http(host, port, token, data):
    """
    Gửi bản tin JSON telemetry lên ThingsBoard qua HTTP POST
    Endpoint: http://<host>:<port>/api/v1/<token>/telemetry
    """
    url = f"http://{host}:{port}/api/v1/{token}/telemetry"
    json_bytes = json.dumps(data).encode("utf-8")
    
    headers = {
        "Content-Type": "application/json",
        "Content-Length": str(len(json_bytes)),
        "User-Agent": "ThingsBoard-Test-Client/1.0"
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
        err_msg = f"HTTP Error {e.code}: {e.reason}"
        return False, e.code, latency, err_msg
    except urllib.error.URLError as e:
        latency = (time.time() - start_time) * 1000
        err_msg = f"URL Error: {e.reason}"
        return False, 0, latency, err_msg
    except Exception as e:
        latency = (time.time() - start_time) * 1000
        return False, 0, latency, str(e)

def run_scenario(host, port, token, delay=2.0):
    """
    Kịch bản kiểm thử toàn diện:
    1. Gửi các giá trị bình thường (25°C, 26.5°C, 27°C): Message đi qua Filter -> False -> Không cảnh báo
    2. Tăng đột ngột lên 40.5°C: Message đi qua Filter -> True -> Create Alarm -> To Email -> Send Email
    3. Tiếp tục quan sát trên Rule Engine Debug log và Inbox
    """
    scenario_steps = [
        {"desc": "Bình thường 1 (Baseline)", "payload": {"temperature": 25.0, "humidity": 60.5}},
        {"desc": "Bình thường 2 (Ổn định)",  "payload": {"temperature": 26.2, "humidity": 59.8}},
        {"desc": "Bình thường 3 (Cận ngưỡng an toàn)", "payload": {"temperature": 27.5, "humidity": 61.0}},
        {"desc": "ĐỘT BIẾN NHIỆT ĐỘ VƯỢT NGƯỠNG (40°C)", "payload": {"temperature": 40.5, "humidity": 45.0}},
        {"desc": "Nhiệt độ duy trì mức báo động", "payload": {"temperature": 41.2, "humidity": 43.5}},
        {"desc": "Hạ nhiệt nhẹ về mức cao", "payload": {"temperature": 39.8, "humidity": 48.0}},
    ]
    
    print(f"[*] Bắt đầu kịch bản kiểm thử End-to-End tới: http://{host}:{port}")
    print(f"[*] Device Access Token: {token}")
    print(f"[*] Khoảng nghỉ giữa các bản tin: {delay}s\n")
    print("-" * 80)
    print(f"{'Bước':<6} | {'Mô tả':<35} | {'Dữ liệu':<25} | {'Kết quả':<10}")
    print("-" * 80)
    
    for idx, step in enumerate(scenario_steps, start=1):
        payload = step["payload"]
        desc = step["desc"]
        temp = payload.get("temperature")
        
        is_alarm_trigger = temp >= 40.0
        flag = " [!] KÍCH HOẠT ALARM" if is_alarm_trigger else " [OK] Bình thường"
        
        success, code, latency, err = send_telemetry_http(host, port, token, payload)
        
        payload_str = json.dumps(payload)
        if success:
            res_str = f"200 OK ({latency:.1f}ms){flag}"
        else:
            res_str = f"LỖI: {err}"
            
        print(f"#{idx:<5} | {desc:<35} | {payload_str:<25} | {res_str}")
        
        if is_alarm_trigger:
            print("  >>>>> ĐÃ GỬI BẢN TIN NHIỆT ĐỘ >= 40°C!")
            print("  >>>>> Hãy kiểm tra ngay:")
            print("        1. Rule Engine Debug Tab (Filter Script: True, Create Alarm: Created)")
            print("        2. Tab 'Alarms' trên Thiết bị (Trạng thái: ACTIVE_UNACK)")
            print("        3. Hộp thư đến (Inbox) của Email người nhận")
            
        time.sleep(delay)
        
    print("-" * 80)
    print("[*] Hoàn thành kịch bản kiểm thử!")

def run_single(host, port, token, temp, hum=None):
    """Gửi một giá trị duy nhất"""
    payload = {"temperature": temp}
    if hum is not None:
        payload["humidity"] = hum
        
    print(f"[*] Gửi bản tin đơn: {payload} tới http://{host}:{port}/api/v1/{token}/telemetry")
    success, code, latency, err = send_telemetry_http(host, port, token, payload)
    if success:
        print(f"[SUCCESS] Mã phản hồi: {code} OK, Độ trễ: {latency:.1f}ms")
        if temp >= 40:
            print("==> Giá trị nhiệt độ >= 40°C! Rule Chain sẽ kích hoạt Alarm và gửi Email.")
        else:
            print("==> Giá trị nhiệt độ bình thường (< 40°C). Message được lưu timeseries, không kích hoạt Alarm.")
    else:
        print(f"[FAILED] Không thể gửi bản tin. Lý do: {err}")

def main():
    print_banner()
    parser = argparse.ArgumentParser(description="Gửi dữ liệu kiểm thử Rule Chain ThingsBoard (Python Standard Library)")
    parser.add_argument("--host", default=DEFAULT_HOST, help=f"Địa chỉ ThingsBoard server (mặc định: {DEFAULT_HOST})")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help=f"Cổng HTTP ThingsBoard (mặc định: {DEFAULT_PORT})")
    parser.add_argument("--token", default=DEFAULT_TOKEN, help="Device Access Token trên ThingsBoard")
    parser.add_argument("--mode", choices=["scenario", "normal", "alarm", "custom"], default="scenario",
                        help="Chế độ kiểm thử: scenario (mô phỏng chuỗi), normal (gửi 26°C), alarm (gửi 40.5°C), custom (tùy chọn)")
    parser.add_argument("--temp", type=float, default=40.5, help="Nhiệt độ tùy chọn khi dùng mode 'custom'")
    parser.add_argument("--delay", type=float, default=2.0, help="Độ trễ giữa các bản tin trong kịch bản (giây)")
    
    args = parser.parse_args()
    
    if args.token == DEFAULT_TOKEN:
        print("[LƯU Ý QUAN TRỌNG]")
        print("Bạn đang dùng Access Token mẫu ('YOUR_DEVICE_ACCESS_TOKEN').")
        print("Để kết nối thành công tới thiết bị thực tế trên ThingsBoard, vui lòng chạy lệnh:")
        print("   python send_telemetry.py --token <DEVICE_ACCESS_TOKEN_CUA_BAN>\n")
        
    if args.mode == "scenario":
        run_scenario(args.host, args.port, args.token, args.delay)
    elif args.mode == "normal":
        run_single(args.host, args.port, args.token, temp=26.5, hum=60.0)
    elif args.mode == "alarm":
        run_single(args.host, args.port, args.token, temp=40.5, hum=45.0)
    elif args.mode == "custom":
        run_single(args.host, args.port, args.token, temp=args.temp)

if __name__ == "__main__":
    main()
