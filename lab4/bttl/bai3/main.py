import requests


WEBHOOK_URL = "YOUR-WEBHOOK-URL"

sensor_data = {
    "device": "ESP32",
    "sensor": "DHT22",
    "temperature_c": 28.5,
    "humidity_percent": 65.0,
}

response = requests.post(WEBHOOK_URL, json=sensor_data, timeout=10)
print(f"Status code: {response.status_code}")
print(f"Response: {response.text}")
