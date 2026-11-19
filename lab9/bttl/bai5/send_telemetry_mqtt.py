#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Lab 9 - Bài 5: Kiểm thử toàn diện chuỗi Rule Chain (End-to-End Testing)
MQTT Client thuần (Pure Python) giao tiếp với ThingsBoard MQTT Broker (port 1883).
Tuân thủ chuẩn MQTT 3.1.1, triển khai bằng thư viện socket chuẩn của Python.
KHÔNG CẦN CÀI ĐẶT THƯ VIỆN BÊN NGOÀI (Zero Dependencies - Không cần paho-mqtt).
"""

import sys
import time
import json
import socket
import struct
import argparse

DEFAULT_HOST = "localhost"
DEFAULT_MQTT_PORT = 1883
DEFAULT_TOKEN = "YOUR_DEVICE_ACCESS_TOKEN"
TOPIC_TELEMETRY = "v1/devices/me/telemetry"

def encode_remaining_length(length: int) -> bytes:
    """Mã hóa độ dài Remaining Length theo chuẩn MQTT 3.1.1"""
    encoded = bytearray()
    while True:
        digit = length % 128
        length //= 128
        if length > 0:
            digit |= 0x80
        encoded.append(digit)
        if length == 0:
            break
    return bytes(encoded)

def build_mqtt_connect_packet(client_id: str, access_token: str, keep_alive: int = 60) -> bytes:
    """
    Tạo gói CONNECT packet MQTT 3.1.1.
    ThingsBoard xác thực thiết bị thông qua trường Username (Username = Access Token).
    """
    # Variable Header: Protocol Name 'MQTT' (4 bytes) + Level 4 (MQTT 3.1.1) + Connect Flags + KeepAlive
    # Connect Flags: 0x82 (Username Flag = 1, Clean Session = 1, các cờ khác = 0)
    protocol_name = b"\x00\x04MQTT"
    protocol_level = b"\x04"
    connect_flags = b"\x82"  # Username flag set, Clean session
    keep_alive_bytes = struct.pack(">H", keep_alive)
    var_header = protocol_name + protocol_level + connect_flags + keep_alive_bytes

    # Payload: Client ID (chuỗi utf-8 tiền tố 2 byte length) + Username (Access Token)
    client_id_bytes = client_id.encode("utf-8")
    payload = struct.pack(">H", len(client_id_bytes)) + client_id_bytes
    token_bytes = access_token.encode("utf-8")
    payload += struct.pack(">H", len(token_bytes)) + token_bytes

    body = var_header + payload
    fixed_header = b"\x10" + encode_remaining_length(len(body))
    return fixed_header + body

def build_mqtt_publish_packet(topic: str, message: str) -> bytes:
    """Tạo gói PUBLISH packet MQTT 3.1.1 (QoS 0)"""
    topic_bytes = topic.encode("utf-8")
    var_header = struct.pack(">H", len(topic_bytes)) + topic_bytes
    payload_bytes = message.encode("utf-8")
    body = var_header + payload_bytes
    fixed_header = b"\x30" + encode_remaining_length(len(body))
    return fixed_header + body

def build_mqtt_disconnect_packet() -> bytes:
    """Tạo gói DISCONNECT packet MQTT 3.1.1"""
    return b"\xe0\x00"

class ThingsBoardMQTTClient:
    def __init__(self, host: str, port: int, access_token: str, client_id: str = "TB_Client_Lab9"):
        self.host = host
        self.port = port
        self.token = access_token
        self.client_id = client_id
        self.sock = None

    def connect(self, timeout: float = 5.0) -> bool:
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(timeout)
            self.sock.connect((self.host, self.port))

            # Gửi gói CONNECT
            connect_packet = build_mqtt_connect_packet(self.client_id, self.token)
            self.sock.sendall(connect_packet)

            # Chờ gói CONNACK (4 bytes: 0x20, 0x02, session_present, return_code)
            connack = self.sock.recv(4)
            if len(connack) >= 4 and connack[0] == 0x20:
                return_code = connack[3]
                if return_code == 0:
                    return True
                else:
                    codes = {
                        1: "Unacceptable protocol version",
                        2: "Identifier rejected",
                        3: "Server unavailable",
                        4: "Bad user name or password (Invalid Access Token)",
                        5: "Not authorized"
                    }
                    print(f"[-] MQTT Kết nối bị từ chối: {codes.get(return_code, f'Code {return_code}')}")
                    return False
            return False
        except Exception as e:
            print(f"[-] Không thể kết nối tới MQTT Broker {self.host}:{self.port} - {e}")
            return False

    def publish_telemetry(self, data: dict) -> bool:
        if not self.sock:
            return False
        try:
            json_str = json.dumps(data)
            publish_packet = build_mqtt_publish_packet(TOPIC_TELEMETRY, json_str)
            self.sock.sendall(publish_packet)
            return True
        except Exception as e:
            print(f"[-] Lỗi gửi bản tin MQTT: {e}")
            return False

    def disconnect(self):
        if self.sock:
            try:
                self.sock.sendall(build_mqtt_disconnect_packet())
                self.sock.close()
            except Exception:
                pass
            self.sock = None

def main():
    print("""
