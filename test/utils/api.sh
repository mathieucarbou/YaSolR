# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2023-2025 Mathieu Carbou
#
#!/usr/bin/env bash

set -eE -o functrace

failure() {
  local lineno=$1
  local msg=$2
  echo "Failed at $lineno: $msg"
}
trap 'failure ${LINENO} "$BASH_COMMAND"' ERR

function body() {
  echo "BODY  http://${HOST}$1" >&2
  curl -s -X GET "http://${HOST}$1"
}

function post() {
  echo "POST  http://${HOST}$@" >&2
  curl -s -o /dev/null -X POST "http://${HOST}$@" >&2
  sleep 2
}

function keys() {
  echo "KEYS  http://${HOST}$1" >&2
  curl -X GET -s "http://${HOST}$1" | jq -cr 'keys|join(",")'
}

function value() {
  echo "VALUE http://${HOST}$1 .$2" >&2
  curl -X GET -s "http://${HOST}$1" | jq -cr ".$2"
}

function assert() {
  if [ "$#" -lt 2 ]; then
    echo "Usage: assert <actual> <expected1> <expected2> "
    exit 1
  fi
  actual=$1
  # echo "> assert $@"
  match=0
  for (( i=2; i<=$#; i+=1 ))    # loop from 1 to N (where N is number of args)
  do
      expected=${!i}
      echo "Expected: <$expected>"
      if [ "$expected" == "$actual" ]; then
        match=1
        break 
      fi
  done
  if [ $match -eq 0 ]; then
    echo "Actual: <$actual>"
    exit 1
  fi
}

function check() {
  if [ "$#" -ne 2 ]; then
    echo "Usage: check <actual> <expression>"
    exit 1
  fi
  echo "> check $@"
  if [ $(bc <<<"$@") -eq 0 ]; then
    echo "Failed test: $@"
    exit 1
  fi
}
