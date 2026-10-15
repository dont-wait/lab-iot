"""Publish CPU va RAM cua Gateway len MQTT broker moi 5 giay."""

import json
import os
import time
from datetime import datetime, timezone

import psutil
import paho.mqtt.client as mqtt


BROKER_HOST = os.getenv("MQTT_BROKER", "localhost")
BROKER_PORT = int(os.getenv("MQTT_PORT", "1883"))
TOPIC = "gateway/system/metrics"
PUBLISH_INTERVAL_SECONDS = 5


def collect_system_metrics() -> dict:
    """Doc thong so CPU, RAM hien tai va tra ve du lieu co the chuyen thanh JSON."""
    memory = psutil.virtual_memory()
    return {
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "cpu_percent": psutil.cpu_percent(interval=None),
        "ram_percent": memory.percent,
        "ram_used_mb": round(memory.used / (1024**2), 2),
        "ram_total_mb": round(memory.total / (1024**2), 2),
    }


def main() -> None:
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="gateway-metrics")
    client.connect(BROKER_HOST, BROKER_PORT, keepalive=60)
    client.loop_start()

    # Khoi tao bo dem CPU. Lan doc tiep theo se tinh muc su dung trong 5 giay.
    psutil.cpu_percent(interval=None)
    print(f"Da ket noi MQTT broker {BROKER_HOST}:{BROKER_PORT}")
    print(f"Dang publish len topic '{TOPIC}' moi {PUBLISH_INTERVAL_SECONDS} giay...")

    try:
        while True:
            time.sleep(PUBLISH_INTERVAL_SECONDS)
            payload = json.dumps(collect_system_metrics(), ensure_ascii=False)
            result = client.publish(TOPIC, payload)
            result.wait_for_publish()
            print(f"Da gui: {payload}")
    except KeyboardInterrupt:
        print("\nDa dung chuong trinh.")
    finally:
        client.loop_stop()
        client.disconnect()


if __name__ == "__main__":
    main()
