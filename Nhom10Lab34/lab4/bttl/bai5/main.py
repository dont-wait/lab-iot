import requests


WEBHOOK_URL = "https://webhook.site/34dc0077-5ceb-4eed-8c24-96b47cf6f448"

sensor_data = {
    "device": "ESP32",
    "sensor": "DHT22",
    "temperature_c": 28.5,
    "humidity_percent": 65.0,
}

try:
    response = requests.post(WEBHOOK_URL, json=sensor_data, timeout=3)
    print(f"Status code: {response.status_code}")
    print(f"Response: {response.text}")
except requests.exceptions.Timeout:
    print("Request het thoi gian cho (timeout sau 3 giay).")
except requests.exceptions.ConnectionError:
    print("Khong the ket noi den server. Kiem tra mang hoac URL.")
except requests.exceptions.RequestException as error:
    print(f"Request that bai: {error}")
