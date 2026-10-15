"""Minimal Paho-MQTT client used to verify a local broker connection."""

from __future__ import annotations

import threading
import time

import paho.mqtt.client as mqtt


BROKER_HOST = "127.0.0.1"
BROKER_PORT = 1883
CONNECT_TIMEOUT_SECONDS = 5


def main() -> int:
    connected = threading.Event()
    connection_error: list[str] = []

    def on_connect(
        client: mqtt.Client,
        userdata: object,
        flags: dict[str, int],
        reason_code: mqtt.ReasonCode,
        properties: mqtt.Properties | None = None,
    ) -> None:
        if reason_code == 0:
            connected.set()
            print(f"Kết nối MQTT thành công: {BROKER_HOST}:{BROKER_PORT}")
        else:
            connection_error.append(str(reason_code))
            connected.set()

    def on_connect_fail(client: mqtt.Client, userdata: object) -> None:
        connection_error.append("Không thể kết nối đến broker")
        connected.set()

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="lab5-bai3")
    client.on_connect = on_connect
    client.on_connect_fail = on_connect_fail

    try:
        client.connect(BROKER_HOST, BROKER_PORT, keepalive=60)
        client.loop_start()
        if not connected.wait(CONNECT_TIMEOUT_SECONDS):
            print("Lỗi: quá thời gian chờ kết nối MQTT.")
            return 1
        if connection_error:
            print(f"Lỗi kết nối MQTT: {connection_error[0]}")
            return 1
        time.sleep(0.2)
        return 0
    except OSError as error:
        print(f"Lỗi mở kết nối đến MQTT broker: {error}")
        return 1
    finally:
        client.loop_stop()
        client.disconnect()


if __name__ == "__main__":
    raise SystemExit(main())

