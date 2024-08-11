# SPDX-License-Identifier: MIT
#
# Copyright (C) 2023-2024 Mathieu Carbou
#
Import("env", "projenv")

import sys
import os
from os.path import join, getsize

sys.path.append(join(env.PioPlatform().get_package_dir("tool-esptoolpy")))
import esptool

# print(env.Dump())
# print(projenv.Dump())


def esp32_create_combined_bin(source, target, env):
    print("Generating factory image for serial flashing")

    # print("Building Safeboot image for board: %s" % env.get("BOARD"))
    # env.Execute("SAFEBOOT_BOARD=%s pio run -d ../../tools/Safeboot" % env.get("BOARD"))

    # print("Building File System image")
    # env.Execute('pio run -t buildfs -e %s' % env['PIOENV'])

    safeboot_offset = 0x10000
    safeboot_image = "../../tools/Safeboot/.pio/build/safeboot/safeboot.bin"

    app_offset = 0x110000
    app_image = env.subst("$BUILD_DIR/${PROGNAME}.bin")

    fs_offset = 0
    fs_image = env.subst("$BUILD_DIR/littlefs.bin")

    if env.get("PARTITIONS_TABLE_CSV").endswith("partitions-4MB.csv"):
        fs_offset = 0x3F0000
    if env.get("PARTITIONS_TABLE_CSV").endswith("partitions-8MB.csv"):
        fs_offset = 0x7E0000
    if fs_offset == 0:
        print(env.get("PARTITIONS_TABLE_CSV"))
        raise Exception("Unknown partition file, cannot determine FS offset")

    factory_image = env.subst("$BUILD_DIR/${PROGNAME}.factory.bin")

    sections = env.subst(env.get("FLASH_EXTRA_IMAGES"))
    chip = env.get("BOARD_MCU")
    flash_size = env.BoardConfig().get("upload.flash_size")
    flash_freq = env.BoardConfig().get("build.f_flash", "40m")
    flash_freq = flash_freq.replace("000000L", "m")
    flash_mode = env.BoardConfig().get("build.flash_mode", "dio")
    memory_type = env.BoardConfig().get("build.arduino.memory_type", "qio_qspi")

    if flash_mode == "qio" or flash_mode == "qout":
        flash_mode = "dio"

    if memory_type == "opi_opi" or memory_type == "opi_qspi":
        flash_mode = "dout"

    cmd = [
        "--chip",
        chip,
        "merge_bin",
        "-o",
        factory_image,
        "--flash_mode",
        flash_mode,
        "--flash_freq",
        flash_freq,
        "--flash_size",
        flash_size,
    ]

    # platformio estimates the amount of flash used to store the firmware. this
    # estimate is not accurate. we perform a final check on the firmware bin
    # size by comparing it against the respective partition size.
    max_size = env.BoardConfig().get("upload.maximum_size", 1)
    fw_size = getsize(env.subst("$BUILD_DIR/${PROGNAME}.bin"))
    if fw_size > max_size:
        raise Exception("Firmware binary too large: %d > %d" % (fw_size, max_size))

    print("    Offset | File")
    for section in sections:
        sect_adr, sect_file = section.split(" ", 1)
        print(f" -   {sect_adr} | {sect_file}")
        cmd += [sect_adr, sect_file]

    if os.path.isfile(safeboot_image):
        print(f" -  {hex(safeboot_offset)} | {safeboot_image}")
        cmd += [hex(safeboot_offset), safeboot_image]

    print(f" - {hex(app_offset)} | {app_image}")
    cmd += [hex(app_offset), app_image]

    if os.path.isfile(fs_image):
        print(f" - {hex(fs_offset)} | {fs_image}")
        cmd += [hex(fs_offset), fs_image]

    print("Using esptool.py arguments: %s" % " ".join(cmd))

    esptool.main(cmd)

    print("Factory image generated: %s" % factory_image)


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", esp32_create_combined_bin)
