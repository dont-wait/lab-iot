# Bai 5: Gateway subscribe lenh dieu khien noi bo

Script dang ky topic wildcard `gateway/command/#` va in moi lenh nhan duoc tu local MQTT broker ra terminal.

## Cai dat va chay

Dam bao Mosquitto (hoac MQTT broker khac) dang chay tai `localhost:1883`, sau do mo terminal thu nhat:

```powershell
python -m pip install -r requirements.txt
python main.py
```

Mo terminal thu hai va gui mot lenh bang cong cu Mosquitto:

```powershell
mosquitto_pub -h localhost -p 1883 -t "gateway/command/led" -m '{"action":"on"}'
```

Terminal chay Gateway se hien thi:

```text
Nhan lenh | topic: gateway/command/led | noi dung: {"action":"on"}
```

Ky tu `#` giup Gateway nhan moi topic con, vi du `gateway/command/led`, `gateway/command/fan` va `gateway/command/relay/1`.

Neu broker nam tren may khac, dat bien moi truong truoc khi chay:

```powershell
$env:MQTT_BROKER = "192.168.1.10"
$env:MQTT_PORT = "1883"
python main.py
```

Nhan `Ctrl+C` de dung chuong trinh.
