#!/usr/bin/env bash

set -e

[ -x "$(command -v python)" ] && PYTHON=python || PYTHON=python3

echo "======================="
echo "Install..."
echo "======================="
echo ""

$PYTHON -m pip install --upgrade pip
pip install --upgrade platformio
pip install --upgrade cpplint

echo ""
echo "======================="
echo "cpplint..."
echo "======================="
echo ""

cpplint \
  --repository=. \
  --recursive \
  --filter=-whitespace/line_length,-whitespace/braces,-whitespace/comments,-runtime/indentation_namespace,-whitespace/indent,-readability/braces,-whitespace/newline,-readability/todo,-build/c++11 \
  --exclude=lib/ElegantOTAPro \
  --exclude=lib/ESPDASH \
  --exclude=lib/ESPDASHPro \
  --exclude=lib/WebSerialLite \
  lib \
  include \
  src

if [ -z "$1" ]; then
  echo ""
  echo "=========================="
  echo "Building default env..."
  echo "=========================="
  echo ""
  pio run
else
  echo ""
  echo "=========================="
  echo "Building env: $1..."
  echo "=========================="
  echo ""
  pio run -e $1
fi
