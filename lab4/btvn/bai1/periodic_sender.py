"""Gửi dữ liệu cảm biến giả lập lên Webhook.site mỗi 10 giây."""

import argparse
import os
import random
import time
from datetime import datetime

import requests


SEND_INTERVAL_SECONDS = 10


def create_sensor_data() -> dict[str, float | str]:
    """Tạo một mẫu dữ liệu nhiệt độ, độ ẩm và thời gian đo."""
    return {
        "temperature": round(random.uniform(20, 40), 2),
        "humidity": round(random.uniform(50, 90), 2),
        "timestamp": datetime.now().astimezone().isoformat(timespec="seconds"),
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Gửi dữ liệu cảm biến giả lập lên Webhook.site mỗi 10 giây."
    )
    parser.add_argument(
        "--url",
        default=os.getenv("WEBHOOK_URL"),
        help="URL Webhook.site (hoặc đặt biến môi trường WEBHOOK_URL)",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    if not args.url:
        raise SystemExit(
            "Chưa có URL webhook. Hãy dùng --url <URL> hoặc đặt biến WEBHOOK_URL."
        )

    print(f"Đang gửi dữ liệu mỗi {SEND_INTERVAL_SECONDS} giây. Nhấn Ctrl+C để dừng.")

    try:
        while True:
            data = create_sensor_data()

            try:
                response = requests.post(args.url, json=data, timeout=10)
                response.raise_for_status()
                print(
                    f"[{data['timestamp']}] Gửi thành công "
                    f"(HTTP {response.status_code}): {data}"
                )
            except requests.RequestException as error:
                print(f"[{data['timestamp']}] Gửi thất bại: {error}")

            time.sleep(SEND_INTERVAL_SECONDS)
    except KeyboardInterrupt:
        print("\nĐã dừng chương trình an toàn.")


if __name__ == "__main__":
    main()
