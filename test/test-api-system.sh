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

assert "$(keys /api/system)" "boots,buzzer,chip_cores,chip_model,chip_revision,cpu_freq,heap_total,heap_usage,heap_used,lights,tasks,temp_sensor,uptime"
check "$(value /api/system 'boots')" ">0"
check "$(value /api/system 'chip_cores')" "==2"
check "$(value /api/system 'heap_total')" ">1000"
check "$(value /api/system 'heap_usage')" ">40"
check "$(value /api/system 'heap_used')" ">1000"
check "$(value /api/system 'uptime')" ">1"

assert "$(value /api/system 'buzzer.enabled')" "false"

assert "$(value /api/system 'lights.code')" "🟢 ⚫ ⚫" "🟢 🟡 ⚫"
assert "$(value /api/system 'lights.green')" "on"
assert "$(value /api/system 'lights.red')" "off"
assert "$(value /api/system 'lights.yellow')" "on" "off"

assert "$(value /api/system 'temp_sensor.enabled')" "true"
check "$(value /api/system 'temp_sensor.temperature')" ">20"
assert "$(value /api/system 'temp_sensor.valid')" "true"

# rw buzzer tests

post "/api/config" -F "buzzer_enable=true"
assert "$(value /api/system 'buzzer.enabled')" "true"

post "/api/config" -F "buzzer_enable=false"
assert "$(value /api/system 'buzzer.enabled')" "false"

# rw temp tests

post "/api/config" -F "sys_tmp_enable=false"
post "/api/config" -F "out1_tmp_enable=false"

assert "$(value /api/system 'temp_sensor.enabled')" "false"
check "$(value /api/system 'temp_sensor.temperature')" "==0"
assert "$(value /api/system 'temp_sensor.valid')" "false"

assert "$(value /api/router 'output1.temp_sensor.enabled')" "false"
check "$(value /api/router 'output1.temp_sensor.temperature')" "==0"
assert "$(value /api/router 'output1.temp_sensor.valid')" "false"

post "/api/config" -F "sys_tmp_enable=true"
post "/api/config" -F "out1_tmp_enable=true"

# update interval is 10 sec for temp
sleep 10

assert "$(value /api/system 'temp_sensor.enabled')" "true"
check "$(value /api/system 'temp_sensor.temperature')" ">20"
assert "$(value /api/system 'temp_sensor.valid')" "true"

assert "$(value /api/router 'output1.temp_sensor.enabled')" "true"
check "$(value /api/router 'output1.temp_sensor.temperature')" ">20"
assert "$(value /api/router 'output1.temp_sensor.valid')" "true"

echo "===================================================================="
echo "SUCCESS: $0"
echo "===================================================================="
