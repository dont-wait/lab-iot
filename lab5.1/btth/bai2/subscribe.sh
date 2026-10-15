#!/usr/bin/env bash
set -euo pipefail

mosquitto_sub -h "${MQTT_HOST:-localhost}" -p "${MQTT_PORT:-1883}" \
  -t "${MQTT_TOPIC:-gateway/test}" -v

