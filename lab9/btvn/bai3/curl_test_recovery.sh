#!/usr/bin/env bash
# ========================================================================
# Script test tự động phục hồi và Clear Alarm trên Linux/macOS cho Bài 3
# ========================================================================

TB_HOST="localhost"
TB_PORT=8080
DEVICE_TOKEN="YOUR_DEVICE_ACCESS_TOKEN"

echo "[PHA 1/3] Gửi telemetry QUÁ NHIỆT (41.5 °C) -> KÍCH HOẠT CREATE ALARM..."
curl -s -i -X POST "http://${TB_HOST}:${TB_PORT}/api/v1/${DEVICE_TOKEN}/telemetry" \
     -H "Content-Type: application/json" \
     -d '{"temperature": 41.5}'

echo ""
echo "========================================================================"
echo "Chờ 4 giây... Kiểm tra Alarm đang ACTIVE trên ThingsBoard."
sleep 4

echo ""
echo "[PHA 2/3] Gửi telemetry VÙNG ĐỆM HYSTERESIS (32.0 °C) -> ALARM VẪN PHẢI ACTIVE..."
curl -s -i -X POST "http://${TB_HOST}:${TB_PORT}/api/v1/${DEVICE_TOKEN}/telemetry" \
     -H "Content-Type: application/json" \
     -d '{"temperature": 32.0}'

echo ""
echo "========================================================================"
echo "Chờ 4 giây... Nhiệt độ chưa dưới 28 °C nên Alarm không được clear."
sleep 4

echo ""
echo "[PHA 3/3] Gửi telemetry AN TOÀN PHỤC HỒI (26.0 °C < 28 °C) -> KÍCH HOẠT CLEAR ALARM & GỬI EMAIL PHỤC HỒI..."
curl -s -i -X POST "http://${TB_HOST}:${TB_PORT}/api/v1/${DEVICE_TOKEN}/telemetry" \
     -H "Content-Type: application/json" \
     -d '{"temperature": 26.0}'

echo ""
echo "========================================================================"
echo "Hoàn thành! Kiểm tra:"
echo "1. Tab Alarms: Alarm chuyển thành CLEARED_UNACK"
echo "2. Email Inbox: Nhận email 'Hệ thống đã trở lại trạng thái ổn định'"
