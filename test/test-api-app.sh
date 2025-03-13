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

assert "$(keys /api/app)" "buildDate,buildHash,debug,firmware,id,manufacturer,model,name,trial,version"
assert "$(value /api/app 'name')" "YaSolR"
assert "$(value /api/app 'manufacturer')" "Mathieu Carbou"
assert "$(value /api/app 'trial')" "true"

echo "===================================================================="
echo "SUCCESS: $0"
echo "===================================================================="
