#!/usr/bin/env bash

set -euo pipefail

BROKER_HOST="${MQTT_BROKER_HOST:-127.0.0.1}"
BROKER_PORT="${MQTT_BROKER_PORT:-1883}"
USERNAME="${MQTT_USERNAME:-gateway-client}"

if ! command -v mosquitto_pub >/dev/null 2>&1; then
    echo "Chua tim thay mosquitto_pub. Hay cai mosquitto-clients truoc." >&2
    exit 1
fi

if [[ -z "${MQTT_PASSWORD:-}" ]]; then
    read -r -s -p "Nhap mat khau cho $USERNAME: " MQTT_PASSWORD
    echo
fi

echo "[1/3] Kiem tra dang nhap dung..."
if mosquitto_pub \
    -h "$BROKER_HOST" \
    -p "$BROKER_PORT" \
    -u "$USERNAME" \
    -P "$MQTT_PASSWORD" \
    -t gateway/aggregated_report \
    -m '{"test":"authenticated"}' \
    -q 1; then
    echo "PASS: Client dang nhap dung va duoc phep publish."
else
    echo "FAIL: Dang nhap dung nhung ket noi that bai." >&2
    exit 1
fi

echo "[2/3] Kiem tra client khong co thong tin dang nhap..."
if mosquitto_pub \
    -h "$BROKER_HOST" \
    -p "$BROKER_PORT" \
    -t gateway/aggregated_report \
    -m '{"test":"anonymous"}' \
    -q 1 >/dev/null 2>&1; then
    echo "FAIL: Broker van cho phep anonymous." >&2
    exit 1
else
    echo "PASS: Client khong co username/password bi tu choi."
fi

echo "[3/3] Kiem tra sai mat khau..."
if mosquitto_pub \
    -h "$BROKER_HOST" \
    -p "$BROKER_PORT" \
    -u "$USERNAME" \
    -P 'wrong-password-for-test' \
    -t gateway/aggregated_report \
    -m '{"test":"wrong-password"}' \
    -q 1 >/dev/null 2>&1; then
    echo "FAIL: Broker chap nhan sai mat khau." >&2
    exit 1
else
    echo "PASS: Client sai mat khau bi tu choi."
fi

echo "Hoan tat kiem tra access control."
