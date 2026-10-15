#!/usr/bin/env bash

set -euo pipefail

if ! command -v node >/dev/null 2>&1 || ! command -v npm >/dev/null 2>&1; then
    echo "Chua tim thay Node.js/npm. Dang cai dat tu apt..."
    sudo apt update
    sudo apt install -y nodejs npm
fi

if ! command -v node-red >/dev/null 2>&1; then
    echo "Dang cai dat Node-RED..."
    sudo npm install -g --unsafe-perm node-red
else
    echo "Node-RED da duoc cai dat: $(node-red --version 2>/dev/null || true)"
fi

NODE_RED_DIR="${HOME}/.node-red"
mkdir -p "$NODE_RED_DIR"

echo "Dang cai node-red-dashboard vao $NODE_RED_DIR ..."
(cd "$NODE_RED_DIR" && npm install node-red-dashboard)

echo
echo "Cai dat hoan tat. Chay Node-RED bang lenh: node-red"
echo "Dashboard se mo tai: http://127.0.0.1:1880/ui"
