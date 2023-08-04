#!/usr/bin/env bash

set -e

pio run -t upload
pio run -t monitor
