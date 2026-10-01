"""Flask server nhận dữ liệu cảm biến qua HTTP POST."""

from flask import Flask, jsonify, request


app = Flask(__name__)


@app.post("/api/sensor")
def receive_sensor_data():
    data = request.get_json(silent=True)

    if data is None:
        return jsonify(status="error", message="Request body must be valid JSON"), 400

    print(f"Dữ liệu cảm biến đã nhận: {data}", flush=True)
    return jsonify(
        status="success",
        message="Data received successfully",
    ), 200


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=False)
