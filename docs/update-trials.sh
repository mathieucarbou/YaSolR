#!/usr/bin/env bash

set -e

to_dir="docs/downloads/trials"
link="/downloads/trials"

rm -f -r $to_dir/*
gh release download v1.0.0-rc8 -D $to_dir -R mathieucarbou/YaSolR-Pro -p "YaSolR-*-trial-*.FACTORY.bin" --clobber

echo "Markdown:"
echo ""

for f in $(ls $to_dir); do
  echo "- [$f]($link/$f)"
done
