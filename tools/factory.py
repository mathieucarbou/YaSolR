# SPDX-License-Identifier: MIT
#
# Copyright (C) 2023-2025 Mathieu Carbou
#
Import("env")

import sys
import os
import requests
from os.path import join, getsize

sys.path.append(join(env.PioPlatform().get_package_dir("tool-esptoolpy")))
import esptool

# print(env.Dump())

quiet = False


def status(msg):
    """Print status message to stderr"""
    if not quiet:
        critical(msg)


def critical(msg):
    """Print critical message to stderr"""
    sys.stderr.write("factory.py: ")
    sys.stderr.write(msg)
    sys.stderr.write("\n")


def generateFactooryImage(source, target, env):
    status("Generating factory image for serial flashing")

    app_offset = 0x10000
    app_image = env.subst("$BUILD_DIR/${PROGNAME}.bin")

    # Set fs_offset = 0 to disable LittleFS image generation
    # Set fs_offset to the correct offset from the partition to generate a LittleFS image
    fs_offset = 0
    fs_image = env.subst("$BUILD_DIR/littlefs.bin")

    safeboot_offset = 0x10000
    safeboot_image = env.GetProjectOption("custom_safeboot_file", "")

    if safeboot_image == "":
        safeboot_project = env.GetProjectOption("custom_safeboot_dir", "")
        if safeboot_project != "":
            status(
                "Building SafeBoot image for board %s from %s"
                % (env.get("BOARD"), safeboot_project)
            )
            if not os.path.isdir(safeboot_project):
                raise Exception("SafeBoot project not found: %s" % safeboot_project)
            env.Execute(
                "SAFEBOOT_BOARD=%s pio run -e safeboot -d %s" % (env.get("BOARD"), safeboot_project)
            )
            safeboot_image = join(safeboot_project, ".pio/build/safeboot/firmware.bin")
            if not os.path.isfile(safeboot_image):
                raise Exception("SafeBoot image not found: %s" % safeboot_image)

    if safeboot_image == "":
        safeboot_url = env.GetProjectOption("custom_safeboot_url", "")
        if safeboot_url != "":
            safeboot_image = env.subst("$BUILD_DIR/safeboot.bin")
            if not os.path.isfile(safeboot_image):
                status(
                    "Downloading SafeBoot image from %s to %s"
                    % (safeboot_url, safeboot_image)
                )
                response = requests.get(safeboot_url)
                if response.status_code != 200:
                    raise Exception("Download error: %d" % response.status_code)
                with open(safeboot_image, "wb") as file:
                    file.write(response.content)

    if fs_offset != 0:
        status("Building File System image")
        env.Execute("pio run -t buildfs -e %s" % env["PIOENV"])

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

    status("     Offset | File")
    for section in sections:
        sect_adr, sect_file = section.split(" ", 1)
        status(f" -   {sect_adr} | {sect_file}")
        cmd += [sect_adr, sect_file]

    if safeboot_image != "":
        if os.path.isfile(safeboot_image):
            app_offset = 0xB0000
            status(f" -  {hex(safeboot_offset)} | {safeboot_image}")
            cmd += [hex(safeboot_offset), safeboot_image]
        else:
            raise Exception("SafeBoot image not found: %s" % safeboot_image)

    status(f" -  {hex(app_offset)} | {app_image}")
    cmd += [hex(app_offset), app_image]

    if fs_image != 0 and os.path.isfile(fs_image):
        status(f" - {hex(fs_offset)} | {fs_image}")
        cmd += [hex(fs_offset), fs_image]

    status("Using esptool.py arguments: %s" % " ".join(cmd))

    esptool.main(cmd)

    status("Factory image generated! You can flash it with:\n> esptool.py write_flash 0x0 %s" % factory_image)


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", generateFactooryImage)
