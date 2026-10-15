#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

if [[ ! -f "$SCRIPT_DIR/passwd" ]]; then
    echo "Chua co file passwd. Hay chay ./setup_auth.sh truoc." >&2
    exit 1
fi

if [[ ! -f "$SCRIPT_DIR/aclfile" ]]; then
    echo "Khong tim thay file aclfile." >&2
    exit 1
fi

cd "$SCRIPT_DIR"
exec mosquitto -c "$SCRIPT_DIR/mosquitto.conf" -v
