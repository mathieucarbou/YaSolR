# SPDX-License-Identifier: MIT
#
# Copyright (C) 2023-2026 Mathieu Carbou
#
Import("env")

import os
import sys
from os.path import join


def status(msg):
    sys.stdout.write("rename.py: %s\n" % msg)


def renameFirmware(source, target, env):
    build_dir = env.subst("$BUILD_DIR")
    progname = env.subst("${PROGNAME}")  # e.g. YaSolR-<ref>-<env>-<lang>.OTA

    # YaSolR-<ref>-<env>-<lang>.OTA.factory.bin -> YaSolR-<ref>-<env>-<lang>.FACTORY.bin
    factory_src = join(build_dir, progname + ".factory.bin")
    factory_dst = join(build_dir, progname.replace(".OTA", ".FACTORY") + ".bin")

    if os.path.isfile(factory_src):
        os.replace(factory_src, factory_dst)
        status("FACTORY -> %s" % os.path.basename(factory_dst))
    else:
        status("WARNING: Factory binary not found: %s" % factory_src)


# Hooks on the same target as factory.py; since rename.py is listed after it in
# extra_scripts, this action is registered second and therefore runs after
# generateFactoryImage has already produced the .factory.bin file.
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", renameFirmware)

