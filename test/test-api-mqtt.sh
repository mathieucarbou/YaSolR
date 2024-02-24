# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2023-2024 Mathieu Carbou
#
#!/usr/bin/env bash

if [ "$1" == "" ]; then
  echo "Usage: $0 <host>"
  exit 1
fi
HOST="$1"
DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
source "${DIR}/utils/api.sh"

post "/api/config" -F "mqtt_enable=false"
assert "$(value /api/config 'mqtt_enable')" "false"

post "/api/config" -F "mqtt_enable=true"
assert "$(value /api/config 'mqtt_enable')" "true"
sleep 6

echo "===================================================================="
echo "SUCCESS: $0"
echo "===================================================================="
