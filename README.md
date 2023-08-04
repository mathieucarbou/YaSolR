[![YaS☀️lR (Yet another Solar Router)](https://yasolr.carbou.me/assets/img/logo.png)](https://yasolr.carbou.me/)

[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](code_of_conduct.md)
[![GPLv3 license](https://img.shields.io/badge/License-GPLv3-blue.svg)](http://perso.crans.org/besson/LICENSE.html)
[![Download](https://img.shields.io/badge/Download-bin-green.svg)](https://yasolr.carbou.me/download)
[![Doc](https://img.shields.io/badge/Doc-html-green.svg)](https://yasolr.carbou.me/manual)
[![Facebook Group](https://img.shields.io/badge/Facebook-group-blue.svg?logo=Facebook&logoColor=white)](https://www.facebook.com/groups/yasolr)

[![YaS☀️lR OSS](https://img.shields.io/badge/YaSolR%20OSS-sources-green.svg)](https://github.com/mathieucarbou/YaSolR-OSS/) [![YaS☀️lR OSS Build](https://github.com/mathieucarbou/YaSolR-OSS/actions/workflows/build-oss.yml/badge.svg?branch=main)](https://github.com/mathieucarbou/YaSolR-OSS/actions/workflows/build-oss.yml)

# Documentation

Please look at the website and manual for more information about how to flash.

👉 Website: [https://yasolr.carbou.me/](https://yasolr.carbou.me/)

# Downloads

**Please make sure to download the firmware matching your board from these locations:**

- [`latest`](https://github.com/mathieucarbou/YaSolR-OSS/releases/tag/latest) (latest development version)

Firmware files are named as follow:

- `YaSolR-<VERSION>-oss--<BOARD>.bin`: for the firmware used to update
- `YaSolR-<VERSION>-oss-<BOARD>.factory.bin`: for the firmware used to flash for the first time or to do a factory reset

Where:

- `VERSION`: YaS☀️lR version, or `main` for the latest development build
- `BOARD`: the board name, from the list of [Compatible ESP32 boards](https://yasolr.carbou.me/build#compatible-esp32-boards)

**Images ending with `-debug` are for development purpose or to troubleshoot issues: they are not optimized and output a lot of debug logs.**

# Developer guide

## Project structure

- `.github`: CI/CD workflows
- `data`: Build components added to the firmware
- `include`: Firmware include code
- `lib`: Firmware libraries
- `pio`: pio scripts
- `src`: Firmware source code
- `test`: Firmware tests
- `tools`: Some random tools
- `partitions.csv`: ESP32 custom partition table
- `platformio_override.ini.template` (to copy to `platformio_override.ini`): PlatformIO configuration override
- `platformio.ini`: PlatformIO configuration

## Building the firmware

1. Configure `platformio_override.ini` to your needs (you can copy the template file `platformio_override.ini.template` to `platformio_override.ini` for that)

2. Build the file system and the firmware:

```bash
pio run -t build
```

## Flashing

First time, flash the entire firmware which includes the partition table and all partitions:

```bash
esptool.py --port /dev/ttyUSB0 \
  --chip esp32 \
  --before default_reset \
  --after hard_reset \
  write_flash \
  --flash_mode dout \
  --flash_freq 40m \
  --flash_size detect \
  0x0 YaSolR-VERSION-MODEL-CHIP.factory.bin
```

Next time, just upload the partition you modify

```bash
pio run -t upload
pio run -t monitor
```
