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

assert "$(keys /api/router)" "energy,output1,output2,power,power_factor,virtual_grid_power"

check "$(value /api/router 'energy')" ">=0"
check "$(value /api/router 'power')" ">=0"
check "$(value /api/router 'power_factor')" ">=0"
check "$(value /api/router 'virtual_grid_power')" "!=0"

# output1

assert "$(value /api/router 'output1.enabled')" "true"
check "$(value /api/router 'output1.power')" ">=0"
assert "$(value /api/router/output1 'output1.state')" "Disabled" "Routing" "manual Bypass" "Auto Bypass" "Idle"

assert "$(value /api/router 'output1.bypass_relay.enabled')" "true"
assert "$(value /api/router 'output1.bypass_relay.state')" "off"
check "$(value /api/router 'output1.bypass_relay.switch_count')" ">=0"

assert "$(value /api/router 'output1.dimmer.enabled')" "true"
assert "$(value /api/router 'output1.dimmer.level')" "0"
assert "$(value /api/router 'output1.dimmer.state')" "off"

assert "$(value /api/router 'output1.temp_sensor.enabled')" "true"
assert "$(value /api/router 'output1.temp_sensor.valid')" "true"
check "$(value /api/router 'output1.temp_sensor.temperature')" ">0"

# output 2

assert "$(value /api/router 'output2.enabled')" "true"
check "$(value /api/router 'output2.power')" "==0"
assert "$(value /api/router/output2 'output2.state')" "Idle"

assert "$(value /api/router 'output2.bypass_relay.enabled')" "true"
assert "$(value /api/router 'output2.bypass_relay.state')" "off"
check "$(value /api/router 'output2.bypass_relay.switch_count')" ">=0"

assert "$(value /api/router 'output2.dimmer.enabled')" "true"
assert "$(value /api/router 'output2.dimmer.level')" "0"
assert "$(value /api/router 'output2.dimmer.state')" "off"

assert "$(value /api/router 'output2.temp_sensor.enabled')" "false"
assert "$(value /api/router 'output2.temp_sensor.valid')" "false"
check "$(value /api/router 'output2.temp_sensor.temperature')" ">=0"

# posts

post "/api/router/output1/bypass_relay" -F "state=on"
assert "$(value /api/router 'output1.bypass_relay.state')" "on"
check "$(value /api/router 'output1.bypass_relay.switch_count')" ">=1"

post "/api/router/output1/bypass_relay" -F "state=off"
check "$(value /api/router 'output1.bypass_relay.switch_count')" ">=2"

post "/api/router/output1/dimmer" -F "level=50"
assert "$(value /api/router 'output1.dimmer.level')" "50"
assert "$(value /api/router 'output1.dimmer.state')" "on"

post "/api/router/output1/dimmer" -F "level=0"
assert "$(value /api/router 'output1.dimmer.level')" "0"
assert "$(value /api/router 'output1.dimmer.state')" "off"

echo "===================================================================="
echo "SUCCESS: $0"
echo "===================================================================="
