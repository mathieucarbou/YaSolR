[![YaS☀️lR (Yet another Solar Router)](https://yasolr.carbou.me/assets/img/logo.png)](https://yasolr.carbou.me/)

[![GPLv3 license](https://img.shields.io/badge/License-GPLv3-blue.svg)](http://perso.crans.org/besson/LICENSE.html)
[![Download](https://img.shields.io/badge/Download-bin-green.svg)](https://yasolr.carbou.me/download)
[![Doc](https://img.shields.io/badge/Doc-html-green.svg)](https://yasolr.carbou.me/manual)

[![YaS☀️lR OSS](https://img.shields.io/badge/YaSolR%20OSS-sources-green.svg)](https://github.com/mathieucarbou/YaSolR-OSS/) [![YaS☀️lR OSS Build](https://github.com/mathieucarbou/YaSolR-OSS/actions/workflows/build-oss.yml/badge.svg?branch=main)](https://github.com/mathieucarbou/YaSolR-OSS/actions/workflows/build-oss.yml)

# Documentation

Please look at the website and manual for more information about how to flash.

👉 Website: [https://yasolr.carbou.me/](https://yasolr.carbou.me/)

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
./build.sh
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
./upload.sh
```
