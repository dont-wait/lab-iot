@echo off
REM ========================================================================
REM Script test tu dong phuc hoi va Clear Alarm tren Windows cho Bai 3
REM ========================================================================

set TB_HOST=localhost
set TB_PORT=8080
set DEVICE_TOKEN=YOUR_DEVICE_ACCESS_TOKEN

echo [PHA 1/3] Gui telemetry QUA NHIET (41.5 oC) -> KICH HOAT CREATE ALARM...
curl -v -X POST "http://%TB_HOST%:%TB_PORT%/api/v1/%DEVICE_TOKEN%/telemetry" ^
     -H "Content-Type: application/json" ^
     -d "{\"temperature\": 41.5}"

echo.
echo ========================================================================
echo Cho 4 giay... Kiem tra Alarm dang ACTIVE tren ThingsBoard.
timeout /t 4 /nobreak > nul

echo.
echo [PHA 2/3] Gui telemetry VUNG DEM HYSTERESIS (32.0 oC) -> ALARM VAN PHAI ACTIVE...
curl -v -X POST "http://%TB_HOST%:%TB_PORT%/api/v1/%DEVICE_TOKEN%/telemetry" ^
     -H "Content-Type: application/json" ^
     -d "{\"temperature\": 32.0}"

echo.
echo ========================================================================
echo Cho 4 giay... Nhiet do chua duoi 28 oC nen Alarm khong duoc clear.
timeout /t 4 /nobreak > nul

echo.
echo [PHA 3/3] Gui telemetry AN TOAN PHUC HOI (26.0 oC < 28 oC) -> KICH HOAT CLEAR ALARM & GUI EMAIL PHUC HOI...
curl -v -X POST "http://%TB_HOST%:%TB_PORT%/api/v1/%DEVICE_TOKEN%/telemetry" ^
     -H "Content-Type: application/json" ^
     -d "{\"temperature\": 26.0}"

echo.
echo ========================================================================
echo Hoan thanh! Kiem tra:
echo 1. Tab Alarms: Alarm chuyen thanh CLEARED_UNACK
echo 2. Email Inbox: Nhan email 'He thong da tro lai trang thai on dinh'
pause
