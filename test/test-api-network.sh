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

assert "$(keys /api/network)" "ip_address,mac_address,state,wifi_bssid,wifi_rssi,wifi_signal,wifi_ssid"
assert "$(value /api/network 'state')" "STA_CONNECTED"
assert "$(value /api/network 'ip_address')" "$HOST"
check "$(value /api/network 'wifi_rssi')" ">-80"
assert "$(value /api/network 'wifi_ssid')" "IoT"
check "$(value /api/network 'wifi_signal')" ">50"

echo "===================================================================="
echo "SUCCESS: $0"
echo "===================================================================="
