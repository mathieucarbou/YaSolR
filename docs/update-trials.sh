#!/usr/bin/env bash

set -e

to_dir="docs/downloads/trials"
link="/downloads/trials"

rm -f -r $to_dir/*
gh release download latest -D $to_dir -R mathieucarbou/YaSolR -p "YaSolR-*-trial-*.bin" --clobber
rm $to_dir/*-debug.bin | true
rm $to_dir/*-debug.FACTORY.bin | true

echo "Markdown:"
echo ""

for f in $(ls $to_dir); do
  echo "- [$f]($link/$f)"
done
