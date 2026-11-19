@echo off
REM ========================================================================
REM Script test nhanh bang cURL tren Windows cho Bai 5
REM ========================================================================

set TB_HOST=localhost
set TB_PORT=8080
set DEVICE_TOKEN=YOUR_DEVICE_ACCESS_TOKEN

echo [1/2] Gui telemetry binh thuong (26.5 oC)...
curl -v -X POST "http://%TB_HOST%:%TB_PORT%/api/v1/%DEVICE_TOKEN%/telemetry" ^
     -H "Content-Type: application/json" ^
     -d "{\"temperature\": 26.5, \"humidity\": 60.0}"

echo.
echo ========================================================================
echo Cho 3 giay truoc khi gui dot bien...
timeout /t 3 /nobreak > nul

echo.
echo [2/2] Gui telemetry DOT BIEN VUOT NGUONG (40.5 oC) -> KICH HOAT ALARM & GUI EMAIL...
curl -v -X POST "http://%TB_HOST%:%TB_PORT%/api/v1/%DEVICE_TOKEN%/telemetry" ^
     -H "Content-Type: application/json" ^
     -d "{\"temperature\": 40.5, \"humidity\": 45.0}"

echo.
echo ========================================================================
echo Kiem tra ket qua:
echo 1. Tab Debug tren ThingsBoard Rule Chain
echo 2. Tab Alarms tren Thiet bi
echo 3. Hop thu den (Inbox) Email
pause