========================================================================
   THINGSBOARD RULE ENGINE - END-TO-END TELEMETRY TEST SCRIPT (MQTT)
   Bài 5: Kiểm thử toàn diện chuỗi Rule Chain bằng giao thức MQTT
   Triển khai: Socket thuần (Pure Python 3.14 Standard Library)
========================================================================
""")
    parser = argparse.ArgumentParser(description="MQTT Client kiểm thử Rule Chain ThingsBoard (Standard Library)")
    parser.add_argument("--host", default=DEFAULT_HOST, help=f"Địa chỉ ThingsBoard MQTT Broker (mặc định: {DEFAULT_HOST})")
    parser.add_argument("--port", type=int, default=DEFAULT_MQTT_PORT, help=f"Cổng MQTT (mặc định: {DEFAULT_MQTT_PORT})")
    parser.add_argument("--token", default=DEFAULT_TOKEN, help="Device Access Token trên ThingsBoard")
    parser.add_argument("--mode", choices=["scenario", "normal", "alarm"], default="scenario",
                        help="Kịch bản gửi dữ liệu: scenario (chuỗi biến thiên), normal (26°C), alarm (40.5°C)")
    parser.add_argument("--delay", type=float, default=2.0, help="Độ trễ giữa các bản tin (giây)")
    args = parser.parse_args()

    client = ThingsBoardMQTTClient(args.host, args.port, args.token)
    print(f"[*] Đang kết nối MQTT Broker {args.host}:{args.port} với Token '{args.token}'...")
    if not client.connect():
        print("[-] Kết nối MQTT thất bại! Hãy kiểm tra địa chỉ host, port và Access Token của thiết bị.")
        sys.exit(1)

    print("[+] Kết nối MQTT thành công (CONNACK: 0x00 Connection Accepted)!")

    try:
        if args.mode == "normal":
            msg = {"temperature": 26.5, "humidity": 60.0}
            print(f"[*] Gửi telemetry bình thường: {msg}")
            client.publish_telemetry(msg)
            print("[+] Đã gửi thành công qua topic 'v1/devices/me/telemetry'!")

        elif args.mode == "alarm":
            msg = {"temperature": 40.5, "humidity": 45.0}
            print(f"[*] Gửi telemetry ĐỘT BIẾN VƯỢT NGƯỠNG: {msg}")
            client.publish_telemetry(msg)
            print("[!] ĐÃ GỬI BẢN TIN VƯỢT NGƯỠNG (40.5°C)!")
            print("    -> Rule Engine sẽ chuyển tiếp qua Filter True -> Create Alarm -> Gửi Email.")

        elif args.mode == "scenario":
            test_points = [
                {"temperature": 25.0, "humidity": 62.0, "note": "Bình thường 1"},
                {"temperature": 26.8, "humidity": 60.5, "note": "Bình thường 2"},
                {"temperature": 27.5, "humidity": 59.0, "note": "Bình thường 3"},
                {"temperature": 40.5, "humidity": 45.0, "note": "ĐỘT BIẾN VƯỢT NGƯỠNG 40°C -> KÍCH HOẠT ALARM"},
                {"temperature": 41.5, "humidity": 43.0, "note": "Duy trì cảnh báo"},
            ]
            print(f"[*] Chạy kịch bản kiểm thử biến thiên nhiệt độ ({len(test_points)} bước):\n")
            for i, pt in enumerate(test_points, 1):
                note = pt.pop("note")
                is_alarm = pt["temperature"] >= 40.0
                flag = " [!] KÍCH HOẠT ALARM" if is_alarm else " [OK] Bình thường"
                client.publish_telemetry(pt)
                print(f"  Bước {i}: Gửi {pt} ({note}){flag}")
                if is_alarm:
                    print("     >>> Quan sát tab Debug trên Rule Engine và kiểm tra Email Inbox!")
                time.sleep(args.delay)
    finally:
        client.disconnect()
        print("\n[*] Đã đóng kết nối MQTT an toàn.")

if __name__ == "__main__":
    main()
