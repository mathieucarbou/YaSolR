# SPDX-License-Identifier: MIT
#
# Copyright (C) 2023-2026 Mathieu Carbou
#
Import("env")

import os
import sys


def get_lang(e):
    for flag in e.GetProjectOption("build_flags"):
        if "YASOLR_LANG" in flag:
            return "fr" if "FR" in flag else "en"
    return "en"


pioenv = env["PIOENV"]
lang = get_lang(env)
ref = os.environ.get("BUILD_REF", "unknown")
progname = "YaSolR-%s-%s-%s.OTA" % (ref, pioenv, lang)

env.Replace(PROGNAME=progname)
sys.stderr.write("progname.py: PROGNAME -> %s\n" % progname)
