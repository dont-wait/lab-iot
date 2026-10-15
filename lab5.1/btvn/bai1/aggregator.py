"""Gateway gom du lieu tu cac sensor node qua MQTT.

Gateway lang nghe ``sensor/+/data``, luu tam cac ban tin nhan duoc va
phat mot bao cao JSON tong hop len ``gateway/aggregated_report`` moi 10 giay.
"""

from __future__ import annotations

import argparse
import json
import logging
import os
import socket
import threading
import time
from collections import defaultdict
from datetime import datetime, timezone
from typing import Any

import paho.mqtt.client as mqtt


LOGGER = logging.getLogger("gateway-aggregator")

DEFAULT_SENSOR_TOPIC = "sensor/+/data"
DEFAULT_REPORT_TOPIC = "gateway/aggregated_report"
DEFAULT_INTERVAL_SECONDS = 10
DEFAULT_KEEPALIVE_SECONDS = 60
DEFAULT_RECONNECT_DELAY_SECONDS = 5


def utc_timestamp() -> str:
    """Tra ve timestamp UTC theo dinh dang ISO 8601."""
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


class MessageBuffer:
    """Bo dem thread-safe cho cac ban tin tu sensor node."""

    def __init__(self) -> None:
        self._messages: dict[str, list[dict[str, Any]]] = defaultdict(list)
        self._message_count = 0
        self._lock = threading.Lock()

    def add(self, sensor_id: str, message: dict[str, Any]) -> None:
        """Them mot ban tin vao nhom cua sensor tuong ung."""
        with self._lock:
            self._messages[sensor_id].append(message)
            self._message_count += 1

    def drain(self) -> tuple[dict[str, list[dict[str, Any]]], int]:
        """Lay toan bo du lieu hien tai va tao bo dem moi."""
        with self._lock:
            messages = {sensor_id: items for sensor_id, items in self._messages.items()}
            count = self._message_count
            self._messages = defaultdict(list)
            self._message_count = 0
        return messages, count

    def restore(
        self,
        messages: dict[str, list[dict[str, Any]]],
        message_count: int,
    ) -> None:
        """Tra lai du lieu vao bo dem neu publish that bai."""
        with self._lock:
            current_messages = self._messages
            restored_messages: dict[str, list[dict[str, Any]]] = defaultdict(list)

            for sensor_id, items in messages.items():
                restored_messages[sensor_id].extend(items)
            for sensor_id, items in current_messages.items():
                restored_messages[sensor_id].extend(items)

            self._messages = restored_messages
            self._message_count = message_count + self._message_count


def sensor_id_from_topic(topic: str) -> str:
    """Lay node ID tu topic dang sensor/<node_id>/data."""
    topic_parts = topic.split("/")
    if len(topic_parts) >= 3 and topic_parts[0] == "sensor":
        return topic_parts[1]
    return topic


def parse_payload(raw_payload: bytes) -> Any:
    """Parse payload MQTT thanh JSON va bao loi neu payload khong hop le."""
    return json.loads(raw_payload.decode("utf-8"))


def create_client(config: argparse.Namespace, buffer: MessageBuffer) -> mqtt.Client:
    """Tao MQTT client va gan cac callback cho Gateway."""
    client = mqtt.Client(client_id=config.client_id)

    if config.username:
        client.username_pw_set(config.username, config.password)

    def on_connect(
        mqtt_client: mqtt.Client,
        _userdata: Any,
        _flags: dict[str, Any],
        return_code: int,
    ) -> None:
        if return_code != mqtt.MQTT_ERR_SUCCESS:
            LOGGER.error("Ket noi MQTT that bai, ma loi: %s", return_code)
            return

        result, _message_id = mqtt_client.subscribe(config.sensor_topic, qos=1)
        if result == mqtt.MQTT_ERR_SUCCESS:
            LOGGER.info("Da subscribe topic: %s", config.sensor_topic)
        else:
            LOGGER.error("Subscribe that bai, ma loi: %s", result)

    def on_disconnect(
        _mqtt_client: mqtt.Client,
        _userdata: Any,
        return_code: int,
    ) -> None:
        if return_code == mqtt.MQTT_ERR_SUCCESS:
            LOGGER.info("Da ngat ket noi MQTT.")
        else:
            LOGGER.warning(
                "Mat ket noi MQTT, ma loi: %s. Client se tu thu ket noi lai.",
                return_code,
            )

    def on_message(
        _mqtt_client: mqtt.Client,
        _userdata: Any,
        message: mqtt.MQTTMessage,
    ) -> None:
        try:
            payload = parse_payload(message.payload)
        except (UnicodeDecodeError, json.JSONDecodeError) as error:
            LOGGER.warning("Bo qua payload JSON khong hop le tu %s: %s", message.topic, error)
            return

        sensor_id = sensor_id_from_topic(message.topic)
        buffered_message = {
            "received_at": utc_timestamp(),
            "topic": message.topic,
            "data": payload,
        }
        buffer.add(sensor_id, buffered_message)
        LOGGER.info("Nhan du lieu tu node %s", sensor_id)

    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_message = on_message
    client.reconnect_delay_set(
        min_delay=DEFAULT_RECONNECT_DELAY_SECONDS,
        max_delay=DEFAULT_RECONNECT_DELAY_SECONDS * 6,
    )
    return client


