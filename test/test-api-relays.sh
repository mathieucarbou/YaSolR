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

assert "$(value /api/relays 'relay1.enabled')" "true"
assert "$(value /api/relays 'relay1.state')" "off"
check "$(value /api/relays 'relay1.switch_count')" ">=0"

assert "$(value /api/relays 'relay2.enabled')" "true"
assert "$(value /api/relays 'relay2.state')" "off"
check "$(value /api/relays 'relay2.switch_count')" ">=0"

post "/api/config" -F "relay2_enable=false"

assert "$(value /api/relays 'relay2.enabled')" "false"
assert "$(value /api/relays 'relay2.state')" "off"

post "/api/config" -F "relay2_enable=true"

assert "$(value /api/relays 'relay2.enabled')" "true"
assert "$(value /api/relays 'relay2.state')" "off"
check "$(value /api/relays 'relay2.switch_count')" ">=0"

post "/api/relays/relay2" -F "state=on" -F "duration=4000"
assert "$(value /api/relays 'relay2.enabled')" "true"
assert "$(value /api/relays 'relay2.state')" "on"
check "$(value /api/relays 'relay2.switch_count')" ">=1"
assert "$(value /api/system 'lights.green')" "on"
assert "$(value /api/system 'lights.red')" "off"
assert "$(value /api/system 'lights.yellow')" "on"

sleep 3
assert "$(value /api/relays 'relay2.enabled')" "true"
assert "$(value /api/relays 'relay2.state')" "off"
check "$(value /api/relays 'relay2.switch_count')" ">=2"
assert "$(value /api/system 'lights.green')" "on"
assert "$(value /api/system 'lights.red')" "off"
assert "$(value /api/system 'lights.yellow')" "on", "off"

echo "===================================================================="
echo "SUCCESS: $0"
echo "===================================================================="
