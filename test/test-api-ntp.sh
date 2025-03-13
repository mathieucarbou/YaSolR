# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2023-2025 Mathieu Carbou
#
#!/usr/bin/env bash

if [ "$1" == "" ]; then
  echo "Usage: $0 <host>"
  exit 1
fi
HOST="$1"
DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
source "${DIR}/utils/api.sh"

assert "$(keys /api/ntp)" "synced"
assert "$(value /api/ntp 'synced')" "true"

post "/api/ntp/update"

post "/api/config" -F "ntp_timezone=Etc/UTC"
sleep 2

post "/api/config" -F "ntp_timezone=Europe/Paris"
sleep 2

echo "===================================================================="
echo "SUCCESS: $0"
echo "===================================================================="
