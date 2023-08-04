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

assert "$(keys /api/grid)" "frequency,jsy,online,power,voltage,zcd"

check "$(value /api/grid 'frequency')" ">48"
check "$(value /api/grid 'frequency')" "<52"
assert "$(value /api/grid 'online')" "true"
check "$(value /api/grid 'power')" "!=0"
check "$(value /api/grid 'voltage')" ">200"

check "$(value /api/grid 'jsy.current1')" ">=0"
check "$(value /api/grid 'jsy.current2')" ">=0"
assert "$(value /api/grid 'jsy.enabled')" "true"
check "$(value /api/grid 'jsy.energy1')" ">=0"
check "$(value /api/grid 'jsy.energy2')" ">=0"
check "$(value /api/grid 'jsy.energyReturned1')" "==0"
check "$(value /api/grid 'jsy.energyReturned2')" "==0"
check "$(value /api/grid 'jsy.frequency')" ">48"
check "$(value /api/grid 'jsy.frequency')" "<52"
check "$(value /api/grid 'jsy.power_factor1')" ">=0"
check "$(value /api/grid 'jsy.power_factor2')" ">=0"
check "$(value /api/grid 'jsy.power1')" ">=0"
check "$(value /api/grid 'jsy.power2')" ">=0"
check "$(value /api/grid 'jsy.voltage1')" ">200"
check "$(value /api/grid 'jsy.voltage2')" ">200"

assert "$(value /api/grid 'zcd.enabled')" "true"
check "$(value /api/grid 'zcd.frequency')" ">48"
check "$(value /api/grid 'zcd.frequency')" "<52"

echo "===================================================================="
echo "SUCCESS: $0"
echo "===================================================================="
