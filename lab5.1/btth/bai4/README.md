# Bai 4: Gateway publish thong so he thong dinh ky

Script doc muc su dung CPU va RAM cua Gateway, dong goi thanh JSON va publish len local MQTT broker moi 5 giay.

## Cai dat va chay

Dam bao Mosquitto (hoac MQTT broker khac) dang chay tai `localhost:1883`, sau do thuc hien:

```powershell
python -m pip install -r requirements.txt
python main.py
```

Topic publish: `gateway/system/metrics`.

Payload mau:

```json
{
  "timestamp": "2026-10-15T03:00:00+00:00",
  "cpu_percent": 12.5,
  "ram_percent": 63.2,
  "ram_used_mb": 10240.5,
  "ram_total_mb": 16384.0
}
```

Co the doi dia chi broker bang bien moi truong:

```powershell
$env:MQTT_BROKER = "192.168.1.10"
$env:MQTT_PORT = "1883"
python main.py
```

Nhan `Ctrl+C` de dung chuong trinh.
