#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PASSWORD_FILE="$SCRIPT_DIR/passwd"

if ! command -v mosquitto_passwd >/dev/null 2>&1; then
    echo "Chua tim thay mosquitto_passwd. Hay cai mosquitto truoc." >&2
    exit 1
fi

umask 077

echo "Tao mat khau cho tai khoan Gateway: gateway-client"
mosquitto_passwd -c "$PASSWORD_FILE" gateway-client

echo "Tao mat khau cho tai khoan Sensor: sensor-client"
mosquitto_passwd "$PASSWORD_FILE" sensor-client

chmod 600 "$PASSWORD_FILE"
echo "Da tao $PASSWORD_FILE voi quyen 600."
echo "Khong commit file passwd vao Git."