def publish_report(
    client: mqtt.Client,
    buffer: MessageBuffer,
    config: argparse.Namespace,
) -> None:
    """Dong goi va publish du lieu da nhan trong mot chu ky."""
    messages, message_count = buffer.drain()
    report = {
        "gateway_id": config.gateway_id,
        "generated_at": utc_timestamp(),
        "message_count": message_count,
        "sensors": messages,
    }
    payload = json.dumps(report, ensure_ascii=False, separators=(",", ":"))

    if not client.is_connected():
        LOGGER.warning("Chua ket noi MQTT; giu lai %d ban tin cho chu ky sau.", message_count)
        buffer.restore(messages, message_count)
        return

    result = client.publish(config.report_topic, payload=payload, qos=1, retain=False)
    if result.rc != mqtt.MQTT_ERR_SUCCESS:
        LOGGER.error("Publish report that bai, ma loi: %s", result.rc)
        buffer.restore(messages, message_count)
        return

    LOGGER.info(
        "Da publish report: %d ban tin tu %d node len %s",
        message_count,
        len(messages),
        config.report_topic,
    )


def parse_args() -> argparse.Namespace:
    """Doc cau hinh tu command line, voi gia tri mac dinh tu bien moi truong."""
    parser = argparse.ArgumentParser(
        description="Gateway gom du lieu sensor qua MQTT moi 10 giay."
    )
    parser.add_argument(
        "--broker-host",
        default=os.getenv("MQTT_BROKER_HOST", "localhost"),
        help="Dia chi MQTT broker (mac dinh: localhost).",
    )
    parser.add_argument(
        "--broker-port",
        type=int,
        default=int(os.getenv("MQTT_BROKER_PORT", "1883")),
        help="Cong MQTT broker (mac dinh: 1883).",
    )
    parser.add_argument(
        "--username",
        default=os.getenv("MQTT_USERNAME"),
        help="Tai khoan MQTT, hoac bien moi truong MQTT_USERNAME.",
    )
    parser.add_argument(
        "--password",
        default=os.getenv("MQTT_PASSWORD", ""),
        help="Mat khau MQTT, hoac bien moi truong MQTT_PASSWORD.",
    )
    parser.add_argument(
        "--client-id",
        default=os.getenv("MQTT_CLIENT_ID", "gateway-aggregator"),
        help="MQTT client ID.",
    )
    parser.add_argument(
        "--gateway-id",
        default=os.getenv("GATEWAY_ID", socket.gethostname()),
        help="ID cua Gateway trong report.",
    )
    parser.add_argument(
        "--sensor-topic",
        default=os.getenv("MQTT_SENSOR_TOPIC", DEFAULT_SENSOR_TOPIC),
        help="Topic subscribe sensor (mac dinh: sensor/+/data).",
    )
    parser.add_argument(
        "--report-topic",
        default=os.getenv("MQTT_REPORT_TOPIC", DEFAULT_REPORT_TOPIC),
        help="Topic publish report (mac dinh: gateway/aggregated_report).",
    )
    parser.add_argument(
        "--interval",
        type=float,
        default=float(os.getenv("AGGREGATION_INTERVAL_SECONDS", DEFAULT_INTERVAL_SECONDS)),
        help="Chu ky publish report theo giay (mac dinh: 10).",
    )
    return parser.parse_args()


def main() -> None:
    """Khoi dong Gateway va duy tri chu ky aggregate/publish."""
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s | %(levelname)s | %(message)s",
    )
    config = parse_args()
    if config.interval <= 0:
        raise SystemExit("--interval phai lon hon 0.")

    buffer = MessageBuffer()
    client = create_client(config, buffer)

    while True:
        try:
            LOGGER.info(
                "Dang ket noi toi MQTT broker %s:%s...",
                config.broker_host,
                config.broker_port,
            )
            client.connect(
                config.broker_host,
                port=config.broker_port,
                keepalive=DEFAULT_KEEPALIVE_SECONDS,
            )
            break
        except OSError as error:
            LOGGER.error("Khong the ket noi MQTT: %s", error)
            LOGGER.info("Thu lai sau %d giay...", DEFAULT_RECONNECT_DELAY_SECONDS)
            time.sleep(DEFAULT_RECONNECT_DELAY_SECONDS)

    client.loop_start()
    next_report_at = time.monotonic() + config.interval

    try:
        while True:
            remaining = next_report_at - time.monotonic()
            if remaining > 0:
                time.sleep(min(remaining, 0.5))
                continue

            publish_report(client, buffer, config)
            next_report_at += config.interval
            while next_report_at <= time.monotonic():
                next_report_at += config.interval
    except KeyboardInterrupt:
        LOGGER.info("Dang dung Gateway...")
    finally:
        client.loop_stop()
        if client.is_connected():
            client.disconnect()
        LOGGER.info("Gateway da dung an toan.")


if __name__ == "__main__":
    main()
