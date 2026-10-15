"""Lang nghe cac lenh dieu khien noi bo gui den Gateway qua MQTT."""

import os

import paho.mqtt.client as mqtt


BROKER_HOST = os.getenv("MQTT_BROKER", "localhost")
BROKER_PORT = int(os.getenv("MQTT_PORT", "1883"))
COMMAND_TOPIC = "gateway/command/#"


def on_connect(client, userdata, flags, reason_code, properties) -> None:
    if reason_code == 0:
        print(f"Da ket noi MQTT broker {BROKER_HOST}:{BROKER_PORT}")
        client.subscribe(COMMAND_TOPIC)
        print(f"Dang lang nghe topic '{COMMAND_TOPIC}'...")
    else:
        print(f"Ket noi that bai, ma loi: {reason_code}")


def on_message(client, userdata, message) -> None:
    payload = message.payload.decode("utf-8", errors="replace")
    print(f"Nhan lenh | topic: {message.topic} | noi dung: {payload}")


def main() -> None:
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="gateway-command")
    client.on_connect = on_connect
    client.on_message = on_message

    print(f"Dang ket noi den MQTT broker {BROKER_HOST}:{BROKER_PORT}...")
    client.connect(BROKER_HOST, BROKER_PORT, keepalive=60)

    try:
        client.loop_forever()
    except KeyboardInterrupt:
        print("\nDa dung chuong trinh.")
    finally:
        client.disconnect()


if __name__ == "__main__":
    main()
