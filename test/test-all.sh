# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2023-2024 Mathieu Carbou
#
#!/usr/bin/env bash

set -eE -o functrace

DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# read-only
"${DIR}/test-api.sh" $@
"${DIR}/test-api-app.sh" $@
"${DIR}/test-api-config.sh" $@
"${DIR}/test-api-grid.sh" $@
"${DIR}/test-api-network.sh" $@

# read-write
"${DIR}/test-api-mqtt.sh" $@
"${DIR}/test-api-ntp.sh" $@
"${DIR}/test-api-relays.sh" $@
"${DIR}/test-api-router.sh" $@
"${DIR}/test-api-system.sh" $@
