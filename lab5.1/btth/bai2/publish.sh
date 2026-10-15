#!/usr/bin/env bash
set -euo pipefail

mosquitto_pub -h "${MQTT_HOST:-localhost}" -p "${MQTT_PORT:-1883}" \
  -t "${MQTT_TOPIC:-gateway/test}" -m "${MQTT_MESSAGE:-Hello Local MQTT}"

