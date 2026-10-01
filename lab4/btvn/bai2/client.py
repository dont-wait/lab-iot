"""Client gửi một mẫu dữ liệu cảm biến tới Flask server cục bộ."""

import random
from datetime import datetime

import requests


SERVER_URL = "http://127.0.0.1:5000/api/sensor"


def main() -> None:
    data = {
        "temperature": round(random.uniform(20, 40), 2),
        "humidity": round(random.uniform(50, 90), 2),
        "timestamp": datetime.now().astimezone().isoformat(timespec="seconds"),
    }

    try:
        response = requests.post(SERVER_URL, json=data, timeout=10)
        response.raise_for_status()
    except requests.RequestException as error:
        print(f"Không thể gửi dữ liệu tới server: {error}")
        return

    print(f"Dữ liệu đã gửi: {data}")
    print(f"Phản hồi từ server (HTTP {response.status_code}): {response.json()}")


if __name__ == "__main__":
    main()
