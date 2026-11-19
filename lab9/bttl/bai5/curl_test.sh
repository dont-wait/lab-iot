#!/usr/bin/env bash
# ========================================================================
# Script test nhanh bằng cURL trên Linux/macOS cho Bài 5
# ========================================================================

TB_HOST="localhost"
TB_PORT=8080
DEVICE_TOKEN="YOUR_DEVICE_ACCESS_TOKEN"

echo "[1/2] Gửi telemetry bình thường (26.5 °C)..."
curl -s -i -X POST "http://${TB_HOST}:${TB_PORT}/api/v1/${DEVICE_TOKEN}/telemetry" \
     -H "Content-Type: application/json" \
     -d '{"temperature": 26.5, "humidity": 60.0}'

echo ""
echo "========================================================================"
echo "Chờ 3 giây trước khi gửi đột biến..."
sleep 3

echo ""
echo "[2/2] Gửi telemetry ĐỘT BIẾN VƯỢT NGƯỠNG (40.5 °C) -> KÍCH HOẠT ALARM & GỬI EMAIL..."
curl -s -i -X POST "http://${TB_HOST}:${TB_PORT}/api/v1/${DEVICE_TOKEN}/telemetry" \
     -H "Content-Type: application/json" \
     -d '{"temperature": 40.5, "humidity": 45.0}'

echo ""
echo "========================================================================"
echo "Kiểm tra kết quả:"
echo "1. Tab Debug trên ThingsBoard Rule Chain"
echo "2. Tab Alarms trên Thiết bị"
echo "3. Hộp thư đến (Inbox) Email"
