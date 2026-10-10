#!/bin/bash
# Convenience wrapper around idf.py and espota.
# Usage:
#   ./build.sh build
#   ./build.sh flash                    (USB, requires device connected)
#   ./build.sh ota [IP]                 (OTA via espota, default 192.168.1.209)
#   ./build.sh flash monitor            (USB flash then open serial monitor)
#   ./build.sh monitor

set -e

OTA_IP="${2:-192.168.1.209}"
ESPOTA=~/.platformio/packages/framework-arduinoespressif32/tools/espota.py

if [ "$1" = "ota" ]; then
    shift
    echo "OTA upload to ${OTA_IP}..."
    python3 "$ESPOTA" -i "$OTA_IP" -p 3232 -f build/withrottle_knob.bin
    exit $?
fi

IDF_PATH="${IDF_PATH:-/home/nils/.espressif/v6.1/esp-idf}"
if [ ! -d "$IDF_PATH" ]; then
    echo "ESP-IDF not found at $IDF_PATH" >&2
    exit 1
fi

IDF_TOOLS_PATH="${IDF_TOOLS_PATH:-/home/nils/.espressif}"
IDF_PYTHON_ENV_PATH="${IDF_PYTHON_ENV_PATH:-/home/nils/.espressif/tools/python/v6.1/venv}"
export IDF_PATH IDF_TOOLS_PATH IDF_PYTHON_ENV_PATH

# Keep the legacy ESP-IDF default path in sync with the active toolchain
# installation so export.sh can resolve the Python virtual environment.
mkdir -p /home/nils/.espressif/python_env
ln -sfn /home/nils/.espressif/tools/python/v6.1/venv /home/nils/.espressif/python_env/idf6.1_py3.12_env
ln -sfn /home/nils/.espressif/tools/espidf.constraints.v6.1.txt /home/nils/.espressif/espidf.constraints.v6.1.txt
# Source the official ESP-IDF environment so the correct toolchain, cmake, ninja,
# and Python env are selected for this workspace.
# shellcheck disable=SC1091
. "$IDF_PATH/export.sh"

exec "$IDF_PATH/tools/idf.py" "$@"
