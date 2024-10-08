[![YaS☀️lR (Yet another Solar Router)](https://yasolr.carbou.me/assets/img/logo.png)](https://yasolr.carbou.me/)

[![GPLv3 license](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.txt)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](code_of_conduct.md)

[![Download](https://img.shields.io/badge/Download-bin-green.svg)](https://yasolr.carbou.me/download)
[![Release](https://img.shields.io/github/release/mathieucarbou/YaSolR.svg)](https://GitHub.com/mathieucarbou/YaSolR/releases/)

[![Build](https://github.com/mathieucarbou/YaSolR/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/mathieucarbou/YaSolR/actions/workflows/build.yml)
[![GitHub latest commit](https://badgen.net/github/last-commit/mathieucarbou/YaSolR)](https://GitHub.com/mathieucarbou/YaSolR/commit/)
[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/mathieucarbou/YaSolR)

[![Doc](https://img.shields.io/badge/Doc-html-green.svg)](https://yasolr.carbou.me/manual)
[![Facebook Group](https://img.shields.io/badge/Facebook-group-blue.svg?logo=Facebook&logoColor=white)](https://www.facebook.com/groups/yasolr)
[![Discord](https://img.shields.io/discord/1293281195798560900.svg)](https://discord.gg/jBTgweft7P)

# Documentation

Please look at the website to know which firmware to download and flash.

👉 Website: [https://yasolr.carbou.me/](https://yasolr.carbou.me/download)

# Downloads

Please look at the release section to find the firmwares.

👉 Releases: [https://github.com/mathieucarbou/YaSolR/releases](https://github.com/mathieucarbou/YaSolR/releases)

**Make sure to download the firmware matching your board.**

Firmware files are named as follow:

- `YaSolR-<VERSION>-<MODEL>-<BOARD>-<LANG>.OTA.bin`: This firmware is used to update through the Web OTA interface
- `YaSolR-<VERSION>-<MODEL>-<BOARD>-<LANG>.FACTORY.bin`: This firmware is used for a first ESP installation, or wen doing a factory reset through USB flashing

Where:

- `VERSION`: YaSolR version, or `main` for the latest development build
- `MODEL`: `oss`, `pro`
- `BOARD`: the board type
- `LANG`: `en`, `fr`, ...

Notes:

- `*.elf` files are used to parse stack traces with [https://maximeborges.github.io/esp-stacktrace-decoder/](https://maximeborges.github.io/esp-stacktrace-decoder/). You do not have to use them.
